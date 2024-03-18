#ifndef __WT_WOCHAT_H__
#define __WT_WOCHAT_H__

#include "wochatypes.h"

#include "dui/dui.h"
#include "wt/wt_mempool.h"
#include "wt/wt_hash.h"
#include "wt/wt_utils.h"
#include "wt/wt_base64.h"
#include "wt/wt_sha256.h"
#include "wt/wt_aes256.h"
#include "wt/wt_chacha20.h"
#include "wt/wt_unicode.h"
#include "sqlite/sqlite3.h"

extern volatile LONG  g_threadCount;
extern volatile LONG  g_Quit;
extern volatile LONG  g_NetworkStatus;
extern volatile LONG  g_messageId;
//extern volatile LONG  g_UpdateMyInfo;

extern DWORD             g_dwMainThreadID;
extern U32		         g_messageSeconds;

extern WTMyInfo*         g_myInfo;

extern HCURSOR			 g_hCursorWE;
extern HCURSOR			 g_hCursorNS;
extern HINSTANCE         g_hInstance;
extern wchar_t           g_AppPath[MAX_PATH + 1];
extern wchar_t           g_DBPath[MAX_PATH + 1];

extern HWND	 g_hWndShareScreen;
extern HWND	 g_hWndChatHistory;
extern HWND	 g_hWndAudioCall;
extern HWND  g_hWndSearchAll;

extern HANDLE             g_MQTTPubEvent;

extern CRITICAL_SECTION    g_csMQTTSub;
extern CRITICAL_SECTION    g_csMQTTPub;
extern CRITICAL_SECTION    g_csSQLiteDB;

extern ID2D1Factory* g_pD2DFactory;
extern IDWriteFactory* g_pDWriteFactory;

#define WT_TEXTFORMAT_MAINTEXT		0
#define WT_TEXTFORMAT_TITLE			1
#define WT_TEXTFORMAT_GROUPNAME		2
#define WT_TEXTFORMAT_ENGLISHSMALL	3
#define WT_TEXTFORMAT_PEOPLENAME	4
#define WT_TEXTFORMAT_INPUTMESSAGE	5
#define WT_TEXTFORMAT_USERNAME		6
#define WT_TEXTFORMAT_OTHER0		7
#define WT_TEXTFORMAT_ENGLISHBIG	8
#define WT_TEXTFORMAT_CHINESEBIG	9
#define WT_TEXTFORMAT_OTHER3		10
#define WT_TEXTFORMAT_OTHER4		11
#define WT_TEXTFORMAT_OTHER5		12
#define WT_TEXTFORMAT_OTHER6		13
#define WT_TEXTFORMAT_OTHER7		14
#define WT_TEXTFORMAT_OTHER8		15
#define WT_TEXTFORMAT_TOTAL			16

#define WM_MQTT_PUBMESSAGE		(WM_USER + 300)
#define WM_MQTT_SUBMESSAGE		(WM_USER + 301)
#define WM_LOADPERCENTMESSAGE	(WM_USER + 302)
#define WM_ALLOTHER_MESSAGE		(WM_USER + 303)

#define WM_WIN_MAINUITHREAD		(WM_USER + 400)
#define WM_WIN_SCREENTHREAD		(WM_USER + 401)
#define WM_WIN_CHATHISTHREAD	(WM_USER + 402)
#define WM_WIN_AUDIOTHREAD		(WM_USER + 403)
#define WM_WIN_SEARCHALLTHREAD	(WM_USER + 404)

#define WM_WIN_BRING_TO_FRONT	(WM_USER + 500)

#define WIN4_GET_PUBLICKEY		1
#define WIN5_UPDATE_MESSAGE		2

#define DLG_ADD_FRIEND			0


#define MESSAGE_TASK_STATE_NULL		0
#define MESSAGE_TASK_STATE_ONE		1
#define MESSAGE_TASK_STATE_COMPLETE	2

extern MessageTask* g_PubQueue;
extern MessageTask* g_SubQueue;

extern int g_win4Width;
extern int g_win4Height;

U32 GetAccountNumber(int* total);
bool DoLogin();
bool DoRegistration();

U8* TryToAddNewFriend(U8 source, U8* pubkey);

U32 PushTaskIntoSendMessageQueue(MessageTask* message_task);
U32 GetReceiverPublicKey(void* parent, U8* pk);

U8* GetRobotPublicKey();

void PopluateFriendInfo(WTFriend* p, U8* blob, U32 blen, U8* utf8Source = nullptr, U8 slen = 0);

void CopyPublicKey(U8* pubkey);

U32 QueryFriendInformation(U8* pubkey);

// send a confirmation to the sender
//int SendConfirmationMessage(U8* pk, U8* hash);
//int DoWoChatLoginOrRegistration(HINSTANCE hInstance);
U32 CreateNewAccount(U16* name, U8 nlen, U16* pwd, U8 plen, U32* skIdx);
U32 OpenAccount(U32 idx, U16* pwd, U32 len);

MessageTask* CreateMyInfoMessage();

S64 GetCurrentUTCTime64();

U32* GetUIBitmap(U8 idx, U32* width = nullptr, U32* height = nullptr);

IDWriteTextFormat* GetTextFormatAndHeight(U8 idx, U16* height = nullptr);

U32 GetTextWidthAndHeight(U8 fontIdx, U16* text, U32 length, int width, int* w, int* h);

int  MosquittoInit();
void MosquittoTerm();

WTFriend* FindFriend(U8* pubkey);

// the background threads
DWORD WINAPI LoadingDataThread(LPVOID lpData);
DWORD WINAPI MQTTSubThread(LPVOID lpData);
DWORD WINAPI MQTTPubThread(LPVOID lpData);

wchar_t* GetMyPublicKeyString();
U32 SaveMyInformation();

#endif // __WT_WOCHAT_H__