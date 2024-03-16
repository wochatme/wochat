#include "stdafx.h"
#include "resource.h"
#include "mosquitto.h"
#include "secp256k1.h"
#include "secp256k1_ecdh.h"

#include "wochatypes.h"
#include "wochatdef.h"
#include "wochat.h"

#define MOSQUITTO_STATUS_CONNECTING		0
#define MOSQUITTO_STATUS_CONNACK_RECVD	1
#define MOSQUITTO_STATUS_WAITING		2
#define MOSQUITTO_STATUS_DISCONNECTING	3
#define MOSQUITTO_STATUS_DISCONNECTED	4
#define MOSQUITTO_STATUS_NOHOPE			5
#define MOSQUITTO_STATUS_DONE			6

typedef struct MQTT_Conf
{
	bool isPub;
	MemoryPoolContext ctx; // the memory pool of this configuration
	MemoryPoolContext mempool; // the memory pool of the sub or pub thread
	HWND hWnd; // the window handle of the UI window
	char* id;
	int protocol_version;
	char* host;
	int port;
	int qos;
	bool retain;
	char* message; /* pub, rr */
	U32 msglen; /* pub, rr */
	char* topic; /* pub, rr */
	char* bind_address;
	char** topics; /* sub, rr */
	int topic_count;
	int maxtopics; /* maximum topics we can use */
	mosquitto_property* connect_props;
	mosquitto_property* publish_props;
	mosquitto_property* subscribe_props;
	mosquitto_property* unsubscribe_props;
	mosquitto_property* disconnect_props;
	mosquitto_property* will_props;
	void* opaque;
} MQTT_Conf;

static MQTT_Conf confSub = { 0 };
static MQTT_Conf confPub = { 0 };

static volatile int pub_status = MOSQUITTO_STATUS_CONNECTING;
static U32 ng_seqnumber = 0;

typedef struct PublishTask
{
	PublishTask* next;
	MessageTask* node;
	U8* message;
	U32 msgLen;
} PublishTask;

// put a message node into the send message queue
U32 PushTaskIntoSendMessageQueue(MessageTask* mt)
{
	MessageTask* p;
	MessageTask* q;
	bool hooked = false;

	if (mt)
	{
		mt->sequence = InterlockedIncrement(&g_messageId);
	}

	EnterCriticalSection(&g_csMQTTPub);
	if (nullptr == g_PubQueue) // there is no task in the queue
	{
		g_PubQueue = mt;
		LeaveCriticalSection(&g_csMQTTPub);
		SetEvent(g_MQTTPubEvent); // tell the Pub thread to work
		return WT_OK;
	}

	p = q = g_PubQueue; // the first node in this queue

	while (p) // try to find the first node that is not processed yet
	{
		if (p->state < MESSAGE_TASK_STATE_COMPLETE) // this task is not processed
			break;
		p = p->next;
	}

	g_PubQueue = p;  // here, g_PubQueue is pointing to the first node not procesded or NULL
	if (nullptr == g_PubQueue) // there is no not-processed task in the queue
	{
		g_PubQueue = mt;
	}
	p = q; // p is pointing to the first node of the queue
	while (p)
	{
		q = p->next;
		if (nullptr == q && !hooked) // we reach the end of the queue, put the new node at the end of the queue
		{
			p->next = mt;
			hooked = true;
		}

		if (p->state == MESSAGE_TASK_STATE_COMPLETE) // this task has been processed, we can free the memory
		{
			if (p->type != 'T') // if the type is 'T', the real text is used by WTChatgroup, so we do not free it.
			{
				wt_pfree(p->message);
			}
			wt_pfree(p);
		}
		p = q;
	}
	LeaveCriticalSection(&g_csMQTTPub);

	if (mt)
	{
		SetEvent(g_MQTTPubEvent); // tell the Pub thread to work
	}
	return WT_OK;
}


S64 GetCurrentUTCTime64()
{
	S64 tm;
	SYSTEMTIME st;

	GetSystemTime(&st);
	U8* p = (U8*)&tm;
#ifndef WT_IS_BIG_ENDIAN
	p[7] = 0;
	p[0] = st.wSecond;
	p[1] = st.wMinute;
	p[2] = st.wHour;
	p[3] = st.wDay;
	p[4] = st.wMonth;
	U16* p16 = (U16*)(p + 5);
	*p16 = st.wYear;
#else
	p[0] = 0;
	p[7] = st.wSecond;
	p[6] = st.wMinute;
	p[5] = st.wHour;
	p[4] = st.wDay;
	p[3] = st.wMonth;
	U16* p16 = (U16*)(p + 1);
	*p16 = st.wYear;
#endif
	return tm;
}

// we have the 32-byte secret key, we want to generate the public key, return 0 if successful
U32 GenPublicKeyFromSecretKey(U8* sk, U8* pk)
{
	U32 ret = WT_FAIL;
	secp256k1_context* ctx;

	ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
	if (ctx)
	{
		int return_val;
		size_t len = 33;
		U8 compressed_pubkey[33];
		U8 randomize[32];
		secp256k1_pubkey pubkey;

		if (WT_OK == wt_GenerateRandom32Bytes(randomize))
		{
			return_val = secp256k1_ec_pubkey_create(ctx, &pubkey, sk);
			if (1 == return_val)
			{
				return_val = secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);
				if (len == 33 && return_val == 1 && pk)
				{
					for (int i = 0; i < 33; i++) pk[i] = compressed_pubkey[i];
					ret = WT_OK;
				}
			}
		}
		secp256k1_context_destroy(ctx);
	}
	return ret;
}

static U32 LoadFriendList()
{
	assert(g_friendRoot == nullptr);

	sqlite3* db;
	int rc = sqlite3_open16(g_DBPath, &db);
	if (SQLITE_OK == rc)
	{
		sqlite3_stmt* stmt = NULL;
		U8 hexSK[65] = { 0 };
		char sql[256] = { 0 };

		assert(g_myInfo);
		wt_Raw2HexString(g_myInfo->skenc, SECRET_KEY_SIZE, hexSK, nullptr);

		sprintf_s((char*)sql, 256, "SELECT pk,dt,sr,ub,at FROM p WHERE id=1 OR me='%s' ORDER BY id", hexSK);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK == rc)
		{
			U8* pk;
			S64 dt;
			U8* sr;
			U8* ub;
			U8* si;
			U32 ub_len, si_len, active;
			WTFriend* p = nullptr;
			WTFriend* q = nullptr;
			bool found = false;
			WTGuy* guy;

			while (SQLITE_ROW == sqlite3_step(stmt))
			{
				pk     = (U8*)sqlite3_column_text(stmt, 0);
				dt     = (U32)sqlite3_column_int64(stmt, 1);
				sr     = (U8*)sqlite3_column_text(stmt, 2);
				ub     = (U8*)sqlite3_column_blob(stmt, 3);
				ub_len = (U32)sqlite3_column_bytes(stmt, 3);
				active = (U32)sqlite3_column_int64(stmt, 4);

				if (ub_len == WT_BLOB_LEN)
				{
					p = (WTFriend*)wt_palloc0(g_messageMemPool, sizeof(WTFriend));
					if (p)
					{
						PopluateFriendInfo(p, ub, ub_len);
						// insert into the hash tab
						found = false;
						guy = (WTGuy*)hash_search(g_peopleHTAB, p->pubkey, HASH_ENTER, &found);
						assert(guy);
						if (!found)
						{
							guy->people = p;
						}
						if (active)
						{
							g_friendTotal++;
							p->property = WT_FRIEND_PROP_ACITVE;
							if (g_friendRoot == nullptr)
							{
								g_friendRoot = p;
								q = p;
							}
							else
							{
								assert(q);
								q->next = p;
								p->prev = q;
								q = p;
							}
						}
					}
				}
			}
			sqlite3_finalize(stmt);
		}
		sqlite3_close(db);
	}
	return WT_OK;
}

