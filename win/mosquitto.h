#ifndef _WT_MOSQUITTO_H_
#define _WT_MOSQUITTO_H_

#include <cstdlib>
#include <time.h>
#include "mosquitto/include/mosquitto.h"
#include "mosquitto/include/mqtt_protocol.h"
#include "dui/dui.h"
#include "wt/wt_mempool.h"
#include "wt/wt_utils.h"

#define CLIENT_PUB 1
#define CLIENT_SUB 2
#define CLIENT_RR 3
#define CLIENT_RESPONSE_TOPIC 4

#define UNUSED(A) (void)(A)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XMQTTMessage
{
	char* host;
	int   port;
	char* topic;
	char* message;
	int   msglen;
} XMQTTMessage;

typedef struct MQTTConf
{
	bool pub_or_sub;  /* true = pub, false = sub */
	MemoryPoolContext ctx; // the memory pool of this configuration
	MemoryPoolContext ctxThread; // the memory pool of the sub or pub thread
	void* hWnd; // the window handle of the UI window
	unsigned char* message_buffer;
	char* id;
	char* id_prefix;
	int protocol_version;
	int keepalive;
	char* host;
	int port;
	int qos;
	bool retain;
	int pub_mode; /* pub, rr */
	char* file_input; /* pub, rr */
	char* message; /* pub, rr */
	int msglen; /* pub, rr */
	char* topic; /* pub, rr */
	char* bind_address;
	int repeat_count; /* pub */
	struct timeval repeat_delay; /* pub */
#ifdef WITH_SRV
	bool use_srv;
#endif
	bool debug;
	bool quiet;
	unsigned int max_inflight;
	char* username;
	char* password;
	char* will_topic;
	char* will_payload;
	int will_payloadlen;
	int will_qos;
	bool will_retain;
	bool clean_session;
	char** topics; /* sub, rr */
	int maxtopics; /* maximum topics we can use */
	int topic_count; /* sub, rr */
	bool exit_after_sub; /* sub */
	bool no_retain; /* sub */
	bool retained_only; /* sub */
	bool remove_retained; /* sub */
	char** filter_outs; /* sub */
	int filter_out_count; /* sub */
	char** unsub_topics; /* sub */
	int unsub_topic_count; /* sub */
	bool verbose; /* sub */
	bool eol; /* sub */
	int msg_count; /* sub */
	char* format; /* sub, rr */
	bool pretty; /* sub, rr */
	unsigned int timeout; /* sub */
	int sub_opts; /* sub */
	long session_expiry_interval;
	int random_filter; /* sub */
#ifdef WITH_SOCKS
	char* socks5_host;
	int socks5_port;
	char* socks5_username;
	char* socks5_password;
#endif
	mosquitto_property* connect_props;
	mosquitto_property* publish_props;
	mosquitto_property* subscribe_props;
	mosquitto_property* unsubscribe_props;
	mosquitto_property* disconnect_props;
	mosquitto_property* will_props;
	bool have_topic_alias; /* pub */
	char* response_topic; /* rr */
	bool tcp_nodelay;
} MQTTConf;

typedef struct MQTT_Methods
{
	void (*message_callback)(struct mosquitto*, void*, const struct mosquitto_message*, const mosquitto_property*);
	void (*connect_callback)(struct mosquitto*, void*, int, int, const mosquitto_property*);
	void (*disconnect_callback)(struct mosquitto*, void*, int, const mosquitto_property*);
	void (*subscribe_callback)(struct mosquitto*, void*, int, int, const int*);
	void (*publish_callback)(struct mosquitto*, void* obj, int mid, int reason_code, const mosquitto_property* props);
	void (*log_callback)(struct mosquitto*, void*, int, const char*);
} MQTT_Methods;

#ifdef __cplusplus
}
#endif

namespace MQTT
{
	 int MQTT_Init();
	 int MQTT_Term();
	 struct mosquitto* MQTT_CreateInstance(MemoryPoolContext ctxThread, void* hWnd, char* client_id, char* host, int port, MQTT_Methods* callback, bool isPub);
	 void MQTT_DestroyInstance(struct mosquitto* mq);
	 int MQTT_AddSubTopic(char* topic, bool isPub = false);
	 void MQTT_HookPubMsgBuffer(unsigned char* message_buffer);
#if 0
	 struct mosquitto* MQTT_PubInit(MQTTPrivateData* privatedata, char* host, int port, MQTT_Methods* callback);
	 int MQTT_PubMessage(Mosquitto q, char* topic, char* message, int msglen);
	 int MQTT_PubTerm(Mosquitto q);

