/*
 *  common basic data types and constant symbols used in the whole project
 */

#ifndef __WT_WOCHATYPES_H__
#define __WT_WOCHATYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define S8      int8_t
#define S16     int16_t
#define S32     int32_t
#define S64     int64_t

#define U8      uint8_t
#define U16     uint16_t
#define U32     uint32_t
#define U64     uint64_t

typedef unsigned char uint8;	/* == 8 bits */
typedef unsigned short uint16;	/* == 16 bits */
typedef unsigned int uint32;	/* == 32 bits */

typedef signed char int8;		/* == 8 bits */
typedef signed short int16;		/* == 16 bits */
typedef signed int int32;		/* == 32 bits */

typedef long long int int64;

typedef unsigned long long int uint64;

/*
 * Size
 *		Size of any memory resident object, as returned by sizeof.
 */
typedef size_t Size;

// #define WT_IS_BIG_ENDIAN			1

#define WT_ENCRYPTION_VERION		1

/* for Secp256k1 key size */
#define SECRET_KEY_SIZE		32
#define PUBLIC_KEY_SIZE		33

/* sha256 hash values always is 256-bit/32-byte long */
#define SHA256_HASH_SIZE	32

/* MQTT client is has maximum 23 bytes */
#define MQTT_CLIENTID_SIZE	23

//#define MQTT_DEFAULT_HOST	("www.boobooke.com")
#define MQTT_DEFAULT_HOST	("test.mosquitto.org")
#define MQTT_DEFAULT_PORT	1883

#define WT_NETWORK_IS_BAD		0
#define WT_NETWORK_IS_GOOD		1

typedef long LONG;
// a message node in the message queue
typedef struct MessageTask
{
	struct MessageTask* next;
	volatile LONG  state;
	volatile LONG  sequence;
	U8   pubkey[33];
	U8   type;
	U32  value32;
	U8   hash[32]; // the SHA256 hash value of this node
	U8*  message;
	U32  msgLen;
} MessageTask;

#define WT_UI_BITMAP_MAX	128

#define WT_MQTT_MAXTOPICS	256

#define WT_NAME_MAX_LEN		64
#define WT_MOTTO_MAX_LEN	128
#define WT_AREA_MAX_LEN		64
#define WT_FROM_MAX_LEN		64

#define WT_LARGE_ICON_WIDTH  128
#define WT_LARGE_ICON_HEIGHT 128
#define WT_SMALL_ICON_WIDTH  32
#define WT_SMALL_ICON_HEIGHT 32

#define WT_LARGE_ICON_SIZE	(WT_LARGE_ICON_WIDTH * WT_LARGE_ICON_HEIGHT * 4)
#define WT_SMALL_ICON_SIZE	(WT_SMALL_ICON_WIDTH * WT_SMALL_ICON_HEIGHT * 4)

#define WT_BLOB_LEN		(4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE + WT_LARGE_ICON_SIZE)

#define WT_FRIEND_SOURCE_BUILDIN		0
#define WT_FRIEND_SOURCE_ADDMANUALLY	1
#define WT_FRIEND_SOURCE_FROMGROUP		2

#define WT_MESSAGE_FROM_OTHER	0x0000
#define WT_MESSAGE_FROM_ME		0x0001
#define WT_MESSAGE_FROM_CONFIRM	0x0002

#define WT_CHATWINDOW_GAP		32

#define WT_MEMCMP(mem1, mem2, length, result)	\
		do { \
			int i; \
			for(i=0; i<length; i++) if(mem1[i] != mem2[i]) break;\
			result = (i == length); \
		} while(0)


#define WT_MYINFO_LARGEICON_ALLOC	0x0000001  /* the large icon is pointing to an alloced memory block*/

typedef struct WTMyInfo
{
	U32  property;
	U32  version;
	S64  creation_date;
	U32  dob;  // day of birth
	U8   sex;
	U8   name_length;
	U8   motto_length;
	U8   area_length;
	U8   name[WT_NAME_MAX_LEN];
	U8   motto[WT_MOTTO_MAX_LEN];
	U8   area[WT_AREA_MAX_LEN];
	U32* icon128;
	U32* icon32;
	U8   skenc[SECRET_KEY_SIZE]; /* the encrypted secret key! */
	U8   seckey[SECRET_KEY_SIZE];
	U8   pubkey[PUBLIC_KEY_SIZE];
} WTMyInfo;

typedef struct WTChatMessage
{
	struct WTChatMessage* next;
	struct WTChatMessage* prev;
	U16  state;
	U16  idx;
	U16* message;  // real message
	U32  message_length;
	int  height;   // in pixel
	int  width;    // in pixel
	U64  ts;		// the time stamp. 
	U16* name;      // The name of this people      
	U8* obj;        // point to GIF/APNG/Video/Pic etc
} WTChatMessage;

typedef struct WTChatGroup WTChatGroup;

typedef struct WTFriend WTFriend;

typedef struct WTChatGroup
{
	WTChatGroup* next;
	WTChatGroup* prev;
	WTFriend* people;
	void* opaque;       // for chat group, not single people, this is useful
	U16  width;		    // in pixel, the width of window 4
	U16  height;		// in pixel
	U32  total;         // total messages in this group
	U32  unread;		// how many unread messages? if more than 254, use ... 
	U64  ts;			// the time stamp of last message. 
	U16* tsText;	    // the text of time stamp
	U16* lastmsg;	    // the last message of this group
	U16  lastMsgLen;
	U16  lastmsgLen;
	WTChatMessage* msgHead;
	WTChatMessage* msgTail;
} WTChatGroup;

