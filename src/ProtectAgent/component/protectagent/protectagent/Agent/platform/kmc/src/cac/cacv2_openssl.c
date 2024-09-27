/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: basic encryption function, which adapts to the OPENSSL interface.
 * Author: Luan Shipeng l00171031
 * Create: 2014-10-27
 * Notes: The OpenSSL version must be 1.0.1j.
 * History: 2018-10-06 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

/*
 *  When the iPSI is used, an alarm is generated if the Windows W4 file is empty.
 * If only the header file is contained, the alarm can be cleared.
 * However, the PCLint reports the 766 alarm.
 */
#ifdef WSEC_COMPILE_CAC_OPENSSL
#include "cacv2_pri.h"
#include "securec.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "openssl/aes.h"
#include "openssl/hmac.h"
#include "wsecv2_mem.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_lock.h"
#include "wsecv2_util.h"

#define OPENSSL_RESEED_LENGTH 64
#define CAC_ALGID_UNKNOWN 0

/* Cac context */
typedef struct {
    WsecHandle ctx;
    WsecUint32 algID;
} CacCtx;

/* Random number */
static WsecVoid ThreadLockRand(void)
{
    WsecThreadLockById(LOCK4KMC_RAND);
}

/* Random number */
static WsecVoid ThreadUnlockRand(void)
{
    WsecThreadUnlockById(LOCK4KMC_RAND);
}

/* Releases the digest algorithm ctx. */
static WsecVoid OpensslDigestFree(WsecHandle *ctx)
{
    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }
    EVP_MD_CTX_destroy((EVP_MD_CTX *)(*ctx));
    *ctx = NULL;
}

/* This command is used to release the CTX of the HMAC algorithm. */
static WsecVoid OpensslHmacFree(WsecHandle *ctx)
{
    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }
#ifdef WSEC_USE_OPENSSL_110
    HMAC_CTX_free((HMAC_CTX *)(*ctx));
#else
    HMAC_CTX_cleanup((HMAC_CTX *)(*ctx));
    WSEC_FREE(*ctx);
#endif
    *ctx = NULL;
}

/* Releasing the Decryption Environment */
void CacCipherFree(WsecHandle *ctx)
{
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }
    cacCtx = (CacCtx *)(*ctx);
    if (cacCtx->ctx != NULL) {
        EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)cacCtx->ctx);
        cacCtx->ctx = NULL;
    }
    WSEC_FREE(cacCtx);
    *ctx = NULL;
}

/* Converts the key management module ID to the OpenSSL ID. */
static const EVP_MD *OpensslDigestAlgToOpensslAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_SHA256):
            return EVP_sha256();
        case (WSEC_ALGID_SHA384):
            return EVP_sha384();
        case (WSEC_ALGID_SHA512):
            return EVP_sha512();
        default:
            return NULL;
    }
}

/* Converts the key management module ID to the OpenSSL ID. */
static const EVP_MD *OpensslHmacAlgToOpensslAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_HMAC_SHA256):
            return EVP_sha256();
        case (WSEC_ALGID_HMAC_SHA384):
            return EVP_sha384();
        case (WSEC_ALGID_HMAC_SHA512):
            return EVP_sha512();
        default:
            return NULL;
    }
}

/* Converts the key management module ID to the OpenSSL ID. */
static const EVP_CIPHER *OpensslSymmAlgToOpensslAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_AES128_CBC):
            return EVP_aes_128_cbc();
        case (WSEC_ALGID_AES256_CBC):
            return EVP_aes_256_cbc();
        case (WSEC_ALGID_AES128_GCM):
            return EVP_aes_128_gcm();
        case (WSEC_ALGID_AES256_GCM):
            return EVP_aes_256_gcm();
        case (WSEC_ALGID_SM4_CBC):
            return EVP_sm4_cbc();
        case (WSEC_ALGID_SM4_CTR):
            return EVP_sm4_ctr();
        default:
            return NULL;
    }
}

/* Converts the key management module ID to the OpenSSL ID. */
static const EVP_MD *OpensslPbkdf2AlgToOpensslAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_PBKDF2_HMAC_SHA256):
            return EVP_sha256();
        case (WSEC_ALGID_PBKDF2_HMAC_SHA384):
            return EVP_sha384();
        case (WSEC_ALGID_PBKDF2_HMAC_SHA512):
            return EVP_sha512();
        default:
            return NULL;
    }
}

/* The AES_GCM encryption algorithm interface is implemented using OpenSSL. */
unsigned long CacEncryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *plaintext, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int len;

    WSEC_UNREFER(keyLen);
    WSEC_ASSERT(algType == WSEC_ALGID_AES128_GCM || algType == WSEC_ALGID_AES256_GCM);
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed on GCM");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(ctx, OpensslSymmAlgToOpensslAlg(algType), NULL, NULL, NULL) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx init on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)ivLen, NULL) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx ctrl on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(ctx, NULL, NULL, (const unsigned char *)key, (const unsigned char *)iv) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx init on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptUpdate(ctx, (unsigned char *)ciphertext, &len,
        (const unsigned char *)plaintext, (int)plaintextLen) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx Update on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ciphertextLen = (WsecUint32)len;

    if (EVP_EncryptFinal_ex(ctx, ((unsigned char *)ciphertext) + *ciphertextLen, &len) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx Final on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ciphertextLen += (WsecUint32)len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)tagLen, tag) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx ctrl on GCM to get tag");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    EVP_CIPHER_CTX_free(ctx);
    return WSEC_SUCCESS;
}