static U32 LoadChatList()
{
	assert(g_chatgroupRoot == nullptr);
#if 10
	g_chatgroupRoot = (WTChatGroup*)wt_palloc0(g_messageMemPool, sizeof(WTChatGroup));
	if (g_chatgroupRoot)
	{
		g_chatgroupRoot->people = g_friendRoot;
		g_friendRoot->chatgroup = g_chatgroupRoot;
		g_chatgroupTotal = 1;
	}
#endif
#if 0
	sqlite3* db;
	int rc = sqlite3_open16(g_DBPath, &db);
	if (SQLITE_OK == rc)
	{
		sqlite3_stmt* stmt = NULL;
		U8* sender;
		U8* receiver;
		U8* message;
		char pk_b64[45] = { 0 };
		char sql[256] = { 0 };

		assert(g_pMyInformation);
		wt_b64_encode((const char*)g_pMyInformation->pubkey, PUBLIC_KEY_SIZE, pk_b64, 44);

		sprintf_s((char*)sql, 256, "SELECT sd,re,tx FROM m WHERE st<>0 AND tp='@' AND (sd='%s' OR re='%s') ORDER BY id", pk_b64, pk_b64);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK == rc)
		{
			bool equal = false;
			U8 pubkey[PUBLIC_KEY_SIZE];
			WTFriend* people;
			WTChatGroup* cg;
			WTGuy* guy;

			while (SQLITE_ROW == sqlite3_step(stmt))
			{
				sender = (U8*)sqlite3_column_text(stmt, 0);
				receiver = (U8*)sqlite3_column_text(stmt, 1);
				message = (U8*)sqlite3_column_text(stmt, 2);

				if (sender != nullptr && strlen((const char*)sender) == 44)
				{
					WT_MEMCMP(sender, pk_b64, 44, equal);
					if (equal && receiver != nullptr && strlen((const char*)receiver) == 44)
					{
						if (wt_b64_decode((const char*)receiver, 44, (char*)pubkey, PUBLIC_KEY_SIZE) == PUBLIC_KEY_SIZE)
						{
							WT_MEMCMP(pubkey, g_pMyInformation->pubkey, PUBLIC_KEY_SIZE, equal);
							if (!equal)
							{

							}
						}
					}
				}

			}
			sqlite3_finalize(stmt);
		}
		sqlite3_close(db);
	}
#endif
	return WT_OK;
}

DWORD WINAPI LoadingDataThread(LPVOID lpData)
{
	U8 percentage = 0;
	HWND hWndUI;

	InterlockedIncrement(&g_threadCount);

	hWndUI = (HWND)(lpData);
	ATLASSERT(::IsWindow(hWndUI));

	while (0 == g_Quit)
	{
		if (percentage == 0)
		{
			LoadFriendList();
		}
		Sleep(20);
		PostMessage(hWndUI, WM_LOADPERCENTMESSAGE, (WPARAM)percentage, 0);
		percentage += 2;
		if (percentage == 10)
		{
			LoadChatList();
		}
		if (percentage >= 104)
			break;
	}

	InterlockedDecrement(&g_threadCount);
    return 0;
}


static U32 MQTTConfInit(MemoryPoolContext pool, HWND hWnd, bool isPub = true)
{
	U32 ret = WT_FAIL;
	MQTT_Conf* conf = isPub ? &confPub : &confSub;

	memset(conf, 0, sizeof(MQTT_Conf));

	if (isPub)
		conf->ctx = wt_mempool_create("PubConfiguration", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);
	else
		conf->ctx = wt_mempool_create("SubConfiguration", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);

	if (conf->ctx)
	{
		conf->mempool = pool;
		conf->hWnd = hWnd;

		if (!isPub)
		{
			conf->topic_count = 0;
			conf->maxtopics = WT_MQTT_MAXTOPICS;
			conf->topics = (char**)wt_palloc(conf->ctx, (WT_MQTT_MAXTOPICS * sizeof(char*))); // an array of char* pointer
			if (conf->topics)
				ret = WT_OK;
		}
		else ret = WT_OK;
	}
	return ret;
}

static void MQTTConfTerm(bool isPub = true)
{
	MQTT_Conf* conf = isPub ? &confPub : &confSub;

	wt_mempool_destroy(conf->ctx);
	memset(conf, 0, sizeof(MQTT_Conf));
}

static void MQTTConfAddSubTopic(char* topic)
{
	MQTT_Conf* conf = &confSub;
	if (conf->ctx)
	{
		if (conf->topic_count < WT_MQTT_MAXTOPICS)
		{
			char* tp = wt_pstrdup(conf->ctx, topic);
			if (tp)
			{
				conf->topics[conf->topic_count] = tp;
				conf->topic_count++;
			}
		}
	}
}

/* each time we only pick up one task from the send out queue */
static PublishTask* CheckSendMessageTasks(MemoryPoolContext mempool)
{
	EnterCriticalSection(&g_csMQTTPub);
	if (g_PubQueue)
	{
		U32 ret, len = 0;
		U32* p32;
		PublishTask* item;
		MessageTask* p = g_PubQueue;
		while (p)
		{
			if (p->state < MESSAGE_TASK_STATE_COMPLETE)
			{
				item = (PublishTask*)wt_palloc0(mempool, sizeof(PublishTask));
				if (item)
				{
					if (p->type == 'T') // it is UTF16 encoding, we convert to UTF8 encoding
					{
						ret = wt_UTF16ToUTF8((U16*)p->message, p->msgLen, nullptr, &len);
						if (WT_OK == ret)
						{
							// we use the first 4 bytes to fill some random data
							// so even the same string has different hash value
							item->message = (U8*)wt_palloc(mempool, len + 4);
							if (item->message)
							{
								p32 = (U32*)item->message;
								*p32 = ng_seqnumber++;
								wt_UTF16ToUTF8((U16*)p->message, p->msgLen, item->message + 4, nullptr);
								assert(ret == WT_OK);
								item->msgLen = len + 4;
								item->node = p;
								LeaveCriticalSection(&g_csMQTTPub);
								return item;
							}
							else
							{
								wt_pfree(item);
								item = nullptr;
							}
						}
						else
						{
							wt_pfree(item);
							item = nullptr;
						}
					}
					else if (p->type == 'Q' || p->type == 'U' || p->type == 'A')
					{
						item->node = p;
						LeaveCriticalSection(&g_csMQTTPub);
						return item;
					}
				}
			}
			p = p->next;
		}
	}
	LeaveCriticalSection(&g_csMQTTPub);

	return nullptr;
}

// put the incoming message into the queue
static U32 PushTaskIntoReceiveMessageQueue(MessageTask* task = nullptr)
{
	U32 ret = 0;
	MessageTask* p;

	EnterCriticalSection(&g_csMQTTSub);
	while (g_SubQueue) // scan the link to find the message that has been processed
	{
		p = g_SubQueue->next;
		if (MESSAGE_TASK_STATE_NULL == g_SubQueue->state) // this task is not processed yet.
			break;

		wt_pfree(g_SubQueue->message);
		wt_pfree(g_SubQueue);
		g_SubQueue = p;
	}

	if (nullptr == g_SubQueue) // there is no task in the queue
	{
		g_SubQueue = task;
		LeaveCriticalSection(&g_csMQTTSub);
		return WT_OK;
	}
	assert(g_SubQueue);
	p = g_SubQueue;
	while (p->next) p = p->next;  // look for the last node

	p->next = task;  // put task as the last node

	LeaveCriticalSection(&g_csMQTTSub);
	return WT_OK;
}

static U32 GetKeyFromSecretKeyAndPlubicKey(U8* sk, U8* pk, U8* key)
{
	U32 ret = WT_FAIL;
	secp256k1_context* ctx;

	assert(sk);
	assert(pk);
	assert(key);

	ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
	if (ctx)
	{
		int rc;
		secp256k1_pubkey pubkey;
		U8 k[SECRET_KEY_SIZE] = { 0 };

		rc = secp256k1_ec_pubkey_parse(ctx, &pubkey, pk, PUBLIC_KEY_SIZE);
		if (1 != rc)
		{
			secp256k1_context_destroy(ctx);
			return WT_SECP256K1_CTX_ERROR;
		}
		rc = secp256k1_ecdh(ctx, k, &pubkey, sk, NULL, NULL);
		if (1 == rc)
		{
			for (int i = 0; i < SECRET_KEY_SIZE; i++)
			{
				key[i] = k[i];
				k[i] = 0;
			}
			ret = WT_OK;
		}
		secp256k1_context_destroy(ctx);
	}
	return ret;
}

static void sub_connect_callback(struct mosquitto* mosq, void* obj, int result, int flags, const mosquitto_property* properties)
{
	UNUSED(flags);
	UNUSED(properties);

	if (result == 0)
	{
		MQTT_Conf* conf = (MQTT_Conf*)obj;
		if (conf)
		{
			if (conf->topic_count > 0)
				mosquitto_subscribe_multiple(mosq, NULL, conf->topic_count, conf->topics, 1, 0, NULL);
		}
	}
}

static void sub_disconnect_callback(struct mosquitto* mosq, void* obj, int rc, const mosquitto_property* properties)
{
}

