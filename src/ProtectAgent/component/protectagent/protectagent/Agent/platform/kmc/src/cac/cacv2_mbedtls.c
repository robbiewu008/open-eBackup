/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: basic encryption function, which adapts to the mbedtls interface.
 * Author: yulong y00354181
 * Create: 2020-03-23
 */

#ifdef WSEC_COMPILE_CAC_MBEDTLS
#include "wsecv2_type.h"
#include "cacv2_pri.h"
#include "securec.h"
#include "mbedtls/platform.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "wsecv2_mem.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_lock.h"
#include "wsecv2_util.h"

#define CAC_ALGID_UNKNOWN 0
#define CAC_AES_BLOCK 16
#define CAC_AES_CBC_IV_LEN 16
#define CAC_AES_128_CBC_KEY_LEN 16
#define CAC_AES_256_CBC_KEY_LEN 32

#define CAC_AES_GCM_IV_LEN 12
#define CAC_AES_128_GCM_KEY_LEN 16
#define CAC_AES_256_GCM_KEY_LEN 32

#define CAC_DIGEST_SHA256_LEN 32
#define CAC_DIGEST_SHA384_LEN 48
#define CAC_DIGEST_SHA512_LEN 64

#define CAC_MBEDLTS_BITS 8

typedef struct {
    WsecHandle ctx;
    WsecUint32 algID;
} CacCtx;


static mbedtls_ctr_drbg_context g_mbedRngCtx;

#define CAC_ALIGN_AES_BLOCK(_size) (((_size) + CAC_AES_BLOCK) & (~(WsecUint32)(CAC_AES_BLOCK - 1)))

static WsecVoid ThreadLockRand(void)
{
    WsecThreadLockById(LOCK4KMC_RAND);
}

static WsecVoid ThreadUnlockRand(void)
{
    WsecThreadUnlockById(LOCK4KMC_RAND);
}

/* Cac free Cipher for stream api */
void CacCipherFree(WsecHandle *ctx)
{
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }
    cacCtx = (CacCtx *)(*ctx);
    if (cacCtx->ctx != NULL) {
        if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
            mbedtls_gcm_free(cacCtx->ctx);
        } else {
            mbedtls_cipher_free(cacCtx->ctx);
        }
        WSEC_FREE(cacCtx->ctx);
    }
    WSEC_FREE(cacCtx);
    *ctx = NULL;
}

/* free digest alg ctx */
static WsecVoid MbedtlsDigestFree(WsecHandle *ctx)
{
    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }

    mbedtls_md_free((mbedtls_md_context_t *)(*ctx));
    WSEC_FREE(*ctx);
}

/* Get Cipher info from alg id  */
static const mbedtls_cipher_info_t *MbedtlsGetCipherInfo(WsecUint32 algId)
{
    switch (algId) {
        case WSEC_ALGID_AES128_CBC:
            return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC);
            break;
        case WSEC_ALGID_AES256_CBC:
            return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC);
            break;
        default:
            return NULL;
    }
}

/* Convert to Mbedtls digest alg info */
static const mbedtls_md_info_t *MbedtlsDigestAlgToMbedtlsAlg(WsecUint32 algId)
{
    switch (algId) {
        case WSEC_ALGID_SHA256:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        case WSEC_ALGID_SHA384:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA384);
        case WSEC_ALGID_SHA512:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
        default:
            return NULL;
    }
}

/* Convert to Mbedtls hmac alg info */
static const mbedtls_md_info_t *MbedtlsHmacAlgToMbedtlsAlg(WsecUint32 algId)
{
    switch (algId) {
        case WSEC_ALGID_HMAC_SHA256:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        case WSEC_ALGID_HMAC_SHA384:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA384);
        case WSEC_ALGID_HMAC_SHA512:
            return mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
        default:
            return NULL;
    }
}

/* Convert to Mbedtls systematic enc alg id */
static WsecAlgtype MbedtlsSymmAlgToMbedtlsAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_AES128_CBC):
            return WSEC_ALGTYPE_SYM;
        case (WSEC_ALGID_AES256_CBC):
            return WSEC_ALGTYPE_SYM;
        case (WSEC_ALGID_AES128_GCM):
            return WSEC_ALGTYPE_SYM;
        case (WSEC_ALGID_AES256_GCM):
            return WSEC_ALGTYPE_SYM;
        default:
            return WSEC_ALGTYPE_UNKNOWN;
    }
}

/* Convert to Mbedtls hmac alg id */
static const mbedtls_md_info_t *MbedltsPbkdf2AlgToMbedtlsMd(WsecUint32 algId)
{
    WsecUint32 mdType;

    switch (algId) {
        case WSEC_ALGID_PBKDF2_HMAC_SHA256:
            mdType = MBEDTLS_MD_SHA256;
            break;
        case WSEC_ALGID_PBKDF2_HMAC_SHA384:
            mdType = MBEDTLS_MD_SHA384;
            break;
        case WSEC_ALGID_PBKDF2_HMAC_SHA512:
            mdType = MBEDTLS_MD_SHA512;
            break;
        default:
            return NULL;
    }

    return mbedtls_md_info_from_type(mdType);
}