	 const char* strerror(int mosq_errno);
	 const char* connack_string(int connack_code);
	 int sub_topic_tokenise(const char* subtopic, char*** topics, int* count);
	 int sub_topic_tokens_free(char*** topics, int count);
	 int lib_version(int* major, int* minor, int* revision);
	 int topic_matches_sub(const char* sub, const char* topic, bool* result);
	 int validate_utf8(const char* str, int len);
	 int subscribe_simple(
		struct mosquitto_message** messages,
		int msg_count,
		bool retained,
		const char* topic,
		int qos = 0,
		const char* host = "localhost",
		int port = 1883,
		const char* client_id = NULL,
		int keepalive = 60,
		bool clean_session = true,
		const char* username = NULL,
		const char* password = NULL,
		const struct libmosquitto_will* will = NULL,
		const struct libmosquitto_tls* tls = NULL);

	 int subscribe_callback(
		int (*callback)(struct mosquitto*, void*, const struct mosquitto_message*),
		void* userdata,
		const char* topic,
		int qos = 0,
		const char* host = "localhost",
		int port = 1883,
		const char* client_id = NULL,
		int keepalive = 60,
		bool clean_session = true,
		const char* username = NULL,
		const char* password = NULL,
		const struct libmosquitto_will* will = NULL,
		const struct libmosquitto_tls* tls = NULL);
#endif
#if 0
	/*
	 * Class: XMqtt
	 *
	 * A mosquitto client class. This is a C++ wrapper class for the mosquitto C
	 * library. Please see mosquitto.h for details of the functions.
	 */
	class  XMqtt 
	{
	private:
		struct mosquitto* m_mosq;
	public:
		XMqtt(const char* id = NULL, bool clean_session = true);
		virtual ~XMqtt();

		int reinitialise(const char* id, bool clean_session);
		int socket();
		int will_set(const char* topic, int payloadlen = 0, const void* payload = NULL, int qos = 0, bool retain = false);
		int will_clear();
		int username_pw_set(const char* username, const char* password = NULL);
		int connect(const char* host, int port = 1883, int keepalive = 60);
		int connect_async(const char* host, int port = 1883, int keepalive = 60);
		int connect(const char* host, int port, int keepalive, const char* bind_address);
		int connect_async(const char* host, int port, int keepalive, const char* bind_address);
		int reconnect();
		int reconnect_async();
		int disconnect();
		int publish(int* mid, const char* topic, int payloadlen = 0, const void* payload = NULL, int qos = 0, bool retain = false);
		int subscribe(int* mid, const char* sub, int qos = 0);
		int unsubscribe(int* mid, const char* sub);
		void reconnect_delay_set(unsigned int reconnect_delay, unsigned int reconnect_delay_max, bool reconnect_exponential_backoff);
		int max_inflight_messages_set(unsigned int max_inflight_messages);
		void message_retry_set(unsigned int message_retry);
		void user_data_set(void* userdata);
		int tls_set(const char* cafile, const char* capath = NULL, const char* certfile = NULL, const char* keyfile = NULL, int (*pw_callback)(char* buf, int size, int rwflag, void* userdata) = NULL);
		int tls_opts_set(int cert_reqs, const char* tls_version = NULL, const char* ciphers = NULL);
		int tls_insecure_set(bool value);
		int tls_psk_set(const char* psk, const char* identity, const char* ciphers = NULL);
		int opts_set(enum mosq_opt_t option, void* value);

		int loop(int timeout = -1, int max_packets = 1);
		int loop_misc();
		int loop_read(int max_packets = 1);
		int loop_write(int max_packets = 1);
		int loop_forever(int timeout = -1, int max_packets = 1);
		int loop_start();
		int loop_stop(bool force = false);
		bool want_write();
		int threaded_set(bool threaded = true);
		int socks5_set(const char* host, int port = 1080, const char* username = NULL, const char* password = NULL);

		// names in the functions commented to prevent unused parameter warning
		virtual void on_connect(int /*rc*/) { return; }
		virtual void on_connect_with_flags(int /*rc*/, int /*flags*/) { return; }
		virtual void on_disconnect(int /*rc*/) { return; }
		virtual void on_publish(int /*mid*/) { return; }
		virtual void on_message(const struct mosquitto_message* /*message*/) { return; }
		virtual void on_subscribe(int /*mid*/, int /*qos_count*/, const int* /*granted_qos*/) { return; }
		virtual void on_unsubscribe(int /*mid*/) { return; }
		virtual void on_log(int /*level*/, const char* /*str*/) { return; }
		virtual void on_error() { return; }
	};
#endif
}

#endif // _WT_MOSQUITTO_H_