static U32 HandleQueryMessage(HWND hWndUI, MemoryPoolContext mempool, U8* primary_key, U8* pubkey_receiver, U8* msg_b64, U32 len_b64)
{
	assert(len_b64 == 140 + 89);
	U32 length_raw = wt_b64_dec_len(len_b64 - 89);
	U8 message_raw[32 + 33 + 4 + 4 + 32];
	U32 crc32 = wt_GenCRC32(msg_b64, 89);

	int rc = wt_b64_decode((const char*)(msg_b64 + 89), len_b64 - 89, (char*)message_raw, length_raw);
	if (rc == 105)
	{
		int i;
		U8 hash0[32];
		U8 hash1[32];
		U8 Ks[32] = { 0 };
		U8 nonce[12] = { 0 };
		AES256_ctx ctxAES = { 0 };
		wt_chacha20_context cxt = { 0 };

		U8* p = message_raw;

		wt_AES256_init(&ctxAES, primary_key);
		wt_AES256_decrypt(&ctxAES, 2, Ks, p); // get the session key at first

		for (i = 0; i < 12; i++) nonce[i] = i;
		wt_chacha20_init(&cxt);
		wt_chacha20_setkey(&cxt, Ks);
		wt_chacha20_starts(&cxt, nonce, 0);
		wt_chacha20_update(&cxt, length_raw - 32, (const unsigned char*)(p + 32), p + 32);  // decrypt the message!
		wt_chacha20_free(&cxt);

		wt_AES256_decrypt(&ctxAES, 2, hash0, p + 32 + 33 + 4 + 4); // get the session key at first
		wt_sha256_hash(message_raw + 32, 33 + 4 + 4, hash1);
		if ((memcmp(hash0, hash1, 32) == 0) && (crc32 == *((U32*)(message_raw + 32 + 33 + 4))))
		{
			MessageTask* task = (MessageTask*)wt_palloc0(mempool, sizeof(MessageTask));
			if (task)
			{
				sqlite3* db;
				task->sequence = InterlockedIncrement(&g_messageId);

				//EnterCriticalSection(&g_csSQLiteDB);
				int rc = sqlite3_open16(g_DBPath, &db);
				if (SQLITE_OK == rc)
				{
					sqlite3_stmt* stmt = NULL;
					rc = sqlite3_prepare_v2(db,
						(const char*)"INSERT INTO m(id,st,dt,hs,sd,re,tp,tx) VALUES((?),1,(?),(?),(?),(?),(?),(?))", -1, &stmt, NULL);
					if (SQLITE_OK == rc)
					{
						U8 result = 0;
						U8* s = msg_b64;
						U8 hexHash[65] = { 0 };
						wt_Raw2HexString(hash0, SHA256_HASH_SIZE, hexHash, NULL);
						S64 dt = GetCurrentUTCTime64();

						rc = sqlite3_bind_int(stmt, 1, task->sequence);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_int64(stmt, 2, dt);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 3, (const char*)hexHash, 64, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 4, (const char*)s, 44, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 5, (const char*)(s + 44), 44, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 6, (const char*)(s + 88), 1, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 7, (const char*)(s + 89), len_b64 - 89, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						if (result == 0)
						{
							rc = sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);
					}
					sqlite3_close(db);
				}
				//LeaveCriticalSection(&g_csSQLiteDB);
				task->value32 = *((U32*)(message_raw + 32 + 33));
				p = message_raw + 32;
				for (i = 0; i < PUBLIC_KEY_SIZE; i++) task->pubkey[i] = p[i];
				task->type = 'R';
				PushTaskIntoReceiveMessageQueue(task);
			}
		}
	}
	return WT_OK;
}

static U32 Handle_F_Message(HWND hWndUI, MemoryPoolContext mempool, U8* pubkey, U8* message, U32 length)
{
#if 0
	MessageTask* task = (MessageTask*)wt_palloc0(mempool, sizeof(MessageTask));
	if (task)
	{
		U32 crc32 = *((U32*)message);

		task->msgLen = length;
		task->message = (U8*)wt_palloc(mempool, task->msgLen);
		if (task->message)
		{
			sqlite3* db;
			U8 hexSK[65] = { 0 };
			U8 hexPK[67] = { 0 };

			wt_Raw2HexString(g_myInfo->skenc, SECRET_KEY_SIZE, hexSK, nullptr);
			wt_Raw2HexString(message + 4, PUBLIC_KEY_SIZE, hexPK, nullptr);

			memcpy(task->message, message, task->msgLen);
			memcpy(task->pubkey, pubkey, PUBLIC_KEY_SIZE);
			task->type = 'F';

			EnterCriticalSection(&g_csSQLiteDB);
			int rc = sqlite3_open16(g_DBPath, &db);
			if (SQLITE_OK == rc)
			{
				int count = 0;
				sqlite3_stmt* stmt = NULL;
				char sql[256] = { 0 };
				sprintf_s((char*)sql, 256, "SELECT count(1) FROM p WHERE me='%s' AND pk='%s' AND at<>0", hexSK, hexPK);
				rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					rc = sqlite3_step(stmt);
					if (rc == SQLITE_ROW)
						count = sqlite3_column_int(stmt, 0);
				}
				sqlite3_finalize(stmt);

				// when the user wants to add a new friend, the UI thread will insert a record in table p.
				// if we cannot find this record in table p, which means the user only want to get this user's
				// information, but does not want to add him/her as his friend, so we put active = 0
				if (count == 0)
					sprintf_s((char*)sql, 256, "INSERT INTO p(at,me,pk,ub,si) VALUES(0,'%s','%s',(?),(?))", hexSK, hexPK);
				else
					sprintf_s((char*)sql, 256, "UPDATE p SET ub=(?),si=(?) WHERE me='%s' AND pk='%s'", hexSK, hexPK);

				rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					U8 result = 0;
					bool isMalloced = false;
					U8* icon = (U8*)malloc(WT_SMALL_ICON_SIZE);
					if (icon)
					{
						U32* largeIcon =
							(U32*)(message + 4 + PUBLIC_KEY_SIZE + 3 + 1 + 4 + WT_NAME_MAX_LEN + WT_MOTTO_MAX_LEN + WT_AREA_MAX_LEN);
						wt_Resize128To32Bmp(largeIcon, (U32*)icon);
						isMalloced = true;
					}
					else icon = (U8*)GetUIBitmap(WT_UI_BMP_MYMSALLICON);
					rc = sqlite3_bind_blob(stmt, 1, message, length, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_blob(stmt, 2, icon, WT_SMALL_ICON_SIZE, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					if (result == 0)
						rc = sqlite3_step(stmt);
					sqlite3_finalize(stmt);

					if (isMalloced)
						free(icon);
				}
				sqlite3_close(db);
			}
			LeaveCriticalSection(&g_csSQLiteDB);

			PushTaskIntoReceiveMessageQueue(task);
			//::PostMessage(hWndUI, WM_MQTT_SUBMESSAGE, 0, 0); // tell the UI thread that a new message is received
		}
		else wt_pfree(task);
	}
#endif
	return WT_OK;
}

static U32 Handle_T_Message(HWND hWndUI, MemoryPoolContext mempool, U8* pubkey, U8* message, U32 length)
{
#if 0
	assert(length > 4);
	if (wt_UTF8ToUTF16(message + 4, length - 4, nullptr, nullptr) == WT_OK)
	{
		MessageTask* task = (MessageTask*)wt_palloc0(mempool, sizeof(MessageTask));
		if (task)
		{
			task->msgLen = length - 4;
			task->message = (U8*)wt_palloc(mempool, task->msgLen);
			if (task->message)
			{
				memcpy(task->message, message + 4, length - 4);
				memcpy(task->pubkey, pubkey, PUBLIC_KEY_SIZE);
				task->type = 'T';
				PushTaskIntoReceiveMessageQueue(task);
				//::PostMessage(hWndUI, WM_MQTT_SUBMESSAGE, 0, 0); // tell the UI thread that a new message is received
			}
			else wt_pfree(task);
		}
	}
#endif
	return WT_OK;
}