/* Implements the AES_GCM decryption algorithm interface in OpenSSL. */
unsigned long CacDecryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen,
    unsigned char *plaintext, WsecUint32 *plaintextLen)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int len;
    int ret;

    WSEC_UNREFER(keyLen);

    WSEC_ASSERT(algType == WSEC_ALGID_AES128_GCM || algType == WSEC_ALGID_AES256_GCM);

    if (tagLen < WSEC_AES_GCM_TAGLEN) {
        WSEC_LOG_E("Wrong Encryption input tag Length for AES_GCM oper");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed on GCM");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(ctx, OpensslSymmAlgToOpensslAlg(algType), NULL, NULL, NULL) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx init on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)ivLen, NULL) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx ctrl on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx init on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &len, ciphertext, (int)ciphertextLen) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx Update on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plaintextLen = (WsecUint32)len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, WSEC_AES_GCM_TAGLEN, tag) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx ctrl on GCM");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = EVP_DecryptFinal_ex(ctx, (unsigned char *)plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);
    if (ret != 1) {
        WSEC_LOG_E("Failed when the Decryption Final");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plaintextLen += (WsecUint32)len;
    return WSEC_SUCCESS;
}

static const EVP_CIPHER *GetCipherAlg(WsecUint32 algId, WsecUint32 keyLen, WsecUint32 ivLen)
{
    if (ivLen != CacSymmIvLen(algId)) {
        WSEC_LOG_E("Wrong IV len.");
        return NULL;
    }
    if (keyLen != CacSymmKeyLen(algId)) {
        WSEC_LOG_E("Wrong Keylen.");
        return NULL;
    }
    return OpensslSymmAlgToOpensslAlg(algId);
}