/* Mbedtls aes gcm implementation */
unsigned long CacEncryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *plaintext, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen)
{
    mbedtls_gcm_context gcmCtx;
    int ret;

    WSEC_UNREFER(algType);
    WSEC_ASSERT(algType == WSEC_ALGID_AES128_GCM || algType == WSEC_ALGID_AES256_GCM);

    mbedtls_gcm_init(&gcmCtx);
    ret = mbedtls_gcm_setkey(&gcmCtx, MBEDTLS_CIPHER_ID_AES, key, (keyLen * CAC_MBEDLTS_BITS));
    if (ret == MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED) {
        WSEC_LOG_E1("gcm set key failed (-0x%04x).", ret);
        mbedtls_gcm_free(&gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = mbedtls_gcm_crypt_and_tag(&gcmCtx, MBEDTLS_GCM_ENCRYPT, plaintextLen, iv, ivLen, key, keyLen,
                                    plaintext, ciphertext, tagLen, tag);
    if (ret != 0) {
        WSEC_LOG_E1("gcm encrypt failed -0x%04x).", ret);
        mbedtls_gcm_free(&gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ciphertextLen = plaintextLen + WSEC_AES_GCM_TAGLEN;
    mbedtls_gcm_free(&gcmCtx);
    return WSEC_SUCCESS;
}

/* Mbedtls aes gcm decrypt implementation */
unsigned long CacDecryptAesGcm(WsecUint32 algType,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen,
    unsigned char *plaintext, WsecUint32 *plaintextLen)
{
    mbedtls_gcm_context gcmCtx;
    int ret;

    WSEC_UNREFER(algType);
    WSEC_ASSERT(algType == WSEC_ALGID_AES128_GCM || algType == WSEC_ALGID_AES256_GCM);

    if (tagLen < WSEC_AES_GCM_TAGLEN) {
        WSEC_LOG_E("Wrong Encryption input tag Length for AES_GCM oper");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    mbedtls_gcm_init(&gcmCtx);
    ret = mbedtls_gcm_setkey(&gcmCtx, MBEDTLS_CIPHER_ID_AES, key, (keyLen * CAC_MBEDLTS_BITS));
    if (ret != 0) {
        WSEC_LOG_E1("set key platform not support 0x%04x.", ret);
        mbedtls_gcm_free(&gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = mbedtls_gcm_auth_decrypt(&gcmCtx, ciphertextLen, iv, ivLen, key, keyLen, tag, tagLen,
                                   ciphertext, plaintext);
    if (ret != 0) {
        WSEC_LOG_E1("gcm decrypt failed 0x%04x.", ret);
        mbedtls_gcm_free(&gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *plaintextLen = ciphertextLen;
    mbedtls_gcm_free(&gcmCtx);
    return WSEC_SUCCESS;
}

/* Mbedtls aes cbc implementation */
static unsigned long DoCbcEncrypt(const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    mbedtls_aes_context aesCtx = { 0 };
    unsigned char tempIv[CAC_AES_CBC_IV_LEN] = { 0 };
    unsigned char *padPlaintext = NULL;
    WsecUint32 paddingLen;
    int paddingVal;
    size_t len;
    int ret;

    mbedtls_aes_init(&aesCtx);
    ret = mbedtls_aes_setkey_enc(&aesCtx, key, (keyLen * CAC_MBEDLTS_BITS));
    if (ret != 0) {
        WSEC_LOG_E1("aes set key failed (-0x%04x).", ret);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    len = CAC_ALIGN_AES_BLOCK(plaintextLen);
    paddingLen = CAC_AES_BLOCK - (plaintextLen % CAC_AES_BLOCK);
    paddingVal = (int)paddingLen;
    padPlaintext = (unsigned char *)WSEC_MALLOC(len);
    if (padPlaintext == NULL) {
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MALLOC_FAIL;
    }

    if (memcpy_s(padPlaintext, len, plaintext, plaintextLen) != EOK) {
        WSEC_FREE(padPlaintext);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MEMCPY_FAIL;
    }

    if (memset_s(padPlaintext + plaintextLen, len - plaintextLen, paddingVal, paddingLen) != EOK) {
        WSEC_CLEAR_FREE(padPlaintext, len);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MEMSET_FAIL;
    }

    if (memcpy_s(tempIv, CAC_AES_CBC_IV_LEN, iv, ivLen) != EOK) {
        WSEC_CLEAR_FREE(padPlaintext, len);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MEMCPY_FAIL;
    }

    ret = mbedtls_aes_crypt_cbc(&aesCtx, MBEDTLS_AES_ENCRYPT, len, tempIv, padPlaintext, ciphertext);
    if (ret != 0) {
        WSEC_LOG_E1("aes enc failed (-0x%04x).", ret);
        WSEC_CLEAR_FREE(padPlaintext, len);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ciphertextLen = (WsecUint32)len;
    /* clear ctx */
    mbedtls_aes_free(&aesCtx);
    WSEC_CLEAR_FREE(padPlaintext, len);

    return WSEC_SUCCESS;
}

/* mbedtls encrypt function for aes cbc and gcm */
static unsigned long MbedtlsEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    WsecAlgtype algType;

    algType = MbedtlsSymmAlgToMbedtlsAlg(algId);
    if (algType == WSEC_ALGTYPE_UNKNOWN) {
        WSEC_LOG_E("DDLDDWrong Encryption algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (key == NULL || iv == NULL || plaintext == NULL || ciphertext == NULL || ciphertextLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (keyLen != CacSymmKeyLen(algId)) {
        WSEC_LOG_E("Wrong Encryption key Keylen.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (keyLen != CacSymmKeyLen(algId)) {
            WSEC_LOG_E("Wrong Encryption key Keylen.");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        if (*ciphertextLen < plaintextLen + WSEC_AES_GCM_TAGLEN) {
            return WSEC_ERR_INVALID_ARG;
        }
        return CacEncryptAesGcm(algId, (const unsigned char *)key, keyLen,
            (const unsigned char *)iv, ivLen, (const unsigned char *)plaintext, plaintextLen,
            (unsigned char *)ciphertext, ciphertextLen,
            (unsigned char *)ciphertext + plaintextLen, WSEC_AES_GCM_TAGLEN);
    }
    if (*ciphertextLen < ((plaintextLen / CAC_AES_BLOCK + 1) * CAC_AES_BLOCK)) {
        WSEC_LOG_E("The Input ciphertext buffer len is not enough on CBC");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    return DoCbcEncrypt(key, keyLen, iv, ivLen, plaintext, plaintextLen, ciphertext, ciphertextLen);
}


/* Run the CBC command to decrypt mbedtls. */
static unsigned long DoCbcDecrypt(const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    mbedtls_aes_context aesCtx = { 0 };
    unsigned char tempIv[CAC_AES_CBC_IV_LEN] = { 0 };
    int ret;

    mbedtls_aes_init(&aesCtx);
    ret = mbedtls_aes_setkey_dec(&aesCtx, key, (keyLen * CAC_MBEDLTS_BITS));
    if (ret != 0) {
        WSEC_LOG_E1("aes set key failed (-0x%04x).", ret);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    unsigned char *tempPlain = (unsigned char *)WSEC_MALLOC(ciphertextLen);
    if (tempPlain == NULL) {
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MALLOC_FAIL;
    }
    if (memcpy_s(tempIv, CAC_AES_CBC_IV_LEN, iv, ivLen) != EOK) {
        WSEC_FREE(tempPlain);
        mbedtls_aes_free(&aesCtx);
        return WSEC_ERR_MEMCPY_FAIL;
    }
    ret = mbedtls_aes_crypt_cbc(&aesCtx, MBEDTLS_AES_DECRYPT, ciphertextLen, tempIv,
                                (const unsigned char *)ciphertext, tempPlain);
    if (ret != 0) {
        WSEC_CLEAR_FREE(tempPlain, ciphertextLen);
        mbedtls_aes_free(&aesCtx);
        WSEC_LOG_E1("mbedtls_aes_crypt_cbc failed (-0x%04x).", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    /* rmv pad add cal len */
    WsecUint32 paddingLen = tempPlain[ciphertextLen - 1];
    memcpy_s(plaintext, *plaintextLen, tempPlain, ciphertextLen - paddingLen);
    *plaintextLen = ciphertextLen - paddingLen;
    WSEC_CLEAR_FREE(tempPlain, ciphertextLen);
    mbedtls_aes_free(&aesCtx);
    return WSEC_SUCCESS;
}

/* mbedtls decrypt implementation */
static unsigned long MbedtlsDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    const unsigned char *tagInCipher = NULL;
    unsigned char tag[WSEC_AES_GCM_TAGLEN] = { 0 };
    WsecAlgtype algType;

    algType = MbedtlsSymmAlgToMbedtlsAlg(algId);
    if (algType == WSEC_ALGTYPE_UNKNOWN) {
        WSEC_LOG_E("Wrong Decryption algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (key == NULL || iv == NULL || ciphertext == NULL || plaintext == NULL || plaintextLen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (keyLen != CacSymmKeyLen(algId)) {
        WSEC_LOG_E1("Wrong Encryption  Keylen %d", keyLen);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (keyLen != CacSymmKeyLen(algId)) {
            WSEC_LOG_E("Wrong Encryption  Keylen");
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
    return DoCbcDecrypt(key, keyLen, iv, ivLen, ciphertext, ciphertextLen, plaintext, plaintextLen);
}

static int MbedtlsMyEntropyFunc(void *data, unsigned char *output, size_t len)
{
    WSEC_UNREFER(data);
    unsigned char *entropy = NULL;
    if (WsecGetEntropy(&entropy, len) == WSEC_FALSE) {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
    if (memcpy_s(output, len, entropy, len) != EOK) {
        WSEC_LOG_E4MEMCPY;
        WsecCleanupEntropy(entropy, len);
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
    WsecCleanupEntropy(entropy, len);
    return 0;
}

/* init rng */
unsigned long CacInitRng(void)
{
    const char *pers = "CTR_DRBG";
    int ret;
    mbedtls_ctr_drbg_init(&g_mbedRngCtx);
    ret = mbedtls_ctr_drbg_seed(&g_mbedRngCtx, MbedtlsMyEntropyFunc, NULL,
                                (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        WSEC_LOG_E1("Call mbedtls_ctr_drbg_seed failed : %d", ret);
        mbedtls_ctr_drbg_free(&g_mbedRngCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

void CacUnInitRng(void)
{
    mbedtls_ctr_drbg_free(&g_mbedRngCtx);
}

/* generate the random number */
unsigned long CacRandom(WsecVoid *buff, WsecUint32 buffLen)
{
    WsecBool returnValue;
    int ret;

    WSEC_ASSERT(buff != NULL);

    ThreadLockRand();
    returnValue = WsecGetRandomNumber((unsigned char *)buff, (size_t)buffLen);
    if (returnValue == WSEC_TRUE) {
        ThreadUnlockRand();
        return WSEC_SUCCESS;
    }

    ret = mbedtls_ctr_drbg_random(&g_mbedRngCtx, buff, buffLen);
    if (ret != 0) {
        WSEC_LOG_E1("Call mbedtls_ctr_drbg_random failed : %d", ret);
        ThreadUnlockRand();
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ThreadUnlockRand();
    return WSEC_SUCCESS;
}

/* convert alg id to cac alg type */
WsecUint32 CacAlgIdToType(WsecUint32 algId)
{
    if (MbedtlsSymmAlgToMbedtlsAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_SYM;
    }

    if (MbedtlsHmacAlgToMbedtlsAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_HMAC;
    }

    if (MbedltsPbkdf2AlgToMbedtlsMd(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_PBKDF;
    }

    if (MbedtlsDigestAlgToMbedtlsAlg(algId) != CAC_ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_DIGEST;
    }

    return WSEC_ALGTYPE_UNKNOWN;
}

/* Digest implementation */
unsigned long CacDigest(WsecUint32 algId,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *digestBuff, WsecUint32 *digestLen)
{
    const mbedtls_md_info_t *mdInfo = NULL;

    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(digestLen != NULL);

    mdInfo = MbedtlsDigestAlgToMbedtlsAlg(algId);
    if (mdInfo == NULL) {
        WSEC_LOG_E("Wrong Digest algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (mbedtls_md(mdInfo, data, dataLen, digestBuff) == 0) {
        *digestLen = mbedtls_md_get_size(mdInfo);
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E("Got failure from mbedtls_md");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* init Digest via id and return digest ctx */
unsigned long CacDigestInit(WsecHandle *ctx, WsecUint32 algId)
{
    const mbedtls_md_info_t *mdInfo = NULL;
    int ret;

    WSEC_ASSERT(ctx != NULL);

    mdInfo = MbedtlsDigestAlgToMbedtlsAlg(algId);
    if (mdInfo == NULL) {
        WSEC_LOG_E("Wrong hmac mdInfo .");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = WSEC_MALLOC(sizeof(mbedtls_md_context_t));
    if (*ctx == NULL) {
        WSEC_LOG_E("Got failure from malloc.");
        return WSEC_ERR_MALLOC_FAIL;
    }

    mbedtls_md_init(*ctx);

    ret = mbedtls_md_setup(*ctx, mdInfo, 0);
    if (ret != 0) {
        WSEC_LOG_E1("md set up failed(-0x%04x).", ret);
        mbedtls_md_free(*ctx);
        WSEC_FREE(*ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = mbedtls_md_starts(*ctx);
    if (ret != 0) {
        WSEC_LOG_E1("md starts failed(-0x%04x).", ret);
        mbedtls_md_free(*ctx);
        WSEC_FREE(*ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Digest update implementation */
unsigned long CacDigestUpdate(const WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen)
{
    int returnValue;

    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    returnValue = mbedtls_md_update((mbedtls_md_context_t *)ctx, data, (size_t)dataLen);
    if (returnValue == 0) {
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("Got failure from mbedtls_md_update: %d", returnValue);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* Digest final implementation */
unsigned long CacDigestFinal(WsecHandle *ctx, WsecVoid *digestBuff, WsecUint32 *buffLen)
{
    int returnValue;

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(buffLen != NULL);

    if ((*ctx) == NULL) {
        WSEC_LOG_E("Got failure from CRYPT_DigestFinal, The content of ctx is NULL");
        return WSEC_ERR_INVALID_ARG;
    }

    returnValue = mbedtls_md_finish((mbedtls_md_context_t *)(*ctx), (unsigned char *)digestBuff);
    if (returnValue != 0) {
        MbedtlsDigestFree(ctx);
        WSEC_LOG_E1("Got failure from md finish: %d", returnValue);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *buffLen = mbedtls_md_get_size(((mbedtls_md_context_t *)*ctx)->md_info);
    MbedtlsDigestFree(ctx);

    return WSEC_SUCCESS;
}

/* free Digest ctx */
WsecVoid CacDigestReleaseCtx(WsecHandle *ctx)
{
    unsigned char digest[SDP_DIGEST_MAX_SIZE] = { 0 };
    int ret;

    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }

    ret = mbedtls_md_finish((mbedtls_md_context_t *)(*ctx), (unsigned char *)digest);
    if (ret != 0) {
        WSEC_LOG_E1("Got failure from md finish: %d", ret);
    }

    MbedtlsDigestFree(ctx);
}

/* get Hmac size via hmac alg id */
WsecUint32 CacHMACSize(WsecUint32 algId)
{
    switch (algId) {
        case WSEC_ALGID_HMAC_SHA256:
            return CAC_DIGEST_SHA256_LEN;
        case WSEC_ALGID_HMAC_SHA384:
            return CAC_DIGEST_SHA384_LEN;
        case WSEC_ALGID_HMAC_SHA512:
            return CAC_DIGEST_SHA512_LEN;
        default:
            return 0;
    }
}

/* cac hmac implementation */
unsigned long CacHmac(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    const mbedtls_md_info_t  *mdInfo = NULL;

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

    mdInfo = MbedtlsHmacAlgToMbedtlsAlg(algId);
    if (mdInfo == NULL) {
        WSEC_LOG_E("Wrong hmac algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (mbedtls_md_hmac(mdInfo, key, keyLen, data, dataLen, hmacBuff) != 0) {
        WSEC_LOG_E("Got failure from HMAC");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *hmacLen = mbedtls_md_get_size(mdInfo);
    return WSEC_SUCCESS;
}

/* cac hmac init for update and final */
unsigned long CacHmacInit(WsecHandle *ctx,
    WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen)
{
    const mbedtls_md_info_t *mdInfo = NULL;
    mbedtls_md_context_t *mdCtx = NULL;
    int ret;

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(key != NULL);

    mdInfo = MbedtlsHmacAlgToMbedtlsAlg(algId);
    if (mdInfo == NULL) {
        WSEC_LOG_E("Wrong hmac mdInfo .");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    mdCtx = WSEC_MALLOC(sizeof(mbedtls_md_context_t));
    if (mdCtx == NULL) {
        WSEC_LOG_E("Got failure from malloc.");
        return WSEC_ERR_MALLOC_FAIL;
    }

    mbedtls_md_init(mdCtx);

    ret = mbedtls_md_setup(mdCtx, mdInfo, 1);
    if (ret != 0) {
        mbedtls_md_free(mdCtx);
        WSEC_FREE(mdCtx);
        WSEC_LOG_E1("md hmac set up failed (-0x%04x).", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = mbedtls_md_hmac_starts(mdCtx, key, keyLen);
    if (ret != 0) {
        mbedtls_md_free(mdCtx);
        WSEC_FREE(mdCtx);
        WSEC_LOG_E1("md hmac starts failed(-0x%04x).", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = (WsecVoid *)mdCtx;
    return WSEC_SUCCESS;
}

/* hmac update implementation */
unsigned long CacHmacUpdate(const WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen)
{
    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (mbedtls_md_hmac_update((mbedtls_md_context_t *)ctx, data, (size_t)dataLen) != 0) {
        WSEC_LOG_E("Got failure from mbedtls_md_hmac_update.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    return WSEC_SUCCESS;
}

/* cac hmac final implementation */
unsigned long CacHmacFinal(WsecHandle *ctx, WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    WsecUint32 returnValue = WSEC_SUCCESS;

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

    if ((*ctx) == NULL) {
        WSEC_LOG_E("The Input ctx of CacHmacFinal is NULL.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (mbedtls_md_hmac_finish((mbedtls_md_context_t *)(*ctx), (unsigned char *)hmacBuff) != 0) {
        WSEC_LOG_E("Got failure from mbedtls_md_hmac_finish.");
        MbedtlsDigestFree(ctx);
        returnValue = WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *hmacLen = mbedtls_md_get_size(((mbedtls_md_context_t *)*ctx)->md_info);
    MbedtlsDigestFree(ctx);

    return returnValue;
}

/* free hmac ctx */
WsecVoid CacHmacReleaseCtx(WsecHandle *ctx)
{
    unsigned char hmacData[SDP_HMAC_MAX_SIZE] = { 0 };
    int ret;

    if (ctx == NULL || (*ctx) == NULL) {
        return;
    }

    ret = mbedtls_md_hmac_finish((mbedtls_md_context_t *)(*ctx), (unsigned char *)hmacData);
    if (ret != 0) {
        WSEC_LOG_E1("Got failure from md finish: %d", ret);
    }

    MbedtlsDigestFree(ctx);
}

/* pbkdf2 mbedtls implementation */
unsigned long CacPbkdf2(WsecUint32 kdfAlg,
    const WsecVoid *kdfPassword, WsecUint32 passwordLen,
    const Pbkdf2ParamConst *pbkdf2Param,
    WsecUint32 deriveKeyLen, WsecVoid *derivedKey)
{
    const mbedtls_md_info_t *mdInfo = NULL;
    mbedtls_md_context_t ctx;
    int ret;

    if (!(kdfPassword != NULL && pbkdf2Param->salt != NULL && derivedKey != NULL && pbkdf2Param->saltLen)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (pbkdf2Param->iter <= 0) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (((passwordLen > 0) && (deriveKeyLen < 1)) ||
        ((passwordLen < 1) && (deriveKeyLen > 0))) {
        return WSEC_ERR_INVALID_ARG;
    }

    mdInfo = MbedltsPbkdf2AlgToMbedtlsMd(kdfAlg);
    if (mdInfo == NULL) {
        WSEC_LOG_E("Wrong KDF algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    mbedtls_md_init(&ctx);
    ret = mbedtls_md_setup(&ctx, mdInfo, 1);
    if (ret != 0) {
        WSEC_LOG_E1("mbedtls_md_setup failed (-0x%04x)", ret);
        mbedtls_md_free(&ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = mbedtls_pkcs5_pbkdf2_hmac(&ctx, kdfPassword, passwordLen, pbkdf2Param->salt, pbkdf2Param->saltLen,
        (unsigned int)pbkdf2Param->iter, deriveKeyLen, derivedKey);
    if (ret != 0) {
        WSEC_LOG_E1("mbedtls_pkcs5_pbkdf2_hmac failed (-0x%04x)", ret);
        mbedtls_md_free(&ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    mbedtls_md_free(&ctx);

    return WSEC_SUCCESS;
}

/* get symmetric enc/dec iv len */
WsecUint32 CacSymmIvLen(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_AES128_CBC):
            return CAC_AES_CBC_IV_LEN;
        case (WSEC_ALGID_AES256_CBC):
            return CAC_AES_CBC_IV_LEN;
        case (WSEC_ALGID_AES128_GCM):
            return CAC_AES_GCM_IV_LEN;
        case (WSEC_ALGID_AES256_GCM):
            return CAC_AES_GCM_IV_LEN;
        default:
            return 0;
    }
}

/* get symmetric enc/dec key len */
WsecUint32 CacSymmKeyLen(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_AES128_CBC):
            return CAC_AES_128_CBC_KEY_LEN;
        case (WSEC_ALGID_AES256_CBC):
            return CAC_AES_256_CBC_KEY_LEN;
        case (WSEC_ALGID_AES128_GCM):
            return CAC_AES_128_GCM_KEY_LEN;
        case (WSEC_ALGID_AES256_GCM):
            return CAC_AES_256_GCM_KEY_LEN;
        default:
            return 0;
    }
}

/* copy utils function */
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

/* cac encrypt implementation with mbedltls */
unsigned long CacEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    WsecBuff ciphertextBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 tempLen;

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
    /* Based on the mbedtls implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(ciphertextBuff, tempLen);
    if (ciphertextBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(ciphertextBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        ret = MbedtlsEncrypt(algId, key, keyLen, iv, ivLen, plaintext, plaintextLen,
            ciphertextBuff.buff, &ciphertextBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Above function return %lu", ret);
            break;
        }

        ret = CopyDest(ciphertextBuff.buff, ciphertextBuff.len, ciphertext, ciphertextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncrypt copy fail");
            break;
        }
    } while (0);

    WSEC_BUFF_FREE(ciphertextBuff);
    return ret;
}

/* cac decrypt implementation */
unsigned long CacDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    WsecBuff plaintextBuff = { NULL, 0 };
    unsigned long ret;
    WsecUint32 tempLen;

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(iv != NULL);
    WSEC_ASSERT(ciphertext != NULL);
    WSEC_ASSERT(plaintext != NULL);
    WSEC_ASSERT(plaintextLen != NULL);

    tempLen = ciphertextLen + CAC_CRYRT_BUFF_PROTECT_SIZE;
    /* Based on the mbedtls implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(plaintextBuff, tempLen);
    if (plaintextBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(plaintextBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        ret = MbedtlsDecrypt(algId, key, keyLen, iv, ivLen, ciphertext, ciphertextLen,
            plaintextBuff.buff, &plaintextBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Above function return %lu", ret);
            break;
        }

        ret = CopyDest(plaintextBuff.buff, plaintextBuff.len, plaintext, plaintextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecrypt copy fail");
            break;
        }
    } while (0);

    WSEC_CLEAR_FREE(plaintextBuff.buff, plaintextBuff.len);
    return ret;
}

/* mbedtls encrypt update implementation */
static unsigned long MbedtlsEncryptUpdate(CacCtx *cacCtx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    int ret;

    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        ret = mbedtls_gcm_update(cacCtx->ctx, plainLen, plaintext, ciphertext);
        if (ret != 0) {
            WSEC_LOG_E1("mbedtls_gcm_update aes enc failed (-0x%04x).", ret);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *cipherLen = plainLen;
    } else {
        ret = mbedtls_cipher_update(cacCtx->ctx, plaintext, plainLen, ciphertext, (size_t *)cipherLen);
        if (ret != 0) {
            WSEC_LOG_E1("mbedtls_cipher_update aes enc failed (-0x%04x).", ret);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/* mbedtls decrypt update implementation */
static unsigned long MbedtlsDecryptUpdate(CacCtx *cacCtx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen)
{
    int ret;

    if (cacCtx == NULL || cacCtx->ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        ret = mbedtls_gcm_update(cacCtx->ctx, cipherLen, ciphertext, plaintext);
        if (ret != 0) {
            WSEC_LOG_E1("mbedtls_gcm_update aes dec failed (-0x%04x).", ret);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *plainLen = cipherLen;
    } else {
        ret = mbedtls_cipher_update(cacCtx->ctx, ciphertext, cipherLen, plaintext, (size_t *)plainLen);
        if (ret != 0) {
            WSEC_LOG_E1("mbedtls_cipher_update aes dec failed (-0x%04x).", ret);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

static unsigned long CheckEncDecParamAndGetAlgType(const WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, const unsigned char *iv)
{
    WsecAlgtype algType;

    algType = MbedtlsSymmAlgToMbedtlsAlg(algID);
    if (algType == WSEC_ALGTYPE_UNKNOWN) {
        WSEC_LOG_E("Unknown AlgID");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (ctx == NULL || key == NULL || iv == NULL) {
        WSEC_LOG_E("Param check fail in mbedtls");
        return WSEC_ERR_INVALID_ARG;
    }

    return WSEC_SUCCESS;
}

static unsigned long CacCryptGcmInit(CacCtx *cacCtx, const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen, int mode)
{
    mbedtls_gcm_context *gcmCtx = NULL;
    gcmCtx = WSEC_MALLOC(sizeof(mbedtls_gcm_context));
    if (gcmCtx == NULL) {
        WSEC_LOG_E1("%s allocate gcmCtx memory failed.", __FUNCTION__);
        return WSEC_ERR_MALLOC_FAIL;
    }
    mbedtls_gcm_init(gcmCtx);
    if (mbedtls_gcm_setkey(gcmCtx, MBEDTLS_CIPHER_ID_AES, key, (keyLen * CAC_MBEDLTS_BITS)) != WSEC_SUCCESS) {
        WSEC_LOG_E1("gcm set key failed in CacCryptGcmInit, mode %d.", mode);
        mbedtls_gcm_free(gcmCtx);
        WSEC_FREE(gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (mbedtls_gcm_starts(gcmCtx, mode, iv, ivLen, NULL, 0) != WSEC_SUCCESS) {
        WSEC_LOG_E1("gcm starts failed in CacCryptGcmInit, mode %d.", mode);
        mbedtls_gcm_free(gcmCtx);
        WSEC_FREE(gcmCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = gcmCtx;
    return WSEC_SUCCESS;
}

static unsigned long CacCryptInit(CacCtx *cacCtx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen, int mode)
{
    int ret = 0;
    const mbedtls_cipher_info_t *info = NULL;
    mbedtls_cipher_context_t *mbedCtx = NULL;

    mbedCtx = WSEC_MALLOC(sizeof(mbedtls_aes_context));
    if (mbedCtx == NULL) {
        WSEC_LOG_E1("aes mbedtls context alloc failed in mode %d.", mode);
        return WSEC_ERR_MALLOC_FAIL;
    }
    mbedtls_cipher_init(mbedCtx);
    do {
        info = MbedtlsGetCipherInfo(algID);
        if (info == NULL) {
            WSEC_LOG_E2("MbedtlsGetCipherInfo in mode %d  failed (-0x%04x).", mode, ret);
            break;
        }
        ret = mbedtls_cipher_setup(mbedCtx, info);
        if (ret != 0) {
            WSEC_LOG_E2("mbedtls_cipher_setup in mode %d  failed (-0x%04x).", mode, ret);
            break;
        }
        ret = mbedtls_cipher_setkey(mbedCtx, key, ((int)keyLen * CAC_MBEDLTS_BITS), mode);
        if (ret != 0) {
            WSEC_LOG_E2("mbedtls_cipher_setkey in mode %d failed (-0x%04x).", mode, ret);
            break;
        }
        ret = mbedtls_cipher_set_iv(mbedCtx, iv, ivLen);
        if (ret != 0) {
            WSEC_LOG_E2("mbedtls_cipher_set_iv in mode %d  failed (-0x%04x).", mode, ret);
            break;
        }
    } while (0);
    if (ret != WSEC_SUCCESS || info == NULL) {
        mbedtls_cipher_free(mbedCtx);
        WSEC_CLEAR_FREE(mbedCtx, sizeof(mbedtls_aes_context));
        mbedCtx = NULL;
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = mbedCtx;
    return WSEC_SUCCESS;
}

/* mbedtls encrypt init implementation */
unsigned long CacEncryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    CacCtx *cacCtx = NULL;
    unsigned long retValue;

    retValue = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv);
    if (retValue != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong encryption check");
        return retValue;
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
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        if (CacCryptGcmInit(cacCtx, key, keyLen, iv, ivLen, MBEDTLS_GCM_ENCRYPT) != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *ctx = (WsecHandle)cacCtx;
        return WSEC_SUCCESS;
    }
    if (CacCryptInit(cacCtx, algID, key, keyLen, iv, ivLen, MBEDTLS_GCM_ENCRYPT) != WSEC_SUCCESS) {
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = cacCtx;
    return WSEC_SUCCESS;
}

/* mbedtls encrypt update implementation */
unsigned long CacEncryptUpdate(WsecHandle ctx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    WsecBuff cipherBuff = { NULL, 0 };
    WsecUint32 protectLen = plainLen + CAC_CRYRT_BUFF_PROTECT_SIZE;
    unsigned long ret;
    CacCtx *cacCtx = (CacCtx *)ctx;

    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        WSEC_LOG_E("Param check fail in CacEncryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (*cipherLen < plainLen) {
        WSEC_LOG_E("Cipher buff len too small");
        return WSEC_ERR_INVALID_ARG;
    }
    /* Based on the mbedtls implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(cipherBuff, protectLen);
    if (cipherBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(cipherBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = MbedtlsEncryptUpdate(cacCtx, plaintext, plainLen, cipherBuff.buff, &cipherBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("mbedtlsEncryptUpdate function return %lu", ret);
            break;
        }

        ret = CopyDest(cipherBuff.buff, cipherBuff.len, ciphertext, cipherLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncryptUpdate copy fail");
            break;
        }
    } while (0);
    WSEC_BUFF_FREE(cipherBuff);
    return ret;
}

static unsigned long CacCryptGcmFinal(CacCtx *cacCtx, unsigned char *tag, WsecUint32 *len, int mode)
{
    int diff;
    unsigned char checkTag[WSEC_AES_GCM_TAGLEN];
    int i;
    memset_s(checkTag, sizeof(checkTag), 0, sizeof(checkTag));
    if (*len < WSEC_AES_GCM_TAGLEN) {
        WSEC_LOG_E("The Input buffer len is not enough for AES_GCM.");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    if (mode == MBEDTLS_GCM_DECRYPT) {
        if (mbedtls_gcm_finish(cacCtx->ctx, checkTag, WSEC_AES_GCM_TAGLEN) != 0) {
            WSEC_LOG_E("mbedtls_gcm_finish decrypt failed in CacCryptGcmFinal.");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        for (diff = 0, i = 0; i < WSEC_AES_GCM_TAGLEN; i++) {
            diff |= tag[i] ^ checkTag[i];
            if (diff != 0) {
                WSEC_LOG_E("AES_GCM decrypt verify tag failed.");
                return WSEC_ERR_HMAC_AUTH_FAIL;
            }
        }
        *len = WSEC_AES_GCM_TAGLEN;
        return WSEC_SUCCESS;
    } else {
        if (mbedtls_gcm_finish(cacCtx->ctx, tag, WSEC_AES_GCM_TAGLEN) != 0) {
            WSEC_LOG_E("mbedtls_gcm_finish encrypt failed in CacCryptGcmFinal.");
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *len = WSEC_AES_GCM_TAGLEN;
        return WSEC_SUCCESS;
    }
}

/* cac encrypt final implementation */
unsigned long CacEncryptFinal(WsecHandle *ctx, unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    CacCtx *cacCtx = NULL;
    WsecBuff cipherBuff = { NULL, 0 };
    int mbedRet;
    unsigned long ret;
    size_t mbedCipherLen;
    WsecUint32 protectLen;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL) {
        /* ctx may be null CacCipherFree will handle this */
        CacCipherFree(ctx);
        return WSEC_ERR_INVALID_ARG;
    }
    cacCtx = (CacCtx *)*ctx;
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        ret = CacCryptGcmFinal(cacCtx, ciphertext, cipherLen, MBEDTLS_GCM_ENCRYPT);
        CacCipherFree(ctx);
        return ret;
    }
    protectLen = (*cipherLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    mbedCipherLen = protectLen;
    WSEC_BUFF_ALLOC(cipherBuff, protectLen);
    if (cipherBuff.buff == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        mbedRet = mbedtls_cipher_finish(cacCtx->ctx, cipherBuff.buff, &mbedCipherLen);
        /* last block size will not out of range */
        if (mbedRet != 0) {
            ret = WSEC_ERR_CRPTO_LIB_FAIL;
            WSEC_LOG_E1("Mbedtls Encrypt Final failed %d.", mbedRet);
            break;
        }
        if (mbedCipherLen > cipherBuff.len) {
            ret = WSEC_ERR_CRPTO_LIB_FAIL;
            WSEC_LOG_E("Mbedtls Encrypt Final exceed buff len.");
            break;
        }
        ret = CopyDest(cipherBuff.buff, (WsecUint32)mbedCipherLen, ciphertext, cipherLen); /* soter 554 */
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncryptFinal copy fail");
            break;
        }
    } while (0);
    CacCipherFree(ctx);
    WSEC_BUFF_FREE(cipherBuff);
    return ret;
}

/* cac decrypt init implementation */
unsigned long CacDecryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    CacCtx *cacCtx = NULL;
    unsigned long retValue;

    retValue = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv);
    if (retValue != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong decryption check");
        return retValue;
    }

    if (keyLen != CacSymmKeyLen(algID) || ivLen != CacSymmIvLen(algID)) {
        WSEC_LOG_E("Wrong encryption IV len or Keylen.");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx = WSEC_MALLOC(sizeof(CacCtx));
    if (cacCtx == NULL) {
        WSEC_LOG_E("Allocate CacCtx memory failed.");
        return WSEC_ERR_MALLOC_FAIL;
    }
    cacCtx->algID = algID;
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        if (CacCryptGcmInit(cacCtx, key, keyLen, iv, ivLen, MBEDTLS_GCM_DECRYPT) != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *ctx = (WsecHandle)cacCtx;
        return WSEC_SUCCESS;
    }
    if (CacCryptInit(cacCtx, algID, key, keyLen, iv, ivLen, MBEDTLS_GCM_DECRYPT) != WSEC_SUCCESS) {
        WSEC_FREE(cacCtx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ctx = cacCtx;
    return WSEC_SUCCESS;
}

/* cac decrypt update implementation */
unsigned long CacDecryptUpdate(WsecHandle ctx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecBuff plainBuff = { NULL, 0 };
    WsecUint32 protectLen = (cipherLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    unsigned long ret;
    CacCtx *cacCtx = (CacCtx *)ctx;

    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        WSEC_LOG_E("Param check fail in CacDecryptUpdate.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (*plainLen < cipherLen) {
        WSEC_LOG_E("Plain buff len too small");
        return WSEC_ERR_INVALID_ARG;
    }
    /* Based on the mbedtls implementation characteristics, the ciphertext buffer needs to be enlarged. */
    WSEC_BUFF_ALLOC(plainBuff, protectLen);
    if (plainBuff.buff == NULL) {
        WSEC_LOG_E4MALLOC(plainBuff.len);
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = MbedtlsDecryptUpdate(cacCtx, ciphertext, cipherLen, plainBuff.buff, &plainBuff.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MbedTls Decrypt update return %lu", ret);
            break;
        }

        ret = CopyDest(plainBuff.buff, plainBuff.len, plaintext, plainLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecryptUpdate copy fail");
            break;
        }
    } while (0);
    WSEC_CLEAR_FREE(plainBuff.buff, plainBuff.len);
    return ret;
}

/* cac decrypt final implementation */
unsigned long CacDecryptFinal(WsecHandle *ctx, unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecBuff plainBuff = { NULL, 0 };
    unsigned long ret;
    int mbedRet;
    size_t mbedPlainLen;
    CacCtx *cacCtx = NULL;
    WsecUint32 protectLen;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL || plaintext == NULL) {
        /* ctx may be null CacCipherFree will handle this */
        CacCipherFree(ctx);
        return WSEC_ERR_INVALID_ARG;
    }
    cacCtx = (CacCtx *)*ctx;
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (CacCryptGcmFinal(cacCtx, plaintext, plainLen, MBEDTLS_GCM_DECRYPT) != 0) {
            WSEC_LOG_E("Mbedtls Decrypt Final CacCryptGcmFinal failed.");
            CacCipherFree(ctx);
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        CacCipherFree(ctx);
        return WSEC_SUCCESS;
    }

    protectLen = (*plainLen + CAC_CRYRT_BUFF_PROTECT_SIZE);
    mbedPlainLen = protectLen;
    WSEC_BUFF_ALLOC(plainBuff, protectLen);
    if (plainBuff.buff == NULL) {
        CacCipherFree(ctx); /* Release the memory. */
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        mbedRet = mbedtls_cipher_finish(cacCtx->ctx, plainBuff.buff, &mbedPlainLen);
        /* mbedPlainLen will not exceed blocklen */
        if (mbedRet != 0) {
            ret = WSEC_ERR_CRPTO_LIB_FAIL;
            WSEC_LOG_E1("Mbedtls Decrypt final failed %d", mbedRet);
            break;
        }
        if (mbedPlainLen > plainBuff.len) {
            ret = WSEC_ERR_CRPTO_LIB_FAIL;
            WSEC_LOG_E("Mbedtls Decrypt final buff exceed.");
            break;
        }
        ret = CopyDest(plainBuff.buff, (WsecUint32)mbedPlainLen, plaintext, plainLen); /* soter 554 */
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("CacDecryptFinal copy failed");
            break;
        }
    } while (0);
    CacCipherFree(ctx);
    WSEC_CLEAR_FREE(plainBuff.buff, plainBuff.len);
    return ret;
}

#endif /* WSEC_COMPILE_CAC_MBEDTLS */
