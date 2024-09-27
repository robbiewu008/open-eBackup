/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Encryption algorithm header file, which is not open to external systems.
 * Author: Luan Shipeng l00171031
 * Create: 2014-10-27
 * History: 2018-10-06 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_SRC_CAC_CACV2_PRI_H
#define KMC_SRC_CAC_CACV2_PRI_H

#include "wsecv2_config.h"
#include "wsecv2_type.h"
#include "sdpv3_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define WSEC_MIN(a, b) ((a) < (b) ? (a) : (b))
/*
 * Padding (CBC) or Tag (GCM) causes the ciphertext to be longer than the plaintext,
 * but Padding does not exceed 32 bytes. In this CBB, the tag does not exceed 32 bytes.
 */
#define CAC_CRYRT_BUFF_PROTECT_SIZE 32

#define WSEC_IS_ENCRYPT_ALGID(id) (CacAlgIdToType(id) == WSEC_ALGTYPE_SYM)
#define WSEC_IS_HASH_ALGID(id)    (CacAlgIdToType(id) == WSEC_ALGTYPE_DIGEST)
#define WSEC_IS_HMAC_ALGID(id)    (CacAlgIdToType(id) == WSEC_ALGTYPE_HMAC)
#define WSEC_IS_PBKDF_ALGID(id)   (CacAlgIdToType(id) == WSEC_ALGTYPE_PBKDF)

/* Cryptographic algorithms invoked internally: random number/encryption/decryption/hash/derivation/HMAC */
WsecUint32 CacAlgIdToType(WsecUint32 algId);

/* Initializing and Uninit the RNG */
unsigned long CacInitRng(void);
void CacUnInitRng(void);

/* Generates a random number. The caller must ensure the validity of the input parameter. */
unsigned long CacRandom(WsecVoid *buff, WsecUint32 buffLen);

/* Hash Algorithm Implementation */
unsigned long CacDigest(WsecUint32 algId,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *digestBuff, WsecUint32 *digestLen);

/* Initialize the hash operation, transfer the ID, and obtain the handle. */
unsigned long CacDigestInit(WsecHandle *ctx, WsecUint32 algId);

/* The input data is hashed. The data can be input multiple times. */
unsigned long CacDigestUpdate(const WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen);

/* End the hash operation and obtain the hash result. */
unsigned long CacDigestFinal(WsecHandle *ctx, WsecVoid *digestBuff, WsecUint32 *buffLen);

/* Releases hash handles. */
WsecVoid CacDigestReleaseCtx(WsecHandle *ctx);

/* Calculates the input data based on the specified algorithm, key, and HMAC and returns the calculation result. */
unsigned long CacHmac(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *hmacBuff, WsecUint32 *hmacLen);

/* Initialize the HMAC, transfer the ID and key, and obtain the handle. */
unsigned long CacHmacInit(WsecHandle *ctx,
    WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen);

/* The input data is HMAC. The data can be input for multiple times. */
unsigned long CacHmacUpdate(WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen);

/* End the HMAC operation and obtain the HAMC result. */
unsigned long CacHmacFinal(WsecHandle *ctx, WsecVoid *hmacBuff, WsecUint32 *hmacLen);

/* Releases the HMAC handle. */
WsecVoid CacHmacReleaseCtx(WsecHandle *ctx);

/* Obtains the HMAC length based on the HMAC algorithm ID. */
WsecUint32 CacHMACSize(WsecUint32 algId);

/* Specify the algorithm ID, password, salt value, and iteration times to obtain the derived key. */
unsigned long CacPbkdf2(WsecUint32 kdfAlg,
    const WsecVoid *kdfPassword, WsecUint32 passwordLen,
    const Pbkdf2ParamConst *pbkdf2Param,
    WsecUint32 deriveKeyLen, WsecVoid *derivedKey);

/* Specify the algorithm ID, key, IV, and plain text, and encrypt them to obtain the cipher text. */
unsigned long CacEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen);

/* Specify the algorithm ID, key, IV, and ciphertext, and decrypt them to obtain the plaintext. */
unsigned long CacDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen);

/* Obtain the block length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmBlockSize(WsecUint32 algId);

/* Obtains the IV length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmIvLen(WsecUint32 algId);

/* Obtain the key length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmKeyLen(WsecUint32 algId);

/* Implementation of the AES_GCM Encryption Algorithm Interface */
unsigned long CacEncryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *plaintext, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen);

/* AES_GCM decryption algorithm interface implementation */
unsigned long CacDecryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen,
    unsigned char *plaintext, WsecUint32 *plaintextLen);

/* Symmetric stream encryption (start) */
unsigned long CacEncryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen);

/* Symmetric stream encryption (add) */
unsigned long CacEncryptUpdate(WsecHandle ctx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen);

/* Symmetric stream encryption (end) */
unsigned long CacEncryptFinal(WsecHandle *ctx, unsigned char *ciphertext, WsecUint32 *cipherLen);

/* Symmetric decryption of stream data (start) */
unsigned long CacDecryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen);

/* Symmetric decryption of stream data (add) */
unsigned long CacDecryptUpdate(WsecHandle ctx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen);

/* Symmetric decryption of stream data (end) */
unsigned long CacDecryptFinal(WsecHandle *ctx, unsigned char *plaintext, WsecUint32 *plainLen);

/* Releasing the Decryption Environment */
void CacCipherFree(WsecHandle *ctx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_CAC_CACV2_PRI_H */
