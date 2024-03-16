#ifndef __WT_SHA256_H__
#define __WT_SHA256_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "wochatypes.h"

#define WT_SHA256_BLOCK_LENGTH			64
#define WT_SHA256_DIGEST_LENGTH			32
#define WT_SHA256_DIGEST_STRING_LENGTH	(WT_SHA256_DIGEST_LENGTH * 2 + 1)
#define WT_SHA256_SHORT_BLOCK_LENGTH	(WT_SHA256_BLOCK_LENGTH - 8)

typedef struct wt_sha256_ctx
{
	uint32		state[8];
	uint64		bitcount;
	uint8		buffer[WT_SHA256_BLOCK_LENGTH];
} wt_sha256_ctx;

void wt_sha256_init(wt_sha256_ctx* context);
void wt_sha256_update(wt_sha256_ctx* context, const unsigned char* data, size_t len);
void wt_sha256_final(wt_sha256_ctx* context, uint8* digest);
void wt_sha256_hash(const unsigned char* data, U32 length, U8* hash);

#ifdef __cplusplus
}
#endif

#endif /* __WT_SHA256_H__ */