#define WT_FRIEND_PROP_ACITVE			0x00000001
#define WT_FRIEND_PROP_LARGEICON_ALLOC	0x00000002
struct WTFriend
{
	struct WTFriend* next;
	struct WTFriend* prev;
	WTChatGroup* chatgroup;
	U32  property;
	U8 pubkey[33];
	U32  version;   // the information version
	U32  dob;       // date of birth;
	U8   sex;
	U16  member;	// if this friend is group, how many members in this group?
	U8   name[WT_NAME_MAX_LEN];		// the name of this friend
	U8   name_length;
	U8   motto[WT_MOTTO_MAX_LEN];	// the motto of this friend
	U8   motto_length;
	U8   area[WT_AREA_MAX_LEN];		// the area of this friend
	U8   area_length;
	U8   from[WT_FROM_MAX_LEN];		// which method do I get this friend
	U8   from_length;
	U32  icon[WT_SMALL_ICON_WIDTH * WT_SMALL_ICON_HEIGHT];
	U32* icon128;
};

typedef struct WTGuy
{
	U8 pubkey[PUBLIC_KEY_SIZE];
	WTFriend* people;
} WTGuy;

#define SETTING_GENERAL		0
#define SETTING_NETWORK		1
#define SETTING_AIROBOT		2
#define SETTING_ABOUT		3

typedef struct WTSetting
{
	struct WTSetting* next;
	U8   id;
	U16* name;		// the group name
	U16  nameLen;
	U16  height;    // the height of this string in pixel
} WTSetting;

#define WT_SEX_UNKOWN			0
#define WT_MALE					1
#define WT_FEMALE				2

#define WT_CHAR_PASSWD_MAX_LEN	64
#define WT_CHAR_NAME_MAX_LEN	21
#define WT_CHAR_MOTTO_MAX_LEN	42
#define WT_CHAR_AREA_MAX_LEN	21

#define WT_OK					0x00000000		/* Successful result */
#define WT_FAIL					0x00000001

#define WT_SOURCEEXHAUSTED		0x00000002
#define WT_TARGETEXHAUSTED		0x00000003
#define WT_SOURCEILLEGAL		0x00000004
#define WT_SK_GENERATE_ERR		0x00000005
#define WT_PK_GENERATE_ERR		0x00000006
#define WT_UTF16ToUTF8_ERR		0x00000007
#define WT_MEMORY_ALLOC_ERR		0x00000008
#define WT_SQLITE_OPEN_ERR      0x00000009
#define WT_SQLITE_PREPARE_ERR   0x0000000A
#define WT_SQLITE_STEP_ERR      0x0000000B
#define WT_SQLITE_FINALIZE_ERR  0x0000000C
#define WT_MEMORY_POOL_ERROR	0x0000000D
#define WT_DYNAMIC_HASHTAB_ERR	0x0000000E
#define WT_SECP256K1_CTX_ERROR  0x0000000F
#define WT_RANDOM_NUMBER_ERROR  0x00000010
#define WT_DWRITE_METRIC_ERROR	0x00000011
#define WT_HEXSTR_TO_RAW_ERROR  0x00000012

#define WT_PARAM_APPLICATION_NAME			0
#define WT_PARAM_SERVER_NAME				1
#define WT_PARAM_SERVER_PORT				2
#define WT_PARAM_NETWORK_PROTOCAL			3

/* the UI parameters for desktop(Windows,MacOS, Linux) application begin with WT_PARAM_D_ */
#define WT_PARAM_D_WIN0_WIDTH				1

/* the UI parameters for mobile(iOS, Android) application begin with WT_PARAM_M_ */
#define WT_PARAM_M_DUMMY					1

#define WT_ICON_SMALL_SIZE		32
#define WT_ICON_LARGE_SIZE		128

#define WT_UI_BMP_MYLARGEICON				0
#define WT_UI_BMP_MYSMALLICON				1
#define WT_UI_BMP_AILARGEICON				2
#define WT_UI_BMP_AIMSALLICON				3

#define WT_BYTE		uint8_t
#define WT_WORD		uint16_t
#define WT_DWORD	uint32_t
#define WT_LONG		uint32_t
// Define the struct with pack alignment
#pragma pack(push, 1)
typedef struct wtBITMAPFILEHEADER 
{
	WT_WORD  bfType;
	WT_DWORD bfSize;
	WT_WORD  bfReserved1;
	WT_WORD  bfReserved2;
	WT_DWORD bfOffBits;
} WT_BITMAPFILEHEADER;

typedef struct wtBITMAPINFOHEADER
{
	WT_DWORD biSize;
	WT_LONG  biWidth;
	WT_LONG  biHeight;
	WT_WORD  biPlanes;
	WT_WORD  biBitCount;
	WT_DWORD biCompression;
	WT_DWORD biSizeImage;
	WT_LONG  biXPelsPerMeter;
	WT_LONG  biYPelsPerMeter;
	WT_DWORD biClrUsed;
	WT_DWORD biClrImportant;
} WT_BITMAPINFOHEADER;

// Restore the original packing alignment
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* __WT_WOCHATYPES_H__ */