/* Execute CBC encryption. */
static unsigned long DoCbcEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    const EVP_CIPHER *alg = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    int len = (int)(*ciphertextLen);
    WsecUint32 blockSize = CacSymmBlockSize(algId);
    int totalLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    alg = GetCipherAlg(algId, keyLen, ivLen);
    if (alg == NULL) {
        WSEC_LOG_E("Wrong algId.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (*ciphertextLen < (blockSize > 0 ? ((plaintextLen / blockSize + 1) * blockSize) : plaintextLen)) {
        WSEC_LOG_E("The Input ciphertext buffer len is not enough on CBC");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    /* Create an encryption context. */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed on CBC");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(ctx, alg, NULL, (const unsigned char *)key, (const unsigned char *)iv) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx init on CBC");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptUpdate(ctx, (unsigned char *)ciphertext, &len,
        (const unsigned char *)plaintext, (int)plaintextLen) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx Update on CBC");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    totalLen = len;
    if ((int)(*ciphertextLen) < totalLen) {
        WSEC_LOG_E("The Input ciphertext buffer len is not enough on CBC after encrypt.");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    len = (int)(*ciphertextLen) - totalLen; /* soter 554 */

    /* End of context encryption completion */
    if (EVP_EncryptFinal_ex(ctx, ((unsigned char *)ciphertext) + totalLen, &len) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx Final on CBC");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ciphertextLen = (WsecUint32)(totalLen + len);
    /* Clear the context. */
    EVP_CIPHER_CTX_free(ctx);

    return WSEC_SUCCESS;
}

/* Use OpenSSL to encrypt the password and input the algorithm ID, key, IV, plaintext, and ciphertext. */
static unsigned long OpensslEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    WsecUint32 tmpCipherLen;
    unsigned long ret;

    if (key == NULL || iv == NULL || plaintext == NULL || ciphertext == NULL || ciphertextLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (keyLen != CacSymmKeyLen(algId)) {
            WSEC_LOG_E("Wrong Encryption key len.");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }

        if (*ciphertextLen < plaintextLen + WSEC_AES_GCM_TAGLEN) {
            return WSEC_ERR_INVALID_ARG;
        }
        tmpCipherLen = plaintextLen;
        ret = CacEncryptAesGcm(algId, (const unsigned char *)key, keyLen,
            (const unsigned char *)iv, ivLen, (const unsigned char *)plaintext, plaintextLen,
            (unsigned char *)ciphertext, &tmpCipherLen,
            (unsigned char *)ciphertext + plaintextLen, WSEC_AES_GCM_TAGLEN);
        if (ret != WSEC_SUCCESS || tmpCipherLen != plaintextLen) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *ciphertextLen = plaintextLen + WSEC_AES_GCM_TAGLEN;
        return WSEC_SUCCESS;
    }

    return DoCbcEncrypt(algId, key, keyLen, iv, ivLen, plaintext, plaintextLen, ciphertext, ciphertextLen);
}

/* Run the CBC command to decrypt OpenSSL. */
static unsigned long DoCbcDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    const EVP_CIPHER *alg = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    int len = (int)*plaintextLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    int totalLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    alg = GetCipherAlg(algId, keyLen, ivLen);
    if (alg == NULL) {
        WSEC_LOG_E("Wrong algId.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if ((WsecUint32)len < ciphertextLen + CacSymmBlockSize(algId)) {
        WSEC_LOG_E("The Input plaintext buffer len is not enough.");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    /* Creates a decryption context. */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(ctx, alg, NULL, (const unsigned char *)key, (const unsigned char *)iv) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx init.");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &len,
        (const unsigned char *)ciphertext, (int)ciphertextLen) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx Update.");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    totalLen = len;
    if ((int)*plaintextLen < totalLen) {
        WSEC_LOG_E("The input plaintext buffer len is not enough after decrypt.");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    len = (int)*plaintextLen - totalLen;    /* soter 554 */

    if (EVP_DecryptFinal_ex(ctx, ((unsigned char *)plaintext) + totalLen, &len) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx Final.");
        EVP_CIPHER_CTX_free(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *plaintextLen = (WsecUint32)(totalLen + len);

    /* Clears the created context. */
    EVP_CIPHER_CTX_free(ctx);

    return WSEC_SUCCESS;
}

/* Use OpenSSL to decrypt the password and input the algorithm ID, key, IV, and ciphertext to obtain the plaintext. */
static unsigned long OpensslDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    unsigned char tag[WSEC_AES_GCM_TAGLEN];
    const unsigned char *tagInCipher = NULL;

    if (key == NULL || iv == NULL || ciphertext == NULL || plaintext == NULL || plaintextLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (keyLen != CacSymmKeyLen(algId)) {
            WSEC_LOG_E("Wrong Encryption Keylen");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        if (ciphertextLen < WSEC_AES_GCM_TAGLEN) {
            return WSEC_ERR_INVALID_ARG;
        }
        tagInCipher = (const unsigned char *)ciphertext + ciphertextLen - WSEC_AES_GCM_TAGLEN;
        if (memcpy_s(tag, (size_t)WSEC_AES_GCM_TAGLEN, tagInCipher, (size_t)WSEC_AES_GCM_TAGLEN) != EOK) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        return CacDecryptAesGcm(algId, (const unsigned char *)key, keyLen,
            (const unsigned char *)iv, ivLen,
            (const unsigned char *)ciphertext, ciphertextLen - WSEC_AES_GCM_TAGLEN,
            tag, (WsecUint32)sizeof(tag),
            (unsigned char *)plaintext, plaintextLen);
    }

    return DoCbcDecrypt(algId, key, keyLen, iv, ivLen, ciphertext, ciphertextLen, plaintext, plaintextLen);
}

/* Initializing the RNG */
unsigned long CacInitRng(void)
{
    return WSEC_SUCCESS;
}

void CacUnInitRng(void)
{
    /* empty function for openssl */
}

static int CacGetRandBytes(unsigned char *buff, int buffLen)
{
    /* RAND_priv_bytes added since OpenSSL 1.1.1 */
#if (OPENSSL_VERSION_NUMBER >= 0x10101000L)
    return RAND_priv_bytes(buff, buffLen);
#else
    return RAND_bytes(buff, buffLen);
#endif
}

/* Generate a random number. */
unsigned long CacRandom(WsecVoid *buff, WsecUint32 buffLen)
{
    int returnCode; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecBool ret = WSEC_FALSE;
    static WsecUint32 counter = 0;
    unsigned char *entropy = NULL;

    WSEC_ASSERT(buff != NULL);

    ThreadLockRand();
    ret = WsecGetRandomNumber((unsigned char *)buff, (size_t)buffLen);
    if (ret == WSEC_TRUE) {
        ThreadUnlockRand();
        return WSEC_SUCCESS;
    }

    if ((counter++ % (1 << 23)) == 0) { /* Seed replenishment period: 2^23 */
        ret = WsecGetEntropy(&entropy, (size_t)OPENSSL_RESEED_LENGTH);
        if (ret == WSEC_FALSE) {
            ThreadUnlockRand();
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        RAND_seed(entropy, OPENSSL_RESEED_LENGTH);
        WsecCleanupEntropy(entropy, (size_t)OPENSSL_RESEED_LENGTH);
    }
    /* In OpenSSL mode, set the OpenSSL to support multiple threads. */
    returnCode = CacGetRandBytes((unsigned char *)buff, (int)buffLen);
    ThreadUnlockRand();
    if (returnCode != 1) {
        WSEC_LOG_E1("Call CacGetRandBytes failed %d", returnCode);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Viewing the algorithm type */
WsecUint32 CacAlgIdToType(WsecUint32 algId)
{
    if (OpensslSymmAlgToOpensslAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_SYM;
    }

    if (OpensslHmacAlgToOpensslAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_HMAC;
    }

    if (OpensslPbkdf2AlgToOpensslAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_PBKDF;
    }

    if (OpensslDigestAlgToOpensslAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_DIGEST;
    }

    return WSEC_ALGTYPE_UNKNOWN;
}

/* Hash Algorithm Implementation */
unsigned long CacDigest(WsecUint32 algId,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *digestBuff, WsecUint32 *digestLen)
{
    const EVP_MD *alg = NULL;
    int ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(digestLen != NULL);

    alg = OpensslDigestAlgToOpensslAlg(algId);
    if (alg == NULL) {
        WSEC_LOG_E("Wrong Digest algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = EVP_Digest(data, (size_t)dataLen, (unsigned char *)digestBuff, digestLen, alg, NULL);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_digest failed %d", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* Initialize the hash operation, transfer the ID, and obtain the handle. */
unsigned long CacDigestInit(WsecHandle *ctx, WsecUint32 algId)
{
    const EVP_MD *alg = NULL;
    int ret = 0;
    alg = OpensslDigestAlgToOpensslAlg(algId);

    WSEC_ASSERT(ctx != NULL);

    if (alg == NULL) {
        WSEC_LOG_E("Wrong Digest algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = EVP_MD_CTX_create();
    if (*ctx != NULL) {
        ret = EVP_DigestInit_ex((EVP_MD_CTX *)(*ctx), alg, NULL);
    }

    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        EVP_MD_CTX_destroy((EVP_MD_CTX *)(*ctx));
        WSEC_LOG_E1("EVP_DigestInit_ex failed %d", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* The input data is hashed. The data can be input multiple times. */
unsigned long CacDigestUpdate(const WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen)
{
    int ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    ret = EVP_DigestUpdate((EVP_MD_CTX *)ctx, data, (size_t)dataLen);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_DigestUpdate failed %d", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* End the hash operation and obtain the hash result. */
unsigned long CacDigestFinal(WsecHandle *ctx, WsecVoid *digestBuff, WsecUint32 *buffLen)
{
    int ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(buffLen != NULL);

    if ((*ctx) == NULL) {
        WSEC_LOG_E("CacDigestFinal ctx is NULL");
        return WSEC_ERR_INVALID_ARG;
    }

    ret = EVP_DigestFinal_ex((EVP_MD_CTX *)(*ctx), (unsigned char *)digestBuff, (WsecUint32 *)buffLen);
    OpensslDigestFree(ctx);
    if (ret != 1) {
        WSEC_LOG_E1("EVP_DigestFinal_ex failed %d", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Releases hash handles. */
WsecVoid CacDigestReleaseCtx(WsecHandle *ctx)
{
    unsigned char digest[SDP_DIGEST_MAX_SIZE] = {0};
    WsecUint32 len = sizeof(digest);

    WSEC_ASSERT(ctx != NULL);

    if ((*ctx) == NULL) {
        return;
    }

    WSEC_UNREFER(EVP_DigestFinal_ex((EVP_MD_CTX *)(*ctx), digest, &len));
    OpensslDigestFree(ctx);
}

/* Obtains the HMAC length based on the HMAC algorithm ID. */
WsecUint32 CacHMACSize(WsecUint32 algId)
{
    if (OpensslHmacAlgToOpensslAlg(algId) != 0) {
        return (WsecUint32)EVP_MD_size(OpensslHmacAlgToOpensslAlg(algId));
    } else {
        return 0;
    }
}

/* Calculates the input data based on the specified algorithm, key, and HMAC and returns the calculation result. */
unsigned long CacHmac(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    const EVP_MD *alg = NULL;
    WsecVoid *hmacData = NULL;

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

    alg = OpensslHmacAlgToOpensslAlg(algId);
    if (alg == NULL) {
        WSEC_LOG_E("Wrong hmac algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    hmacData = HMAC(alg, key, (int)keyLen, (const unsigned char *)data, (size_t)dataLen,
        (unsigned char *)hmacBuff, hmacLen);
    if (hmacData != NULL) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E("HMAC failed");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* Initialize the HMAC, transfer the ID and key, and obtain the handle. */
unsigned long CacHmacInit(WsecHandle *ctx,
    WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen)
{
    const EVP_MD *alg = NULL;
    HMAC_CTX *hmacCtx = NULL;

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(key != NULL);

    alg = OpensslHmacAlgToOpensslAlg(algId);
    if (alg == NULL) {
        WSEC_LOG_E("Wrong hmac algId.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

#ifdef WSEC_USE_OPENSSL_110
    hmacCtx = (HMAC_CTX *)HMAC_CTX_new();
    if (hmacCtx == NULL) {
        WSEC_LOG_E("Hmac ctx creation failed.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
#else
    hmacCtx = (HMAC_CTX *)WSEC_MALLOC(sizeof(HMAC_CTX));
    if (hmacCtx == NULL) {
        WSEC_LOG_E("Hmac ctx creation failed.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    HMAC_CTX_init(hmacCtx);
#endif

    if (HMAC_Init_ex(hmacCtx, key, (int)keyLen, alg, NULL) != 1) {
        WSEC_LOG_E("HMAC_Init_ex failed.");
#ifdef WSEC_USE_OPENSSL_110
        HMAC_CTX_free(hmacCtx);
#else
        WSEC_FREE(hmacCtx);
#endif
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = (WsecVoid *)hmacCtx;
    return WSEC_SUCCESS;
}

/* The input data is HMAC. The data can be input for multiple times. */
unsigned long CacHmacUpdate(WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen)
{
    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (HMAC_Update((HMAC_CTX *)ctx, (const unsigned char *)data, (size_t)dataLen) != 1) {
        WSEC_LOG_E("HMAC_Update failed.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    return WSEC_SUCCESS;
}

/* End the HMAC operation and obtain the HAMC result. */
unsigned long CacHmacFinal(WsecHandle *ctx, WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    WsecUint32 ret = WSEC_SUCCESS;

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

    if ((*ctx) == NULL) {
        WSEC_LOG_E("CacHmacFinal ctx is NULL.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (HMAC_Final((HMAC_CTX *)(*ctx), (unsigned char *)hmacBuff, (WsecUint32 *)hmacLen) != 1) {
        WSEC_LOG_E("HMAC_Final failed.");
        ret = WSEC_ERR_CRPTO_LIB_FAIL;
    }

    OpensslHmacFree(ctx);

    return ret;
}

/* Releases the HMAC handle. */
WsecVoid CacHmacReleaseCtx(WsecHandle *ctx)
{
    unsigned char hmacData[SDP_HMAC_MAX_SIZE] = {0};
    WsecUint32 hmacLen = sizeof(hmacData);

    WSEC_ASSERT(ctx != NULL);

    if ((*ctx) == NULL) {
        return;
    }

    WSEC_UNREFER(HMAC_Final((HMAC_CTX *)(*ctx), hmacData, &hmacLen));
    OpensslHmacFree(ctx);
}

/* Specify the algorithm ID, password, salt value, and iteration times to obtain the derived key. */
unsigned long CacPbkdf2(WsecUint32 kdfAlg,
    const WsecVoid *kdfPassword, WsecUint32 passwordLen,
    const Pbkdf2ParamConst *pbkdf2Param,
    WsecUint32 deriveKeyLen, WsecVoid *derivedKey)
{
    const EVP_MD *ctx = NULL;

    if (kdfPassword == NULL || pbkdf2Param == NULL || pbkdf2Param->salt == NULL || pbkdf2Param->saltLen == 0 ||
        derivedKey == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (pbkdf2Param->iter <= 0) {
        return WSEC_ERR_INVALID_ARG;
    }
    if ((passwordLen > 0) && (deriveKeyLen < 1)) {
        return WSEC_ERR_INVALID_ARG;
    }
    if ((passwordLen < 1) && (deriveKeyLen > 0)) {
        return WSEC_ERR_INVALID_ARG;
    }

    ctx = OpensslPbkdf2AlgToOpensslAlg(kdfAlg);
    if (ctx == NULL) {
        WSEC_LOG_E("Wrong PBKDF algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (PKCS5_PBKDF2_HMAC((const char *)kdfPassword, (int)passwordLen,
        pbkdf2Param->salt, (int)pbkdf2Param->saltLen,
        pbkdf2Param->iter, ctx, (int)deriveKeyLen, (unsigned char *)derivedKey) == 1) {
        return WSEC_SUCCESS;
    } else {
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* Obtain the block length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmBlockSize(WsecUint32 algId)
{
    if (OpensslSymmAlgToOpensslAlg(algId) != 0) {
        return (WsecUint32)EVP_CIPHER_block_size(OpensslSymmAlgToOpensslAlg(algId));
    } else {
        return 0;
    }
}

/* Obtains the IV length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmIvLen(WsecUint32 algId)
{
    if (OpensslSymmAlgToOpensslAlg(algId) != 0) {
        return (WsecUint32)EVP_CIPHER_iv_length(OpensslSymmAlgToOpensslAlg(algId));
    } else {
        return 0;
    }
}

/* Obtain the key length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmKeyLen(WsecUint32 algId)
{
    if (OpensslSymmAlgToOpensslAlg(algId) != 0) {
        return (WsecUint32)EVP_CIPHER_key_length(OpensslSymmAlgToOpensslAlg(algId));
    } else {
        return 0;
    }
}

/* Copy to the destination buffer and set the length to the size of the source data. */
static unsigned long CopyDest(const unsigned char *src, WsecUint32 srcLen, unsigned char *dest, WsecUint32 *destLen)
{
    if (*destLen < srcLen) {
        WSEC_LOG_E2("%u bytes cipher-buff needed, but only %u provided.", srcLen, *destLen);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    if (memcpy_s(dest, (size_t)*destLen, src, (size_t)srcLen) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *destLen = (WsecUint32)srcLen;
    return WSEC_SUCCESS;
}

/* Specify the algorithm ID, key, IV, and plain text, and encrypt them to obtain the cipher text. */
unsigned long CacEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    WsecBuff ciphertextBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 tempLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(iv != NULL);
    WSEC_ASSERT(plaintext != NULL);
    WSEC_ASSERT(ciphertext != NULL);
    WSEC_ASSERT(ciphertextLen != NULL);

    if (*ciphertextLen < plaintextLen) {
        WSEC_LOG_E("Cipher buff len too small");
        return WSEC_ERR_INVALID_ARG;
    }

    tempLen = plaintextLen + CAC_CRYRT_BUFF_PROTECT_SIZE;

    /* Based on the OpenSSL implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(ciphertextBuff, tempLen);
    if (ciphertextBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(ciphertextBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        ret = OpensslEncrypt(algId, key, keyLen, iv, ivLen, plaintext, plaintextLen,
            ciphertextBuff.buff, &ciphertextBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Above function return %lu", ret);
            break;
        }

        ret = CopyDest((const unsigned char *)ciphertextBuff.buff, ciphertextBuff.len, (unsigned char *)ciphertext, ciphertextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncrypt copy failed");
            break;
        }
    } while (0);

    WSEC_BUFF_FREE(ciphertextBuff);
    return ret;
}

/* Specify the algorithm ID, key, IV, and ciphertext, and decrypt them to obtain the plaintext. */
unsigned long CacDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    WsecBuff plaintextBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 tempLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(iv != NULL);
    WSEC_ASSERT(ciphertext != NULL);
    WSEC_ASSERT(plaintext != NULL);
    WSEC_ASSERT(plaintextLen != NULL);

    tempLen = ciphertextLen + CAC_CRYRT_BUFF_PROTECT_SIZE;

    /* The plaintext buffer needs to be enlarged based on OpenSSL implementation characteristics. */
    WSEC_BUFF_ALLOC(plaintextBuff, tempLen);
    if (plaintextBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(plaintextBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        ret = OpensslDecrypt(algId, key, keyLen, iv, ivLen, ciphertext, ciphertextLen,
            plaintextBuff.buff, &plaintextBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("OpensslDecrypt failed %lu", ret);
            break;
        }

        ret = CopyDest((const unsigned char *)plaintextBuff.buff, plaintextBuff.len, (unsigned char *)plaintext, plaintextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecrypt copy fail");
            break;
        }
    } while (0);

    WSEC_CLEAR_FREE(plaintextBuff.buff, plaintextBuff.len);
    return ret;
}

/* OPENSSL performs encryption. */
static unsigned long OpensslEncryptUpdate(const CacCtx *cacCtx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    int ret;
    EVP_CIPHER_CTX *evpCtx = (EVP_CIPHER_CTX *)cacCtx->ctx;
    if (evpCtx == NULL) {
        WSEC_LOG_E("Param check fail in OpensslEncryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (EVP_EncryptUpdate(evpCtx, ciphertext, (int *)cipherLen, plaintext, (int)plainLen) != 1) {
            WSEC_LOG_E("Failed when the encryption ctx Update on GCM");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        return WSEC_SUCCESS;
    }
    if (*cipherLen < plainLen + (WsecUint32)EVP_CIPHER_CTX_block_size(evpCtx)) {
        WSEC_LOG_E("Ciphertext buffer len is not enough, make sure it is at least plainLen + blocksize");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    ret = EVP_EncryptUpdate(evpCtx, ciphertext, (int *)cipherLen, plaintext, (int)plainLen);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_EncryptUpdate failed %d.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* OpenSSL stream data ends. */
static unsigned long OpensslEncryptFinal(WsecHandle ctx, unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    int ret;
    if (ctx == NULL || ciphertext == NULL || cipherLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    ret = EVP_EncryptFinal_ex((EVP_CIPHER_CTX *)ctx, ciphertext, (int *)cipherLen);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_EncryptFinal_ex failed %d.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* openssl performs decryption. */
static unsigned long OpensslDecryptUpdate(const CacCtx *cacCtx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen)
{
    int ret;
    EVP_CIPHER_CTX *evpCtx = (EVP_CIPHER_CTX *)cacCtx->ctx;
    if (evpCtx == NULL) {
        WSEC_LOG_E("Param check fail in OpensslDecryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (EVP_DecryptUpdate(evpCtx, plaintext, (int *)plainLen, ciphertext, (int)cipherLen) != 1) {
            WSEC_LOG_E("Failed when the Decryption ctx Update on GCM");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        return WSEC_SUCCESS;
    }
    if (*plainLen < cipherLen + (WsecUint32)EVP_CIPHER_CTX_block_size(evpCtx)) {
        WSEC_LOG_E("Plaintext buffer len is not enough, make sure it is at least cipherLen + blocksize");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    ret = EVP_DecryptUpdate(evpCtx, plaintext, (int *)plainLen, ciphertext, (int)cipherLen);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_DecryptUpdate failed %d.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* OpenSSL stream data ends. */
static unsigned long OpensslDecryptFinal(WsecHandle ctx, unsigned char *plaintext, WsecUint32 *plainLen)
{
    int ret;
    if (ctx == NULL || plaintext == NULL || plainLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    ret = EVP_DecryptFinal_ex((EVP_CIPHER_CTX *)(ctx), plaintext, (int *)plainLen);
    if (ret == 1) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("EVP_DecryptFinal_ex failed %d.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* Checking the Init Parameter for Stream Encryption and Decryption and Obtaining the Algorithm Type */
static unsigned long CheckEncDecParamAndGetAlgType(const WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, const unsigned char *iv, const EVP_CIPHER **algType)
{
    WSEC_ASSERT(algType != NULL);
    *algType = OpensslSymmAlgToOpensslAlg(algID);
    if (*algType == NULL) {
        WSEC_LOG_E1("Unknown AlgID %u.", algID);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (ctx == NULL || key == NULL || iv == NULL) {
        WSEC_LOG_E("Param check fail in openssl.");
        return WSEC_ERR_INVALID_ARG;
    }

    return WSEC_SUCCESS;
}

static unsigned long CacEnryptAesGcmInit(CacCtx *cacCtx,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    EVP_CIPHER_CTX *evpCtx = NULL;

    WSEC_UNREFER(keyLen);
    WSEC_ASSERT(cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM);
    evpCtx = EVP_CIPHER_CTX_new();
    if (evpCtx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed on GCM");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(evpCtx, OpensslSymmAlgToOpensslAlg(cacCtx->algID), NULL, NULL, NULL) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx init on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_CIPHER_CTX_ctrl(evpCtx, EVP_CTRL_GCM_SET_IVLEN, (int)ivLen, NULL) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx ctrl on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(evpCtx, NULL, NULL, (const unsigned char *)key, (const unsigned char *)iv) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx init on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = (WsecHandle)evpCtx;
    return WSEC_SUCCESS;
}

/* Symmetric stream encryption (start) */
unsigned long CacEncryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    const EVP_CIPHER *algType = NULL;
    EVP_CIPHER_CTX *evpCtx = NULL;
    unsigned long ret;
    CacCtx *cacCtx = NULL;
    ret = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv, &algType);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong encryption check");
        return ret;
    }
    if (keyLen != CacSymmKeyLen(algID) || ivLen != CacSymmIvLen(algID)) {
        WSEC_LOG_E("Wrong encryption IV len or Keylen.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    cacCtx = (CacCtx *)WSEC_MALLOC(sizeof(CacCtx));
    if (cacCtx == NULL) {
        WSEC_LOG_E1("%s allocate CacCtx memory failed.", __FUNCTION__);
        return WSEC_ERR_MALLOC_FAIL;
    }
    cacCtx->algID = algID;

    /* AES GCM encrypt */
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        ret = CacEnryptAesGcmInit(cacCtx, key, keyLen, iv, ivLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
            return ret;
        }
        *ctx = (WsecHandle)cacCtx;
        return ret;
    }

    evpCtx = EVP_CIPHER_CTX_new();
    if (evpCtx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed.");
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_EncryptInit_ex(evpCtx, algType, NULL, key, iv) != 1) {
        WSEC_LOG_E("EVP_EncryptInit_ex failed.");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = evpCtx;
    *ctx = (WsecHandle)cacCtx;
    return WSEC_SUCCESS;
}

/* Symmetric stream encryption (add) */
unsigned long CacEncryptUpdate(WsecHandle ctx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    WsecBuff cipherBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 protectLen = plainLen + CAC_CRYRT_BUFF_PROTECT_SIZE;
    CacCtx *cacCtx = NULL;

    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        WSEC_LOG_E("Param check fail in CacEncryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (*cipherLen < plainLen) {
        WSEC_LOG_E("Cipher buff len too small");
        return WSEC_ERR_INVALID_ARG;
    }

    cacCtx = (CacCtx *)ctx;

    /* Based on the OpenSSL implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(cipherBuff, protectLen);
    if (cipherBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(cipherBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = OpensslEncryptUpdate(cacCtx, plaintext, plainLen, (unsigned char *)cipherBuff.buff, &cipherBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("OpensslEncryptUpdate failed %lu", ret);
            break;
        }

        ret = CopyDest((const unsigned char *)cipherBuff.buff, cipherBuff.len, ciphertext, cipherLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncryptUpdate copy fail");
            break;
        }
    } while (0);
    WSEC_BUFF_FREE(cipherBuff);
    return ret;
}
static unsigned long CacEncryptAesGcmFinal(const CacCtx *cacCtx,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    WSEC_ASSERT(cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM);
    EVP_CIPHER_CTX *evpCtx = NULL;
    unsigned char *tag = ciphertext;
    WsecUint32 tagLen = WSEC_AES_GCM_TAGLEN;
    WSEC_ASSERT(*cipherLen >= tagLen);

    evpCtx = (EVP_CIPHER_CTX *)cacCtx->ctx;
    if (EVP_EncryptFinal_ex(evpCtx, ciphertext, (int *)cipherLen) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx Final on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (EVP_CIPHER_CTX_ctrl(evpCtx, EVP_CTRL_GCM_GET_TAG, (int)tagLen, tag) != 1) {
        WSEC_LOG_E("Failed when the encryption ctx ctrl on GCM to get tag");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *cipherLen = tagLen;
    return WSEC_SUCCESS;
}

/* Symmetric stream encryption (end) */
unsigned long CacEncryptFinal(WsecHandle *ctx, unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    WsecBuff cipherBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 protectLen;
    CacCtx *cacCtx = NULL;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_INVALID_ARG;
    }
    cacCtx = (CacCtx *)*ctx;
    /* AES GCM encrypt */
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (*cipherLen < WSEC_AES_GCM_TAGLEN) {
            WSEC_LOG_E("The Input ciphertext buffer len is not enough for AES_GCM.");
            return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
        }
        ret = CacEncryptAesGcmFinal(cacCtx, ciphertext, cipherLen);
        if (ret != WSEC_SUCCESS || *cipherLen != WSEC_AES_GCM_TAGLEN) {
            CacCipherFree(ctx); /* Release the memory. */
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        CacCipherFree(ctx); /* Release the memory. */
        return ret;
    }
    protectLen = (*cipherLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    /* Based on the OpenSSL implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(cipherBuff, protectLen);
    if (cipherBuff.buff == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = OpensslEncryptFinal(cacCtx->ctx, (unsigned char *)cipherBuff.buff, &cipherBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("OpensslEncryptFinal failed %lu", ret);
            break;
        }

        ret = CopyDest((const unsigned char *)cipherBuff.buff, cipherBuff.len, ciphertext, cipherLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncryptFinal copy fail");
            break;
        }
    } while (0);
    CacCipherFree(ctx); /* Release the memory. */
    WSEC_BUFF_FREE(cipherBuff);
    return ret;
}

static unsigned long CacDeryptAesGcmInit(CacCtx *cacCtx,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    WSEC_UNREFER(keyLen);
    WSEC_ASSERT(cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM);
    EVP_CIPHER_CTX *evpCtx = NULL;
    evpCtx = EVP_CIPHER_CTX_new();
    if (evpCtx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed on GCM");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(evpCtx, OpensslSymmAlgToOpensslAlg(cacCtx->algID), NULL, NULL, NULL) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctevpCtxx init on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_CIPHER_CTX_ctrl(evpCtx, EVP_CTRL_GCM_SET_IVLEN, (int)ivLen, NULL) != 1) {
        WSEC_LOG_E("Failed when the Decryption evpCtx ctrl on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(evpCtx, NULL, NULL, key, iv) != 1) {
        WSEC_LOG_E("Failed when the Decryption evpCtx init on GCM");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = (WsecHandle)evpCtx;
    return WSEC_SUCCESS;
}

/* Symmetric decryption of stream data (start) */
unsigned long CacDecryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    const EVP_CIPHER *algType = NULL;
    EVP_CIPHER_CTX *evpCtx = NULL;
    unsigned long ret;
    CacCtx *cacCtx = NULL;
    ret = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv, &algType);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong decryption check.");
        return ret;
    }
    if (keyLen != CacSymmKeyLen(algID) || ivLen != CacSymmIvLen(algID)) {
        WSEC_LOG_E("Wrong decryption IV len or Keylen.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    cacCtx = (CacCtx *)WSEC_MALLOC(sizeof(CacCtx));
    if (cacCtx == NULL) {
        WSEC_LOG_E1("%s allocate CacCtx memory failed.", __FUNCTION__);
        return WSEC_ERR_MALLOC_FAIL;
    }
    cacCtx->algID = algID;

    /* AES GCM Derypt */
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        ret = CacDeryptAesGcmInit(cacCtx, key, keyLen, iv, ivLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
            return ret;
        }
        *ctx = (WsecHandle)cacCtx;
        return ret;
    }

    evpCtx = EVP_CIPHER_CTX_new();
    if (evpCtx == NULL) {
        WSEC_LOG_E("CIPHER CTX creation failed.");
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (EVP_DecryptInit_ex(evpCtx, algType, NULL, key, iv) != 1) {
        WSEC_LOG_E("EVP_DecryptInit_ex failed.");
        EVP_CIPHER_CTX_free(evpCtx);
        evpCtx = NULL;
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = evpCtx;
    *ctx = (WsecHandle)cacCtx;
    return WSEC_SUCCESS;
}

/* Symmetric decryption of stream data (add) */
unsigned long CacDecryptUpdate(WsecHandle ctx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecBuff plainBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 protectLen = (cipherLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        WSEC_LOG_E("Param check fail in CacDecryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (*plainLen < cipherLen) {
        WSEC_LOG_E("Plain buff len too small");
        return WSEC_ERR_INVALID_ARG;
    }

    cacCtx = (CacCtx *)ctx;
    /* The plaintext buffer needs to be enlarged based on OpenSSL implementation characteristics. */
    WSEC_BUFF_ALLOC(plainBuff, protectLen);
    if (plainBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(plainBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = OpensslDecryptUpdate(cacCtx, ciphertext, cipherLen, (unsigned char *)plainBuff.buff, &plainBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("OpensslDecryptUpdate failed %lu", ret);
            break;
        }

        ret = CopyDest((const unsigned char *)plainBuff.buff, plainBuff.len, plaintext, plainLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecryptUpdate copy failed");
            break;
        }
    } while (0);

    WSEC_CLEAR_FREE(plainBuff.buff, plainBuff.len);
    return ret;
}

static unsigned long CacDecryptAesGcmFinal(const CacCtx *cacCtx, unsigned char *plaintext, WsecUint32 *plainLen)
{
    WSEC_ASSERT(cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM);
    EVP_CIPHER_CTX *evpCtx = NULL;
    unsigned char *tag = plaintext;
    int len;
    WSEC_ASSERT(*plainLen >= WSEC_AES_GCM_TAGLEN);

    evpCtx = (EVP_CIPHER_CTX *)cacCtx->ctx;
    if (EVP_CIPHER_CTX_ctrl(evpCtx, EVP_CTRL_GCM_SET_TAG, WSEC_AES_GCM_TAGLEN, tag) != 1) {
        WSEC_LOG_E("Failed when the Decryption ctx ctrl on GCM");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (EVP_DecryptFinal_ex(evpCtx, (unsigned char *)tag, &len) != 1) {
        WSEC_LOG_E("Failed when the Decryption Final");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plainLen = (WsecUint32)len;
    return WSEC_SUCCESS;
}

/* Symmetric decryption of stream data (end) */
unsigned long CacDecryptFinal(WsecHandle *ctx, unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecBuff plainBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 protectLen;
    CacCtx *cacCtx = NULL;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL || plaintext == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_INVALID_ARG;
    }
    cacCtx = (CacCtx *)*ctx;
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (*plainLen < WSEC_AES_GCM_TAGLEN) {
            WSEC_LOG_E("The Input plaintext buffer len is not enough for AES_GCM.");
            return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
        }
        ret = CacDecryptAesGcmFinal(cacCtx, plaintext, plainLen);
        if (ret != WSEC_SUCCESS) {
            CacCipherFree(ctx);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        CacCipherFree(ctx);
        return ret;
    }
    /* The plaintext buffer needs to be enlarged based on OpenSSL implementation characteristics. */
    protectLen = (*plainLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    WSEC_BUFF_ALLOC(plainBuff, protectLen);
    if (plainBuff.buff == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = OpensslDecryptFinal(cacCtx->ctx, (unsigned char *)plainBuff.buff, &plainBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("OpensslDecryptFinal failed %lu", ret);
            break;
        }
        ret = CopyDest((const unsigned char *)plainBuff.buff, plainBuff.len, plaintext, plainLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecryptFinal copy failed");
            break;
        }
    } while (0);
    CacCipherFree(ctx); /* Release the memory. */
    WSEC_CLEAR_FREE(plainBuff.buff, protectLen);
    return ret;
}

#endif /* WSEC_COMPILE_CAC_OPENSSL */
