/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef _CRYPTO_SM2_H
#define _CRYPTO_SM2_H

#include <crypto/sm3.h>
#include <crypto/akcipher.h>


#define SM2_DEFAULT_USERID "1234567812345678"
#define SM2_DEFAULT_USERID_LEN 16

extern int sm2_compute_z_digest(struct crypto_akcipher *tfm,
			const unsigned char *id, size_t id_len,
			unsigned char dgst[SM3_DIGEST_SIZE]);

#endif 