static U32 HandleCommonMessage(HWND hWndUI, MemoryPoolContext mempool, U8* primary_key, U8* pubkey_receiver, U8* msg_b64, U32 len_b64)
{
	U32 crc32 = wt_GenCRC32(msg_b64, 89);
	U32 length_raw = wt_b64_dec_len(len_b64 - 89);
	U8* message_raw = (U8*)wt_palloc(mempool, length_raw);

	if (message_raw)
	{
		int rc = wt_b64_decode((const char*)(msg_b64 + 89), len_b64 - 89, (char*)message_raw, length_raw);
		if (rc > 0)
		{
			U8 i;
			U8 Ks[SECRET_KEY_SIZE];
			U8 nonce[12] = { 0 };
			U8 hash0[SHA256_HASH_SIZE] = { 0 };
			U8 hash1[SHA256_HASH_SIZE] = { 0 };
			U8 mask[8] = { 0 };
			AES256_ctx ctxAES = { 0 };
			wt_chacha20_context cxt = { 0 };

			wt_AES256_init(&ctxAES, primary_key);
			wt_AES256_decrypt(&ctxAES, 2, Ks, message_raw); // get the session key at first
			for (i = 0; i < 12; i++) nonce[i] = i;
			wt_chacha20_init(&cxt);
			wt_chacha20_setkey(&cxt, Ks);
			wt_chacha20_starts(&cxt, nonce, 0);
			wt_chacha20_update(&cxt, length_raw - 32, (const unsigned char*)(message_raw + 32), message_raw + 32);  // decrypt the message!
			wt_chacha20_free(&cxt);
			U32 crc32Hash = wt_GenCRC32(message_raw + 32, 32);
			wt_AES256_decrypt(&ctxAES, 2, hash0, message_raw + 32);

			U8* p = message_raw;
			U8* q = (U8*)&crc32Hash; for (i = 0; i < 12; i++) p[64 + i] ^= q[i % 4];
			U8  vs = p[64];
			U8  op = p[65];
			U8  idx = p[66];
			U8  rs = p[67];
			U32 length = *((U32*)(p + 68)); // the length of the original message

			q = p + 76 + idx; // now p is pointing to the real message
			wt_sha256_hash(q, length, hash1);

			if (0 == memcmp(hash0, hash1, 32) && crc32 == *((U32*)(p + 72)))
			{
				// ok, this is a legal message, we record it in out M table
				sqlite3* db;
				//EnterCriticalSection(&g_csSQLiteDB);
				int rc = sqlite3_open16(g_DBPath, &db);
				if (SQLITE_OK == rc)
				{
					sqlite3_stmt* stmt = NULL;
					rc = sqlite3_prepare_v2(db,
						(const char*)"INSERT INTO m(id,st,dt,hs,sd,re,tp,tx) VALUES((?),1,(?),(?),(?),(?),(?),(?))", -1, &stmt, NULL);
					if (SQLITE_OK == rc)
					{
						U8 result = 0;
						U8* s = msg_b64;
						U8 hexHash[65] = { 0 };
						wt_Raw2HexString(hash0, SHA256_HASH_SIZE, hexHash, NULL);
						S64 dt = GetCurrentUTCTime64();
						int mid = InterlockedIncrement(&g_messageId);
						rc = sqlite3_bind_int64(stmt, 1, mid);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_int64(stmt, 2, dt);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 3, (const char*)hexHash, 64, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 4, (const char*)s, 44, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 5, (const char*)(s + 44), 44, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 6, (const char*)(s + 88), 1, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						rc = sqlite3_bind_text(stmt, 7, (const char*)(s + 89), len_b64 - 89, SQLITE_TRANSIENT);
						if (SQLITE_OK != rc) result++;
						if (result == 0)
						{
							rc = sqlite3_step(stmt);
						}
						sqlite3_finalize(stmt);
					}
					sqlite3_close(db);
				}
				//LeaveCriticalSection(&g_csSQLiteDB);

				switch (op)
				{
				case 'T':
					Handle_T_Message(hWndUI, mempool, pubkey_receiver, q, length);
					break;
				case 'F':
					if (length == WT_BLOB_LEN)
					{
						Handle_F_Message(hWndUI, mempool, pubkey_receiver, q, length);
					}
					break;
				default:
					break;
				}
			}
		}
		wt_pfree(message_raw);
	}
	return WT_OK;
}


static void sub_message_callback(struct mosquitto* mosq, void* obj, const struct mosquitto_message* message, const mosquitto_property* properties)
{
	HWND hWndUI;
	MemoryPoolContext pool;
	U8 Kp[32] = { 0 };
	U8 pkSender[33] = { 0 };
	U8 pkReceiver[33] = { 0 };
	MQTT_Conf* conf = (MQTT_Conf*)obj;
	assert(conf);
	pool = conf->mempool;
	assert(pool);

	UNUSED(properties);

	hWndUI = (HWND)conf->hWnd;
	assert(::IsWindow(hWndUI));

	U8* msssage_b64 = (U8*)message->payload;
	U32 length_b64 = (U32)message->payloadlen;

	// do some pre-check at first 
	if (length_b64 < 89 + 140) // this is the minimal size for the packet
		return;
	if (wt_b64_decode((const char*)msssage_b64, 44, (char*)pkSender, PUBLIC_KEY_SIZE) != PUBLIC_KEY_SIZE)
		return;
	if (wt_b64_decode((const char*)(msssage_b64 + 44), 44, (char*)pkReceiver, PUBLIC_KEY_SIZE) != PUBLIC_KEY_SIZE)
		return;
	if (memcmp(pkReceiver, g_myInfo->pubkey, PUBLIC_KEY_SIZE))
		return;
	if (WT_OK != GetKeyFromSecretKeyAndPlubicKey(g_myInfo->seckey, pkSender, Kp))
		return;

	switch (msssage_b64[88])
	{
	case '@':
		HandleCommonMessage(hWndUI, pool, Kp, pkSender, msssage_b64, length_b64);
		break;
	case '#':
		if (length_b64 == 89 + 140)
		{
			HandleQueryMessage(hWndUI, pool, Kp, pkSender, msssage_b64, length_b64);
		}
		break;
	default:
		break;
	}
}

DWORD WINAPI MQTTSubThread(LPVOID lpData)
{
	MemoryPoolContext mempool;
	HWND hWndUI = (HWND)(lpData);
	ATLASSERT(::IsWindow(hWndUI));

	InterlockedIncrement(&g_threadCount);

	mempool = wt_mempool_create("MQTT_SUB_POOL", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);
	if (mempool)
	{
		int rc = 0;
		struct mosquitto* mosq = nullptr;
		char topic[67];

		MQTTConfInit(mempool, hWndUI, false);

		wt_Raw2HexString(g_myInfo->pubkey, PUBLIC_KEY_SIZE, (U8*)topic, nullptr);
		MQTTConfAddSubTopic(topic);

		mosq = mosquitto_new((const char*)g_MQTTSubClientId, false, &confSub);
		if (mosq)
		{
			mosquitto_connect_v5_callback_set(mosq, sub_connect_callback);
			mosquitto_disconnect_v5_callback_set(mosq, sub_disconnect_callback);
			mosquitto_message_v5_callback_set(mosq, sub_message_callback);
			mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);

			while (0 == g_Quit)
			{
				rc = mosquitto_connect_bind_v5(mosq, MQTT_DEFAULT_HOST, MQTT_DEFAULT_PORT, 60, NULL, NULL);
				if (MOSQ_ERR_SUCCESS == rc)
				{
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_GOOD);
					break;
				}
				else
				{
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_BAD); // if we cannot connect, the network is not good
					Sleep(1000); // wait for 1 second to try
				}
			}

			while (0 == g_Quit) // the main loop
			{
				for (;;)
				{
					if (g_Quit)
						break;
					rc = mosquitto_loop(mosq, -1, 1);
					if (rc == MOSQ_ERR_SUCCESS)
					{
						InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_GOOD);
						//PushTaskIntoReceiveMessageQueue(nullptr); // free the task that had been processed.
						//Sleep(100);
						continue;
					}
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_BAD);
					break;
				}

				if (g_Quit)
					break;

				do
				{
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_BAD);
					rc = mosquitto_reconnect(mosq);
				} while (0 == g_Quit && rc != MOSQ_ERR_SUCCESS);

				if (g_Quit)
					break;

				if (MOSQ_ERR_SUCCESS == rc)  // because we can connect successfully, the network is good
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_GOOD);
				else
					InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_BAD);
			}
			mosquitto_disconnect(mosq);
			mosquitto_destroy(mosq);
		}

		MQTTConfTerm(false);
	}

	wt_mempool_destroy(mempool);
	InterlockedDecrement(&g_threadCount);
    return 0;
}

