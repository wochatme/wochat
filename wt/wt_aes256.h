/*********************************************************************
* Copyright (c) 2016 Pieter Wuille                                   *
* Distributed under the MIT software license, see the accompanying   *
* file COPYING or http://www.opensource.org/licenses/mit-license.php.*
**********************************************************************/

#ifndef __WT_AES256_H__
#define __WT_AES256_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdlib.h>

typedef struct 
{
    uint16_t slice[8];
} AES_state;

typedef struct 
{
    AES_state rk[15];
} AES256_ctx;

void wt_AES256_init(AES256_ctx* ctx, const unsigned char* key32);
void wt_AES256_encrypt(const AES256_ctx* ctx, size_t blocks, unsigned char* cipher16, const unsigned char* plain16);
void wt_AES256_decrypt(const AES256_ctx* ctx, size_t blocks, unsigned char* plain16, const unsigned char* cipher16);

#ifdef __cplusplus
}
#endif

#endif /* __WT_AES256_H__ */