char* Make_AQ_Message(U8 mtype, MemoryPoolContext pool, U8* pubkey_receiver, U8* pubkey, U32* length, U8* hash)
{
	char* message_b64 = nullptr;
	U8 Kp[SECRET_KEY_SIZE] = { 0 };

	assert(hash);

	if (WT_OK == GetKeyFromSecretKeyAndPlubicKey(g_myInfo->seckey, pubkey_receiver, Kp))
	{
		AES256_ctx ctxAES = { 0 };
		wt_chacha20_context cxt = { 0 };
		U32 crc32, i;
		U8 Ks[SECRET_KEY_SIZE] = { 0 };
		U8 nonce[12] = { 0 };
		U8 msg_raw[32 + 1 + 33 + 4 + 32] = { 0 };
		U8 msg_len = 32 + 1 + 33 + 4 + 32;
		U8 legnth_64 = wt_b64_enc_len(msg_len);
		U8 head[89];
		U8* q;

		U8* p = head;
		wt_b64_encode((const char*)g_myInfo->pubkey, PUBLIC_KEY_SIZE, (char*)p, 44);
		wt_b64_encode((const char*)pubkey_receiver, PUBLIC_KEY_SIZE, (char*)(p + 44), 44);
		p[88] = (mtype == 'Q') ? '?' : '$';
		crc32 = wt_GenCRC32(head, 89);

		p = msg_raw;
		wt_GenerateRandom32Bytes(Ks); // genarate the session key, it is a randome number
		wt_AES256_init(&ctxAES, Kp);
		wt_AES256_encrypt(&ctxAES, 2, p, Ks);
		p[32] = mtype;
		memcpy(p + 33, pubkey, PUBLIC_KEY_SIZE);
		q = (U8*)&crc32;
		for (i = 0; i < 4; i++) p[66 + i] = q[i];
		wt_sha256_hash(msg_raw + 32, 1 + 33 + 4, hash);
		wt_AES256_encrypt(&ctxAES, 2, msg_raw + 32 + 1 + 33 + 4, hash);

		wt_chacha20_init(&cxt);
		wt_chacha20_setkey(&cxt, Ks);
		for (i = 0; i < 12; i++) nonce[i] = i;
		wt_chacha20_starts(&cxt, nonce, 0);
		wt_chacha20_update(&cxt, 1 + 33 + 4 + 32, (const unsigned char*)(msg_raw + 32), msg_raw + 32);
		wt_chacha20_free(&cxt);

		message_b64 = (char*)wt_palloc(pool, 89 + legnth_64);
		if (message_b64)
		{
			for (i = 0; i < 89; i++) message_b64[i] = head[i];
			wt_b64_encode((const char*)msg_raw, msg_len, (char*)(message_b64 + 89), legnth_64);
			//message_b64[89 + legnth_64] = 0;
			if (length) *length = 89 + legnth_64;
		}
	}
	return message_b64;
}

#define UT_MIN_PACKET_SIZE	248
char* Make_UT_Message(U8 mtype, MemoryPoolContext pool, U8* pubkey_receiver, U8* msg_org, U32 len_org, U32* length, U8* hash)
{
	char* message_b64 = nullptr;
	U32 length_b64 = 0;
	U8 Kp[SECRET_KEY_SIZE] = { 0 };

	if (WT_OK == GetKeyFromSecretKeyAndPlubicKey(g_myInfo->seckey, pubkey_receiver, Kp))
	{
		U8* message_raw;
		U32 crc32, length_raw, offset;
		U8 head[89];

		wt_b64_encode((const char*)g_myInfo->pubkey, PUBLIC_KEY_SIZE, (char*)head, 44);
		wt_b64_encode((const char*)pubkey_receiver, PUBLIC_KEY_SIZE, (char*)(head + 44), 44);
		head[88] = (mtype == 'T') ? '@' : '^';
		crc32 = wt_GenCRC32(head, 89);

		offset = 0;
		length_raw = len_org + 76; // if the message is equal or more than 252 bytes, we do not make random data
		if (len_org < UT_MIN_PACKET_SIZE) // we keep the minimal size to 252 bytes to make it harder for the attacker
		{
			length_raw = UT_MIN_PACKET_SIZE + 76;
			offset = (U8)(UT_MIN_PACKET_SIZE - len_org);
		}

		message_raw = (U8*)wt_palloc(pool, length_raw);
		if (message_raw)
		{
			U8 idx;
			U32 i, crc32Hash;
			wt_chacha20_context cxt = { 0 };
			AES256_ctx ctxAES = { 0 };
			U8 Ks[32] = { 0 };
			U8 nonce[12] = { 0 };

			assert(hash);
			wt_sha256_hash(msg_org, len_org, hash);
			wt_GenerateRandom32Bytes(Ks); // genarate the session key, it is a randome number

			// the first 32 bytes contain the encrypted session key. Ks : key session
			wt_AES256_init(&ctxAES, Kp);
			wt_AES256_encrypt(&ctxAES, 2, message_raw, Ks);
			// the second 32 bytes contain the encrypted SHA256 of the original message.
			wt_AES256_encrypt(&ctxAES, 2, message_raw + 32, hash);
			crc32Hash = wt_GenCRC32(message_raw + 32, SHA256_HASH_SIZE);

			U8* p = message_raw;
			// handle the next 4 bytes
			p[64] = WT_ENCRYPTION_VERION; // the version byte
			p[65] = mtype; // the data type byte
			idx = wt_GenRandomU8(offset); // a random offset
			p[66] = idx;
			p[67] = 'X'; // reserved byte
			// save the original message length in the next 4 bytes
			U32* p32 = (U32*)(p + 68);
			*p32 = len_org;

			U8* q = (U8*)&crc32; p[72] = q[0]; p[73] = q[1]; p[74] = q[2]; p[75] = q[3];

			// mask the 12 bytes to make it a little bit harder for the attacker :-)
			q = (U8*)&crc32Hash; for (i = 0; i < 12; i++) p[64 + i] ^= q[i % 4];

			if (offset) // the original message is less than 248 bytes, we fill the random data in the 248 bytes
				wt_FillRandomData(p + 76, length_raw - 76);

			// save the original message
			memcpy(p + 76 + idx, msg_org, len_org);

			wt_chacha20_init(&cxt);
			wt_chacha20_setkey(&cxt, Ks); // use the session key as the key for Chacha20 encryption algorithm.
			for (i = 0; i < 12; i++) nonce[i] = i;
			wt_chacha20_starts(&cxt, nonce, 0);
			// encrypt the message except the first 32 bytes, because the first 32 bytes contain the encrypted session key
			wt_chacha20_update(&cxt, length_raw - 32, (const unsigned char*)(message_raw + 32), message_raw + 32);
			wt_chacha20_free(&cxt);

			// we complete the raw message, so conver the raw message to base64 encoding string
			length_b64 = wt_b64_enc_len(length_raw);
			message_b64 = (char*)wt_palloc(pool, 89 + length_b64 + 1);
			if (message_b64)
			{
				for (i = 0; i < 89; i++) message_b64[i] = head[i];
				wt_b64_encode((const char*)message_raw, length_raw, (char*)(message_b64 + 89), length_b64);
				message_b64[89 + length_b64] = 0;
				if (length) *length = 89 + length_b64;
			}
			wt_pfree(message_raw);
		}
	}
	return message_b64;
}

static void pub_connect_callback(struct mosquitto* mosq, void* obj, int result, int flags, const mosquitto_property* properties)
{
	if (result == 0) // we connect successfuly
	{
		U32 length = 0;
		char topic[67] = { 0 };

		MQTT_Conf* conf = (MQTT_Conf*)obj;
		assert(conf);
		MemoryPoolContext pool = conf->mempool;
		assert(pool);

		InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_GOOD); // the network is good

		pub_status = MOSQUITTO_STATUS_CONNACK_RECVD;
		PublishTask* p = (PublishTask*)conf->opaque;
		if (p)
		{
			MessageTask* mt = p->node;
			if (mt)
			{
				length = 0;
				switch (mt->type)
				{
				case 'Q':
				case 'A':
					conf->message = Make_AQ_Message(mt->type, pool, mt->pubkey, mt->message, &length, mt->hash);
					break;
				case 'T':
					conf->message = Make_UT_Message(mt->type, pool, mt->pubkey, p->message, p->msgLen, &length, mt->hash);
					break;
				case 'U':
					conf->message = Make_UT_Message(mt->type, pool, mt->pubkey, mt->message, mt->msgLen, &length, mt->hash);
					break;
				default:
					break;
				}

				if (conf->message && length)
				{
					conf->msglen = length;
					wt_Raw2HexString(mt->pubkey, PUBLIC_KEY_SIZE, (U8*)topic, NULL);
					mosquitto_publish_v5(mosq, NULL, topic, length, conf->message, 1, false, NULL);
				}
			}
		}
	}
	else
	{
		InterlockedExchange(&g_NetworkStatus, WT_NETWORK_IS_BAD); // the network is good
		pub_status = MOSQUITTO_STATUS_NOHOPE;
	}
}

void pub_disconnect_callback(struct mosquitto* mosq, void* obj, int rc, const mosquitto_property* properties)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(properties);
	if (rc == 0)
	{
		pub_status = MOSQUITTO_STATUS_DISCONNECTED;
	}
}

void pub_publish_callback(struct mosquitto* mosq, void* obj, int mid, int reason_code, const mosquitto_property* properties)
{
	MQTT_Conf* conf = (MQTT_Conf*)obj;
	if (conf)
	{
		int i, mid;
		U8 hexHash[65];
		for (i = 0; i < 64; i++) hexHash[i] = 'X';
		hexHash[64] = '\0';

		PublishTask* task = (PublishTask*)conf->opaque;
		assert(task);
		MessageTask* mt = task->node;
		assert(mt);
		wt_Raw2HexString(mt->hash, SHA256_HASH_SIZE, hexHash, NULL);
		mid = mt->sequence;
		InterlockedIncrement(&(mt->state));
		if (conf->message)
		{
			//EnterCriticalSection(&g_csSQLiteDB);
			sqlite3* db;
			int rc = sqlite3_open16(g_DBPath, &db);
			if (SQLITE_OK == rc)
			{
				sqlite3_stmt* stmt = NULL;
				rc = sqlite3_prepare_v2(db,
					(const char*)"INSERT INTO m(id,st,dt,hs,sd,re,tp,tx) VALUES((?),1,(?),(?),(?),(?),(?),(?))", -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					U8 result = 0;
					char* p = conf->message;
					S64 dt = GetCurrentUTCTime64();

					rc = sqlite3_bind_int(stmt, 1, mid);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_int64(stmt, 2, dt);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_text(stmt, 3, (const char*)hexHash, 64, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_text(stmt, 4, (const char*)p, 44, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_text(stmt, 5, (const char*)(p + 44), 44, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_text(stmt, 6, (const char*)(p + 88), 1, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_text(stmt, 7, (const char*)(p + 89), conf->msglen - 89, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					if (result == 0)
					{
						rc = sqlite3_step(stmt);
					}
					sqlite3_finalize(stmt);
				}
				sqlite3_close(db);
			}
			//LeaveCriticalSection(&g_csSQLiteDB);

			wt_pfree(conf->message);
			conf->message = NULL;
			conf->msglen = 0;
		}
		pub_status = MOSQUITTO_STATUS_DONE;
	}
}

DWORD WINAPI MQTTPubThread(LPVOID lpData)
{
	MemoryPoolContext mempool;
	HWND hWndUI = (HWND)(lpData);
	ATLASSERT(::IsWindow(hWndUI));

	InterlockedIncrement(&g_threadCount);

	mempool = wt_mempool_create("MQTT_PUB_POOL", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);
	if (mempool)
	{
		int rc = 0;
		DWORD dwRet;
		struct mosquitto* mosq = nullptr;
		PublishTask* task = nullptr;

		MQTTConfInit(mempool, hWndUI, true);

		ng_seqnumber = wt_GenRandomU32(0x7FFFFFFF);

		while (true)
		{
			dwRet = WaitForSingleObject(g_MQTTPubEvent, 666); // wake-up every 666 ms

			if (g_Quit) // we will quit the whole program
				break;

			if ((dwRet != WAIT_OBJECT_0) && (dwRet != WAIT_TIMEOUT))
				continue;

			task = CheckSendMessageTasks(mempool); // try to pickup one task in the sendout queue

			if (task) // we have the tasks in the queue that need to send out.
			{
				confPub.opaque = task;
				mosq = mosquitto_new((const char*)g_MQTTPubClientId, false, &confPub);
				if (mosq)
				{
					mosquitto_connect_v5_callback_set(mosq, pub_connect_callback);
					mosquitto_disconnect_v5_callback_set(mosq, pub_disconnect_callback);
					mosquitto_publish_v5_callback_set(mosq, pub_publish_callback);
					mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
					pub_status = MOSQUITTO_STATUS_CONNECTING;

					rc = mosquitto_connect_bind_v5(mosq, MQTT_DEFAULT_HOST, MQTT_DEFAULT_PORT, 60, NULL, NULL);
					if (rc == MOSQ_ERR_SUCCESS)
					{
						do
						{
							rc = mosquitto_loop(mosq, 1000, 1);
							if (pub_status == MOSQUITTO_STATUS_DONE || pub_status == MOSQUITTO_STATUS_NOHOPE)
								break;
							//Sleep(100);
						} while (rc == MOSQ_ERR_SUCCESS);
					}
					mosquitto_disconnect(mosq);
					mosquitto_destroy(mosq);
					wt_mempool_reset(mempool); // clean up the memory pool
				}
			}
		}
		MQTTConfTerm();
	}

	wt_mempool_destroy(mempool);
	InterlockedDecrement(&g_threadCount);
    return 0;
}

static const U8 txtUtf8Name[] = { 0xE9,0xBB,0x84,0xE5,0xA4,0xA7,0xE4,0xBB,0x99,0 };
static const U8 txtUf8Motto[] = { 0xE4,0xB8,0x80,0xE5,0x88,0x87,0xE7,0x9A,0x86,0xE6,0x9C,0x89,0xE5,0x8F,0xAF,0xE8,0x83,0xBD,0 };
static const U8 txtUtf8Area[] = { 0xE5,0xAE,0x87,0xE5,0xAE,0x99,0xE8,0xB5,0xB7,0xE7,0x82,0xB9,0 };
static const U8 txtUtf8Source[] = { 0xE7,0xB3,0xBB,0xE7,0xBB,0x9F,0xE8,0x87,0xAA,0xE5,0xB8,0xA6,0 };

U32 GetAccountNumber(int* total)
{
	U32 ret = WT_FAIL;
	int rc;
	bool bAvailabe = false;
	sqlite3* db;
	sqlite3_stmt* stmt = NULL;

	assert(total);

	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (GetFileAttributesExW(g_DBPath, GetFileExInfoStandard, &fileInfo) != 0)
	{
		if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			bAvailabe = true;
	}

	rc = sqlite3_open16(g_DBPath, &db);
	if (SQLITE_OK != rc)
	{
		sqlite3_close(db);
		return WT_SQLITE_OPEN_ERR;
	}

	if (!bAvailabe) // it is the first time to create the DB, so we create the table structure
	{
		U32 bloblen;
		U8* blobuf;
		char sql[SQL_STMT_MAX_LEN + 1] = { 0 };

		*total = 0;
		g_messageId = 0;

		/* create the table at first */
		rc = sqlite3_prepare_v2(db,
			(const char*)"CREATE TABLE c(id INTEGER PRIMARY KEY,tp INTEGER NOT NULL,i0 INTEGER,i1 INTEGER,tx TEXT,bb BLOB)",
			-1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		rc = sqlite3_prepare_v2(db,
			(const char*)"CREATE TABLE k(id INTEGER PRIMARY KEY AUTOINCREMENT,pt INTEGER NOT NULL,at INTEGER DEFAULT 1,dt INTEGER,sk CHAR(64) NOT NULL UNIQUE,ub BLOB)",
			-1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		rc = sqlite3_prepare_v2(db,
			(const char*)"CREATE TABLE p(id INTEGER PRIMARY KEY AUTOINCREMENT,dt INTEGER,at INTEGER DEFAULT 1,me CHAR(64) NOT NULL,pk CHAR(66) NOT NULL,sr TEXT,ub BLOB)",
			-1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		rc = sqlite3_prepare_v2(db,
			(const char*)"CREATE TABLE m(id INTEGER PRIMARY KEY,dt INTEGER,st INTEGER,hs CHAR(64) NOT NULL,sd CHAR(44) NOT NULL,re CHAR(44) NOT NULL,tp CHAR(1),tx TEXT)",
			-1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		rc = sqlite3_prepare_v2(db,
			(const char*)"CREATE TABLE q(id INTEGER PRIMARY KEY AUTOINCREMENT,dt INTEGER,tx TEXT)",
			-1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);
#if 0
		/* insert some initial data into the tables */
		sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "INSERT INTO c(id,tp,tx) VALUES(%d,1,'WoChat')", WT_PARAM_APPLICATION_NAME);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "INSERT INTO c(id,tp,tx) VALUES(%d,1,'www.boobooke.com')", WT_PARAM_SERVER_NAME);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "INSERT INTO c(id,tp,i0) VALUES(%d,0,1883)", WT_PARAM_SERVER_PORT);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);

		sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "INSERT INTO c(id,tp,tx) VALUES(%d,1,'MQTT')", WT_PARAM_NETWORK_PROTOCAL);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK != rc) goto Exit_CheckWoChatDatabase;
		rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc)
		{
			sqlite3_finalize(stmt);
			goto Exit_CheckWoChatDatabase;
		}
		sqlite3_finalize(stmt);
#endif
		// insert the Robot's public key
		bloblen = WT_BLOB_LEN;
		blobuf = (U8*)malloc(bloblen);
		if (blobuf)
		{
			int i;
			U8* imgRobotLarge = nullptr;
			U8* imgRobotSmall = nullptr;
			U32 crc32, wS = 0, hS = 0, wL = 0, hL = 0;

			U8* q = GetRobotPublicKey();
			assert(q);
			U8 hexRobotPK[67] = { 0 };
			wt_Raw2HexString(q, PUBLIC_KEY_SIZE, hexRobotPK, nullptr);

			U8* p = blobuf + 4;
			for (i = 0; i < PUBLIC_KEY_SIZE; i++) p[i] = q[i]; // save the public key of the Robot
			p = blobuf + 4 + PUBLIC_KEY_SIZE;
			// clear the first part of the buffer
			for (i = 0; i < (4 + WT_NAME_MAX_LEN + WT_MOTTO_MAX_LEN + WT_AREA_MAX_LEN); i++) p[i] = 0;

			p = blobuf + 4 + PUBLIC_KEY_SIZE + 4; /* now point to the name field */
			for (i = 0; i < 9; i++) p[i] = txtUtf8Name[i];

			p = blobuf + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN; /* now point to the motto field */
			for (i = 0; i < 18; i++) p[i] = txtUf8Motto[i];

			p = blobuf + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_MOTTO_MAX_LEN; /* now point to the area field */
			for (i = 0; i < 12; i++) p[i] = txtUtf8Area[i];

			/* now point to the small icon field */
			p = blobuf + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_MOTTO_MAX_LEN + WT_AREA_MAX_LEN; 
			imgRobotSmall = (U8*)GetUIBitmap(WT_UI_BMP_AIMSALLICON, &wS, &hS);
			assert(imgRobotSmall);
			assert(wS * hS * 4 == WT_SMALL_ICON_SIZE);

			for (i = 0; i < WT_SMALL_ICON_SIZE; i++) p[i] = imgRobotSmall[i];

			/* now point to the large icon field */
			p = blobuf + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_MOTTO_MAX_LEN + WT_AREA_MAX_LEN + WT_SMALL_ICON_SIZE; 
			imgRobotLarge = (U8*)GetUIBitmap(WT_UI_BMP_AILARGEICON, &wL, &hL);
			assert(imgRobotLarge);
			assert(wL * hL * 4 == WT_LARGE_ICON_SIZE);
			for (i = 0; i < WT_LARGE_ICON_SIZE; i++) p[i] = imgRobotLarge[i];

			crc32 = wt_GenCRC32(blobuf + 4 + PUBLIC_KEY_SIZE, bloblen - 4 - PUBLIC_KEY_SIZE);

			sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "INSERT INTO p(me,pk,dt,sr,ub) VALUES('%s','%s',(?),(?),(?))","XXX", hexRobotPK);
			rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
			if (SQLITE_OK == rc)
			{
				U8 result = 0;
				S64 dt = GetCurrentUTCTime64();
				rc = sqlite3_bind_int64(stmt, 1, dt);
				if (SQLITE_OK != rc) result++;
				rc = sqlite3_bind_text(stmt, 2, (const char*)txtUtf8Source, 12, SQLITE_TRANSIENT);
				if (SQLITE_OK != rc) result++;
				rc = sqlite3_bind_blob(stmt, 3, blobuf, bloblen, SQLITE_TRANSIENT);
				if (SQLITE_OK != rc) result++;
				if (result == 0)
				{
					rc = sqlite3_step(stmt);
					if (SQLITE_DONE == rc)
						ret = WT_OK;
				}
				sqlite3_finalize(stmt);
			}
			free(blobuf);
			blobuf = nullptr;
		}
	}
	else
	{
		rc = sqlite3_prepare_v2(db, (const char*)"SELECT IFNULL(MAX(id),0) FROM m", -1, &stmt, NULL);
		if (SQLITE_OK == rc)
		{
			rc = sqlite3_step(stmt);
			if (SQLITE_ROW == rc)
			{
				g_messageId = (LONG)sqlite3_column_int(stmt, 0);
				sqlite3_finalize(stmt);
				rc = sqlite3_prepare_v2(db, (const char*)"SELECT count(1) FROM k WHERE pt=0 AND at<>0", -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					rc = sqlite3_step(stmt);
					if (SQLITE_ROW == rc)
					{
						*total = sqlite3_column_int(stmt, 0);
						ret = WT_OK;
					}
					sqlite3_finalize(stmt);
				}
			}
			else sqlite3_finalize(stmt);
		}
	}

Exit_CheckWoChatDatabase:
	sqlite3_close(db);
    return ret;
}

int MosquittoInit()
{
    return mosquitto_lib_init();
}

void MosquittoTerm()
{
    mosquitto_lib_cleanup();
}

MessageTask* CreateMyInfoMessage()
{
	MessageTask* mt = (MessageTask*)wt_palloc0(g_messageMemPool, sizeof(MessageTask));
	if (mt)
	{
		mt->msgLen = WT_BLOB_LEN;
		mt->message = (U8*)wt_palloc0(g_messageMemPool, mt->msgLen);
		if (mt->message)
		{
			U32 i, utf8len = 0;
			U8* pkRobot = GetRobotPublicKey();
			assert(pkRobot);
			mt->state = MESSAGE_TASK_STATE_ONE;
			mt->type = 'U';
			for (i = 0; i < PUBLIC_KEY_SIZE; i++) mt->pubkey[i] = pkRobot[i];

			U8* p = mt->message;
			U32* p32 = (U32*)p;
			*p32 = g_myInfo->version;
			for (i = 0; i < PUBLIC_KEY_SIZE; i++) p[4 + i] = g_myInfo->pubkey[i];
			for (i = 0; i < (4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN); i++) p[4 + PUBLIC_KEY_SIZE + i] = 0;
			//p[3] = g_myInfo->sex;
			//p32 = (U32*)(p + 4);
			//*p32 = g_myInfo->dob;
			U8* name  = mt->message + 4 + PUBLIC_KEY_SIZE + 4;
			U8* area  = mt->message + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN;
			U8* motto = mt->message + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN;
			U8* iconS = mt->message + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN;
			U8* iconL = mt->message + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE;

			if (wt_UTF16ToUTF8((U16*)g_myInfo->name, g_myInfo->name_length, nullptr, &utf8len) == WT_OK)
			{
				assert(utf8len < WT_NAME_MAX_LEN);
				wt_UTF16ToUTF8((U16*)g_myInfo->name, g_myInfo->name_length, name, nullptr);
			}
			if (wt_UTF16ToUTF8((U16*)g_myInfo->motto, g_myInfo->motto_length, nullptr, &utf8len) == WT_OK)
			{
				assert(utf8len < WT_NAME_MAX_LEN);
				wt_UTF16ToUTF8((U16*)g_myInfo->motto, g_myInfo->motto_length, motto, nullptr);
			}
			if (wt_UTF16ToUTF8((U16*)g_myInfo->area, g_myInfo->area_length, nullptr, &utf8len) == WT_OK)
			{
				assert(utf8len < WT_NAME_MAX_LEN);
				wt_UTF16ToUTF8((U16*)g_myInfo->area, g_myInfo->area_length, area, nullptr);
			}
			memcpy(iconS, g_myInfo->icon, WT_SMALL_ICON_SIZE);
			memcpy(iconL, g_myInfo->iconLarge, WT_LARGE_ICON_SIZE);
		}
		else
		{
			wt_pfree(mt);
			mt = nullptr;
		}
	}
	return mt;
}

WTFriend* FindFriend(U8* pubkey)
{
	WTFriend* people = nullptr;
	bool found = false;
	assert(pubkey);

	WTGuy* guy = (WTGuy*)hash_search(g_peopleHTAB, pubkey, HASH_FIND, &found);
	if (found)
	{
		assert(guy);
		people = guy->people;
	}
	return people;
}

void PopluateFriendInfo(WTFriend* p, U8* blob, U32 blen)
{
	U8* s;
	U8* robotPK;
	U32* img;
	U32 i, length, utf16len, status;

	assert(p);
	assert(blob);
	assert(blen == WT_BLOB_LEN);

	U8* pk = blob + 4;
	for (i = 0; i < PUBLIC_KEY_SIZE; i++) p->pubkey[i] = pk[i];

	p->version = *((U32*)blob);
	//p->dob = *((U32*)(blob + 4 + PUBLIC_KEY_SIZE + 3 + 1));
	//p->sex = blob[4 + PUBLIC_KEY_SIZE + 3];

	U8* utf8Name  = blob + 4 + PUBLIC_KEY_SIZE + 4;
	U8* utf8Area  = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN;
	U8* utf8Motto = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN;
	U8* iconS     = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN;
	U8* iconL     = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE;

	utf16len = length = 0; s = utf8Name;
	while (*s++)
	{
		length++;
		if (length == WT_NAME_MAX_LEN - 1)
			break;
	}
	status = wt_UTF8ToUTF16(utf8Name, length, nullptr, &utf16len);
	if (WT_OK == status && utf16len < (WT_NAME_MAX_LEN >> 1))
	{
		wt_UTF8ToUTF16(utf8Name, length, (U16*)p->name, nullptr);
		p->name_legnth = utf16len;
	}
	utf16len = length = 0; s = utf8Area;
	while (*s++)
	{
		length++;
		if (length == WT_AREA_MAX_LEN - 1)
			break;
	}
	status = wt_UTF8ToUTF16(utf8Area, length, nullptr, &utf16len);
	if (WT_OK == status && utf16len < (WT_AREA_MAX_LEN >> 1))
	{
		wt_UTF8ToUTF16(utf8Area, length, (U16*)p->area, nullptr);
		p->area_legnth = utf16len;
	}
	utf16len = length = 0; s = utf8Motto;
	while (*s++)
	{
		length++;
		if (length == WT_MOTTO_MAX_LEN - 1)
			break;
	}
	status = wt_UTF8ToUTF16(utf8Motto, length, nullptr, &utf16len);
	if (WT_OK == status && utf16len < (WT_MOTTO_MAX_LEN >> 1))
	{
		wt_UTF8ToUTF16(utf8Motto, length, (U16*)p->motto, nullptr);
		p->motto_legnth = utf16len;
	}

	memcpy(p->icon, iconS, WT_SMALL_ICON_SIZE);

	robotPK = GetRobotPublicKey();
	if (memcmp(p->pubkey, robotPK, PUBLIC_KEY_SIZE) == 0)
		img = GetUIBitmap(WT_UI_BMP_AILARGEICON);
	else
		img = GetUIBitmap(WT_UI_BMP_MYLARGEICON);
	assert(img);

	p->wLarge = p->hLarge = 128;
	if (memcmp(img, iconL, WT_LARGE_ICON_SIZE) == 0)
	{
		p->iconLarge = img;
	}
	else
	{
		p->iconLarge = (U32*)wt_palloc(g_messageMemPool, WT_LARGE_ICON_SIZE);
		if (p->iconLarge)
		{
			p->property |= WT_FRIEND_PROP_LARGEICON_ALLOC;
			memcpy(p->iconLarge, iconL, WT_LARGE_ICON_SIZE);
		}
		else p->iconLarge = img;
	}
}

U32 QueryFriendInformation(U8* pubkey)
{
	U32 ret = WT_FAIL;
	MessageTask* task = (MessageTask*)wt_palloc0(g_messageMemPool, sizeof(MessageTask));
	if (task)
	{
		task->message = (U8*)wt_palloc0(g_messageMemPool, PUBLIC_KEY_SIZE);
		if (task->message)
		{
			int i;
			U8* robot = GetRobotPublicKey();
			assert(robot);
			for (i = 0; i < PUBLIC_KEY_SIZE; i++) task->pubkey[i] = robot[i];
			for (i = 0; i < PUBLIC_KEY_SIZE; i++) task->message[i] = pubkey[i];
			task->msgLen = PUBLIC_KEY_SIZE;
			task->state = MESSAGE_TASK_STATE_ONE;
			task->type = 'A';
			PushTaskIntoSendMessageQueue(task);
			ret = WT_OK;
		}
		else wt_pfree(task);
	}
	return ret;
}

static const U8 defaultTextName[]   = { 0xE5,0xA7,0x93,0xE5,0x90,0x8D,0xE6,0x9C,0xAA,0xE7,0x9F,0xA5,0 };
static const U8 defaultTextMotto[]  = { 0xE6,0x97,0xA0,0xE5,0xBA,0xA7,0xE5,0x8F,0xB3,0xE9,0x93,0xAD,0 };
static const U8 defaultTextArea[]   = { 0xE4,0xB8,0x8D,0xE6,0x98,0x8E,0xE5,0x9C,0xB0,0xE5,0x8C,0xBA,0 };
static const U8 defaultTextSource[] = { 0xE6,0x89,0x8B,0xE5,0x8A,0xA8,0xE6,0xB7,0xBB,0xE5,0x8A,0xA0,0 };

U8* TryToAddNewFriend(U8 source, U8* pubkey)
{
	U8* buf = nullptr;
	sqlite3* db;
	int rc = sqlite3_open16(g_DBPath, &db);
	if (SQLITE_OK == rc)
	{
		int count = 0;
		sqlite3_stmt* stmt = NULL;
		char sql[256] = { 0 };
		U8 hexPK[67] = { 0 };
		U8 hexSK[65] = { 0 };

		wt_Raw2HexString(g_myInfo->skenc, SECRET_KEY_SIZE, hexSK, nullptr);
		wt_Raw2HexString(pubkey, PUBLIC_KEY_SIZE, hexPK, nullptr);
		sprintf_s((char*)sql, 256, "SELECT count(1) FROM p WHERE me='%s' AND pk='%s' AND at<>0", hexSK, hexPK);
		rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
		if (SQLITE_OK == rc)
		{
			rc = sqlite3_step(stmt);
			if (rc == SQLITE_ROW)
				count = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);

		if (count == 0)
		{
			U8* blob = (U8*)malloc(WT_BLOB_LEN);
			if (blob)
			{
				int i;
				U8* p = blob + 4;
				for (i = 0; i < PUBLIC_KEY_SIZE; i++) p[i] = pubkey[i];

				p = blob + 4 + PUBLIC_KEY_SIZE;
				for (i = 0; i < (4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN); i++) p[i] = 0;

				p = blob + 4 + PUBLIC_KEY_SIZE + 4; /* now point to the name field */
				for (i = 0; i < 12; i++) p[i] = defaultTextName[i];
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN; /* now point to the area field */
				for (i = 0; i < 12; i++) p[i] = defaultTextArea[i];
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN; /* now point to the area field */
				for (i = 0; i < 12; i++) p[i] = defaultTextMotto[i];
				
				/* now point to the small icon field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN;
				U8* imgMeSmall = (U8*)GetUIBitmap(WT_UI_BMP_MYSMALLICON);
				assert(imgMeSmall);
				for (i = 0; i < WT_SMALL_ICON_SIZE; i++) p[i] = imgMeSmall[i];

				/* now point to the large icon field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE;
				U8* imgMeLarge = (U8*)GetUIBitmap(WT_UI_BMP_MYLARGEICON);
				assert(imgMeLarge);
				for (i = 0; i < WT_LARGE_ICON_SIZE; i++) p[i] = imgMeLarge[i];
				
				U32 crc32 = wt_GenCRC32(blob + 4 + PUBLIC_KEY_SIZE, WT_BLOB_LEN - 4 - PUBLIC_KEY_SIZE);
				*((U32*)blob) = crc32;

				sprintf_s((char*)sql, 256, "INSERT INTO p(me,pk,dt,sr,ub) VALUES('%s','%s',(?),(?),(?))", hexSK, hexPK);
				rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					U8* utf8Source;
					U8 result = 0;
					S64 dt = GetCurrentUTCTime64();
					rc = sqlite3_bind_int64(stmt, 1, dt);
					if (SQLITE_OK != rc) result++;
					utf8Source = (U8*)defaultTextSource;
					rc = sqlite3_bind_text(stmt, 2, (const char*)utf8Source, 12, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_blob(stmt, 3, blob, WT_BLOB_LEN, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					if (result == 0)
					{
						rc = sqlite3_step(stmt);
						if (SQLITE_DONE == rc)
							buf = blob;
						else
							free(blob);
					}
					sqlite3_finalize(stmt);
				}
			}
		}
		sqlite3_close(db);
	}

	return buf;
}

