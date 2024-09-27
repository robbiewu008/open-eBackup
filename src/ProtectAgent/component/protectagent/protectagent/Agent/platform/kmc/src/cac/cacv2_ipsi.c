/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: basic encryption function, adapting to the IPSI interface
 * Author: Luan Shipeng l00171031
 * Create: 2014-10-27
 * Notes: The IPSI version must be VPP_V300R003C28SPC001 or later.
 * History: 2018-10-06 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

/*
 * When OpenSSL is used, an alarm is reported if the Windows W4 file is empty.
 * If only the header file is contained, the alarm can be cleared. However, the PCLint reports the 766 alarm.
 */
#ifdef WSEC_COMPILE_CAC_IPSI
#include "cacv2_pri.h"
#include "securec.h"
#include "sec_crypto.h"
#include "crypto_def.h"
#include "sec_def.h"
#include "sec_sys.h"
#include "ipsi_types.h"
#include "wsecv2_mem.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_lock.h"
#include "wsecv2_util.h"

#define ENTROPY_BLOCK_SIZE (size_t)16
#define APPEND_BLOCK_COUNT 1
#define PBKDF2_FIRST_ITERATION_SALT_MAX_LEN ((SDP_SALT_LEN) * 4)
#define CAC_INIT_PERSONAL_STR "DRBGKMC3"
#define CAC_INIT_RESEED_INTERVAL 0
#define CAC_INIT_HEALTH_CHECK_INTERVAL 0
#define BLOCK_CONTENT_LEN 4
#define PBKDF2_COUNTER_LEN 4
#define FIRST_BLOCK_CONTENT_OFFSET 24
#define SECOND_BLOCK_CONTENT_OFFSET 16
#define THIRD_BLOCK_CONTENT_OFFSET 8
#define AND_OPER_LOW_BYTES 0xFF

/* Cac context */
typedef struct {
    WsecHandle    ctx;
    WsecUint32    algID;
} CacCtx;

/* Random number generator access lock */
static WsecVoid ThreadLockRand(void)
{
    WsecThreadLockById(LOCK4KMC_RAND);
}

/* Random number generator access unlock */
static WsecVoid ThreadUnlockRand(void)
{
    WsecThreadUnlockById(LOCK4KMC_RAND);
}

/* Releasing the Decryption Environment */
void CacCipherFree(WsecHandle *ctx)
{
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || *ctx == NULL) {
        return;
    }
    cacCtx = (CacCtx *)(*ctx);
    if (cacCtx->ctx != NULL) {
        if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
            CRYPT_aeadRemoveSession((CRYPT_AEAD_CTX)cacCtx->ctx);
            cacCtx->ctx = NULL;
        } else {
            crypt_freeCtx((CRYPT_CTX *)&(cacCtx->ctx));
        }
    }
    WSEC_FREE(cacCtx);
}

/* If the value of ctx is not NULL, an exception occurs and a log is recorded. */
static WsecVoid CheckDrbgCtx(const IPSI_DRBG_CTX *ctx)
{
    if (ctx == NULL) {
        WSEC_LOG_I("IPSI DRBG CTX is NULL");
    }
}

/*
 * Callback function for obtaining the entropy value
 * (The first parameter of the function has the pclint plus 818 alarm,
 * but the modification involves platform alignment and intrusive modification of open-source software.)
 * After being reviewed by the network owner and secure coding standard owner, the conclusion is shielded.)
 */
static size_t IpsiGetEntropy(IPSI_DRBG_CTX *ctx,
    SEC_UCHAR **outPointer, SEC_INT entropy, size_t minLen, size_t maxLen)
{
    WsecBool ret = WSEC_FALSE;
    size_t returnLen = ((minLen + (APPEND_BLOCK_COUNT + 1) * ENTROPY_BLOCK_SIZE - 1) / (ENTROPY_BLOCK_SIZE)) *
        (ENTROPY_BLOCK_SIZE);
    WSEC_UNREFER(entropy);
    WSEC_UNREFER(maxLen);
    CheckDrbgCtx(ctx);
    if (returnLen > maxLen) {
        WSEC_LOG_E3("IpsiGetEntropy minLen=%zu maxLen=%zu returnLen=%zu", minLen, maxLen, returnLen);
        return 0;
    }
    ret = WsecGetEntropy(outPointer, returnLen);
    return ret ? returnLen : 0;
}

/*
 * The entropy value buffer is cleared. (The first parameter of the function has the pclint plus 818 alarm.
 * However, the modification involves platform alignment and intrusive modification of open-source software.)
 * After being reviewed by the network owner and secure coding standard owner, the conclusion is shielded.)
 */
static SEC_VOID IpsiCleanupEntropy(IPSI_DRBG_CTX *ctx, SEC_UCHAR *out, size_t outLen)
{
    CheckDrbgCtx(ctx);
    WsecCleanupEntropy(out, outLen);
}

/* ID conversion, which is used to convert the ID of the key management module to the ID of the IPSI. */
static WsecUint32 IpsiDigestAlgToIpsiAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_SHA256):
            return ALGID_SHA256;
        case (WSEC_ALGID_SHA384):
            return ALGID_SHA384;
        case (WSEC_ALGID_SHA512):
            return ALGID_SHA512;
        default:
            return ALGID_UNKNOWN;
    }
}

/* ID conversion, which is used to convert the ID of the key management module to the ID of the IPSI. */
static WsecUint32 IpsiHmacAlgToIpsiAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_HMAC_SHA256):
            return ALGID_HMAC_SHA256;
        case (WSEC_ALGID_HMAC_SHA384):
            return ALGID_HMAC_SHA384;
        case (WSEC_ALGID_HMAC_SHA512):
            return ALGID_HMAC_SHA512;
        default:
            return ALGID_UNKNOWN;
    }
}

/* ID conversion, which is used to convert the ID of the key management module to the ID of the IPSI. */
static WsecUint32 IpsiSymmAlgToIpsiAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_AES128_CBC):
            return ALGID_AES128_CBC;
        case (WSEC_ALGID_AES256_CBC):
            return ALGID_AES256_CBC;
        case (WSEC_ALGID_AES128_GCM):
            return ALGID_AES128_GCM;
        case (WSEC_ALGID_AES256_GCM):
            return ALGID_AES256_GCM;
        default:
            return ALGID_UNKNOWN;
    }
}

/* ID conversion, which is used to convert the ID of the key management module to the ID of the IPSI. */
static WsecUint32 IpsiPbkdf2AlgToIpsiAlg(WsecUint32 algId)
{
    switch (algId) {
        case (WSEC_ALGID_PBKDF2_HMAC_SHA256):
            return ALGID_HMAC_SHA256;
        case (WSEC_ALGID_PBKDF2_HMAC_SHA384):
            return ALGID_HMAC_SHA384;
        case (WSEC_ALGID_PBKDF2_HMAC_SHA512):
            return ALGID_HMAC_SHA512;
        default:
            return ALGID_UNKNOWN;
    }
}

/* Check the HMAC algorithm used in PBKDF2. */
static WsecBool IpsiCheckHmacAlgForPbkdf2(WsecUint32 kdfAlg)
{
    /* Valid are: ALGID_HMAC_SHA256 and ALGID_HMAC_SHA384 ALGID_HMAC_SHA512 */
    if ((kdfAlg == ALGID_HMAC_SHA256)
        || (kdfAlg == ALGID_HMAC_SHA384)
        || (kdfAlg == ALGID_HMAC_SHA512)) {
        return WSEC_TRUE;
    }
    return WSEC_FALSE;
}

/* Self-implemented Pbkdf2 is being prepared. */
static unsigned long IpsiPbkdf2BasedOnHmacPrepare(WsecUint32 kdfAlg,
    const unsigned char *password, WsecUint32 passwordLen,
    WsecUint32 blockCount,
    unsigned char *firstIteration,
    WsecUint32 firstIterationLen,
    unsigned char *tempDigest,
    SEC_UINT32 *tempDigestLen)
{
    SEC_UINT32 ret;
    unsigned char *blockCnt = NULL;
    WSEC_ASSERT(firstIteration != NULL);
    WSEC_ASSERT(firstIterationLen > PBKDF2_COUNTER_LEN); /* PBKDF2 counter 4 bytes */
    WSEC_ASSERT(tempDigest != NULL);
    WSEC_ASSERT(tempDigestLen != NULL);
    int cntIndexBase = 0;
    blockCnt = firstIteration + firstIterationLen - PBKDF2_COUNTER_LEN; /* PBKDF2 counter 4 bytes */
    blockCnt[cntIndexBase++] = (unsigned char)((blockCount >> FIRST_BLOCK_CONTENT_OFFSET) &
        AND_OPER_LOW_BYTES); /* Bit 0 is the 32-bit blockCount right shift 24-bit result. */
    blockCnt[cntIndexBase++] = (unsigned char)((blockCount >> SECOND_BLOCK_CONTENT_OFFSET) &
        AND_OPER_LOW_BYTES); /* Bit 1 is the 32-bit blockCount right shift 16-bit result. */
    /* The second bit indicates the 32-bit blockCount result after right shifting by eight bits. */
    blockCnt[cntIndexBase++] = (unsigned char)((blockCount >> THIRD_BLOCK_CONTENT_OFFSET) &
        AND_OPER_LOW_BYTES);
    blockCnt[cntIndexBase++] = (unsigned char)(blockCount &
        AND_OPER_LOW_BYTES); /* Bit 3: result of 32-bit blockCount and 0xff */
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_hmac(kdfAlg, password, passwordLen, firstIteration, firstIterationLen,
        tempDigest, tempDigestLen);
#else
    ret = CRYPT_hmac(kdfAlg, password, passwordLen, firstIteration, firstIterationLen,
        tempDigest, tempDigestLen, *tempDigestLen);
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_hmac failed %u", (WsecUint32)ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

static unsigned long IpsiFillFirstIterationForPbkdf2(unsigned char *firstIteration, size_t firstIterationLen,
    const unsigned char *salt, WsecUint32 saltLen)
{
    if (firstIteration == NULL || salt == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (saltLen > PBKDF2_FIRST_ITERATION_SALT_MAX_LEN) {
        WSEC_LOG_E1("Salt len (%u) is too long.", saltLen);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (memcpy_s(firstIteration, firstIterationLen, salt, (size_t)saltLen) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    return WSEC_SUCCESS;
}

/* HMAC-based PBKDF2 algorithm */
static unsigned long IpsiPbkdf2BasedOnHmac(WsecUint32 kdfAlg,
    const unsigned char *password, WsecUint32 passwordLen,
    const unsigned char *salt, WsecUint32 saltLen,
    WsecUint32 deriveKeyLen,
    int iter,
    unsigned char *ipsiDerivedKey)
{
    WsecUint32 blockCount = 1;
    int i;
    WsecUint32 bytesCount;
    WsecUint32 currentLen;
    unsigned char tempDigest[MAX_DIGEST_SIZE] = {0};
    SEC_UINT32 hmacLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    unsigned long ret;
    unsigned char firstIteration[PBKDF2_FIRST_ITERATION_SALT_MAX_LEN + 4] = {0}; /* Count of 4-byte groups */
    WsecUint32 remainLen = deriveKeyLen;

    ret = IpsiFillFirstIterationForPbkdf2(firstIteration, sizeof(firstIteration), salt, saltLen);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    if (IpsiCheckHmacAlgForPbkdf2(kdfAlg) == WSEC_FALSE) {
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    hmacLen = (SEC_UINT32)CRYPT_HMAC_size(kdfAlg);
    WSEC_ASSERT(hmacLen <= sizeof(tempDigest));
    while (remainLen != 0) {
        currentLen = (remainLen > hmacLen) ? hmacLen : remainLen;
        /* The length of firstIteration is the salt value and 4-byte blockCnt. */
        ret = IpsiPbkdf2BasedOnHmacPrepare(kdfAlg, password, passwordLen, blockCount,
            firstIteration, saltLen + BLOCK_CONTENT_LEN, tempDigest, &hmacLen);
        if (ret != WSEC_SUCCESS) {
            return ret;
        }

        if (memcpy_s(ipsiDerivedKey, (size_t)currentLen, tempDigest, (size_t)currentLen) != EOK) {
            WSEC_LOG_E4MEMCPY;
            return WSEC_ERR_MEMCPY_FAIL;
        }

        for (i = 1; i < iter; i++) {    /* soter 573 */
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
            ret = CRYPT_hmac(kdfAlg, password, passwordLen, tempDigest, hmacLen, tempDigest, &hmacLen);
#else
            ret = CRYPT_hmac(kdfAlg, password, passwordLen, tempDigest, hmacLen,
                tempDigest, &hmacLen, (SEC_UINT32)sizeof(tempDigest));
#endif
            if (ret != SEC_SUCCESS) {
                return WSEC_ERR_CRPTO_LIB_FAIL;
            }

            for (bytesCount = 0; bytesCount < currentLen; bytesCount++) {
                ipsiDerivedKey[bytesCount] ^= tempDigest[bytesCount];
            }
        }

        remainLen -= currentLen;    /* soter 554 */
        blockCount++;
        ipsiDerivedKey += currentLen;
    }

    return ret;
}

/* IPSI Implementation of the AES_GCM Encryption Algorithm Interface */
unsigned long CacEncryptAesGcm(WsecUint32 algId,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *plaintext, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen)
{
    IPSI_AEAD_SETUP_DATA_S aeadSetupData = { IpsiSymmAlgToIpsiAlg(algId), IPSI_SYM_CIPHER_ENCRYPT, key, keyLen, NULL };
    CRYPT_AEAD_CTX ctx = SEC_NULL;
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_FULL, iv, ivLen, NULL, 0, NULL };
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    size_t outLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    size_t inLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM);
    inLen = (size_t)plaintextLen;
    ret = CRYPT_aeadInitSession(&ctx, &aeadSetupData, NULL);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadInitSession failed %u", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    aeadData.ctx = ctx;

    outLen = *ciphertextLen;
#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen,
        ciphertext, &outLen, outLen,
        (unsigned char *)tag, tagLen, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen,
        ciphertext, &outLen,
        (unsigned char *)tag, tagLen, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen,
        ciphertext, &outLen, outLen,
        (unsigned char *)tag, tagLen, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %u", __FUNCTION__, ret);
        CRYPT_aeadRemoveSession(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *ciphertextLen = (WsecUint32)outLen;
    CRYPT_aeadRemoveSession(ctx);
    return WSEC_SUCCESS;
}

/* IPSI Implementation of the AES_GCM Decryption Algorithm Interface */
unsigned long CacDecryptAesGcm(WsecUint32 algId,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen,
    const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    unsigned char *tag, WsecUint32 tagLen,
    unsigned char *plaintext, WsecUint32 *plaintextLen)
{
    IPSI_AEAD_SETUP_DATA_S aeadSetupData = { IpsiSymmAlgToIpsiAlg(algId), IPSI_SYM_CIPHER_DECRYPT, key, keyLen, NULL };
    CRYPT_AEAD_CTX ctx = SEC_NULL;
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_FULL, iv, ivLen, NULL, 0, NULL };
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    size_t outLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    size_t inLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM);
    if (tagLen < WSEC_AES_GCM_TAGLEN) {
        WSEC_LOG_E("Wrong Encryption input Cipher Length for AES_GCM oper");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    inLen = (size_t)ciphertextLen;
    ret = CRYPT_aeadInitSession(&ctx, &aeadSetupData, NULL);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadInitSession failed %u", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    aeadData.ctx = ctx;
    outLen = *plaintextLen;
#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen,
        plaintext, &outLen, outLen,
        tag, tagLen, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen,
        plaintext, &outLen,
        tag, tagLen, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen,
        plaintext, &outLen, outLen,
        tag, tagLen, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %u", __FUNCTION__, ret);
        CRYPT_aeadRemoveSession(ctx);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *plaintextLen = (WsecUint32)outLen;
    CRYPT_aeadRemoveSession(ctx);
    return WSEC_SUCCESS;
}

/* RNG(DRBG) initialization */
unsigned long CacInitRng(void)
{
    SEC_UCHAR initStr[sizeof(CAC_INIT_PERSONAL_STR)] = CAC_INIT_PERSONAL_STR;
    SEC_UINT32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    IPSI_RAND_SETUP_DATA_S randSetupData = {
        ALGID_HMAC_SHA256, 0, initStr, sizeof(CAC_INIT_PERSONAL_STR) - 1,
        IpsiGetEntropy, IpsiCleanupEntropy, ENTROPY_BLOCK_SIZE, IpsiGetEntropy, IpsiCleanupEntropy,
        NULL, NULL, NULL, NULL, NULL, CAC_INIT_RESEED_INTERVAL, CAC_INIT_HEALTH_CHECK_INTERVAL
    };

    if (WsecIsRngSupplied() == WSEC_TRUE) {
        WSEC_LOG_I("IPSI using rng supplied by app");
        return WSEC_SUCCESS;
    }

    ret = IPSI_CRYPT_enableDrbgMultithread();
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("Call IPSI_CRYPT_enableDrbgMultithread failed : %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    ret = IPSI_CRYPT_enableDrbgSwitchLock();
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("Call IPSI_CRYPT_enableDrbgSwitchLock failed : %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = IPSI_CRYPT_enable_drbg(IPSI_CRYPT_DRBG_ENABLE);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("Call IPSI_CRYPT_enable_drbg failed : %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    ret = IPSI_CRYPT_rand_init(&randSetupData, NULL);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("Call IPSI_CRYPT_rand_init failed : %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

void CacUnInitRng(void)
{
    /* empty function for ipsi */
}

/* Generates a random number. The caller must ensure the validity of the input parameter. */
unsigned long CacRandom(WsecVoid *buff, WsecUint32 buffLen)
{
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecBool randomReturn = WSEC_FALSE;

    WSEC_ASSERT(buff != NULL);

    ThreadLockRand();
    randomReturn = WsecGetRandomNumber((unsigned char *)buff, (size_t)buffLen);
    if (randomReturn == WSEC_TRUE) {
        ThreadUnlockRand();
        return WSEC_SUCCESS;
    }
    ret = IPSI_CRYPT_rand_bytes((SEC_UCHAR *)buff, (SEC_INT)buffLen);
    ThreadUnlockRand();
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("Call CRYPT_random failed : %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    } else {
        return WSEC_SUCCESS;
    }
}

/* Viewing the algorithm type */
WsecUint32 CacAlgIdToType(WsecUint32 algId)
{
    if (IpsiSymmAlgToIpsiAlg(algId) != ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_SYM;
    }

    if (IpsiHmacAlgToIpsiAlg(algId) != ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_HMAC;
    }

    if (IpsiPbkdf2AlgToIpsiAlg(algId) != ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_PBKDF;
    }

    if (IpsiDigestAlgToIpsiAlg(algId) != ALGID_UNKNOWN) {
        return WSEC_ALGTYPE_DIGEST;
    }

    return WSEC_ALGTYPE_UNKNOWN;
}

/* Hash Algorithm Implementation */
unsigned long CacDigest(WsecUint32 algId,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *digestBuff, WsecUint32 *digestLen)
{
    WsecUint32 alg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UCHAR tempDigest[SDP_DIGEST_MAX_SIZE] = {0};
    SEC_UINT32 tempDigestLen = sizeof(tempDigest);
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(digestLen != NULL);

    alg = IpsiDigestAlgToIpsiAlg(algId);
    if (alg == ALGID_UNKNOWN) {
        WSEC_LOG_E("Wrong Digest algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_digest(alg, data, dataLen, tempDigest, &tempDigestLen);
#else
    ret = CRYPT_digest(alg, data, dataLen, tempDigest, &tempDigestLen, (SEC_UINT32)sizeof(tempDigest));
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_digest failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (memcpy_s(digestBuff, (size_t)*digestLen, tempDigest, (size_t)tempDigestLen) != EOK) {
        WSEC_LOG_E2("CacDigest inLen=%lu tempLen=%lu", (unsigned long)*digestLen, (unsigned long)tempDigestLen);
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *digestLen = (WsecUint32)tempDigestLen;
    return WSEC_SUCCESS;
}

/* Initialize the hash operation, transfer the ID, and obtain the handle. */
unsigned long CacDigestInit(WsecHandle *ctx, WsecUint32 algId)
{
    WsecUint32 alg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    alg = IpsiDigestAlgToIpsiAlg(algId);

    WSEC_ASSERT(ctx != NULL);

    if (alg == ALGID_UNKNOWN) {
        WSEC_LOG_E("Wrong Digest algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = CRYPT_digestInit((CRYPT_CTX *)ctx, alg);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_digestinit failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

/* The input data is hashed. The data can be input multiple times. */
unsigned long CacDigestUpdate(const WsecHandle ctx,
    const WsecVoid *data, WsecUint32 dataLen)
{
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    ret = CRYPT_digestUpdate((CRYPT_CTX)ctx, data, dataLen);
    if (ret == SEC_SUCCESS) {
        return WSEC_SUCCESS;    /* soter 669 */
    } else {
        WSEC_LOG_E1("CRYPT_digestUpdate: %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

/* End the hash operation and obtain the hash result. */
unsigned long CacDigestFinal(WsecHandle *ctx, WsecVoid *digestBuff, WsecUint32 *buffLen)
{
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UCHAR tempDigest[SDP_DIGEST_MAX_SIZE] = {0};
    SEC_UINT32 tempDigestLen = sizeof(tempDigest);

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(digestBuff != NULL);
    WSEC_ASSERT(buffLen != NULL);

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_digestFinal((CRYPT_CTX *)ctx, tempDigest, &tempDigestLen);
#else
    ret = CRYPT_digestFinal((CRYPT_CTX *)ctx, tempDigest, &tempDigestLen, (SEC_UINT32)sizeof(tempDigest));
#endif
    *ctx = NULL;
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_DigestFinal failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (memcpy_s(digestBuff, (size_t)*buffLen, tempDigest, (size_t)tempDigestLen) != EOK) {
        WSEC_LOG_E2("CacDigestFinal inLen=%u tempLen=%u", *buffLen, (WsecUint32)tempDigestLen);
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *buffLen = (WsecUint32)tempDigestLen;
    return WSEC_SUCCESS;
}

/* Releases hash handles. */
WsecVoid CacDigestReleaseCtx(WsecHandle *ctx)
{
    SEC_UCHAR digest[SDP_DIGEST_MAX_SIZE] = {0};
    SEC_UINT32 len = sizeof(digest);

    WSEC_ASSERT(ctx != NULL);

    if ((*ctx) == NULL) {
        return;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    WSEC_UNREFER(CRYPT_digestFinal((CRYPT_CTX *)ctx, digest, &len));
#else
    WSEC_UNREFER(CRYPT_digestFinal((CRYPT_CTX *)ctx, digest, &len, (SEC_UINT32)sizeof(digest)));
#endif
    *ctx = NULL;
}

/* Obtains the HMAC length based on the HMAC algorithm ID. */
WsecUint32 CacHMACSize(WsecUint32 algId)
{
    WsecUint32 ipsiAlg = IpsiHmacAlgToIpsiAlg(algId);
    return (ipsiAlg != ALGID_UNKNOWN) ? CRYPT_HMAC_size(ipsiAlg) : 0;
}

/* Calculates the input data based on the specified algorithm, key, and HMAC and returns the calculation result. */
unsigned long CacHmac(WsecUint32 algId, const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *data, WsecUint32 dataLen,
    WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    WsecUint32 alg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UCHAR tempHmac[SDP_HMAC_MAX_SIZE] = {0};
    SEC_UINT32 tempHmacLen = sizeof(tempHmac);

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(data != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

    alg = IpsiHmacAlgToIpsiAlg(algId);
    if (alg == 0) {
        WSEC_LOG_E("Wrong hmac algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_hmac(alg, key, keyLen, data, dataLen, tempHmac, &tempHmacLen);
#else
    ret = CRYPT_hmac(alg, key, keyLen, data, dataLen, tempHmac, &tempHmacLen, (SEC_UINT32)sizeof(tempHmac));
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_hmac failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (memcpy_s(hmacBuff, (size_t)*hmacLen, tempHmac, (size_t)tempHmacLen) != EOK) {
        WSEC_LOG_E2("CacHmac inLen=%lu tempLen=%lu", (unsigned long)*hmacLen, (unsigned long)tempHmacLen);
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *hmacLen = (WsecUint32)tempHmacLen;
    return WSEC_SUCCESS;
}

/* Initialize the HMAC, transfer the ID and key, and obtain the handle. */
unsigned long CacHmacInit(WsecHandle *ctx,
    WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen)
{
    WsecUint32 alg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(key != NULL);

    alg = IpsiHmacAlgToIpsiAlg(algId);
    if (alg == 0) {
        WSEC_LOG_E("Wrong hmac algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    ret = CRYPT_hmacInit((CRYPT_CTX *)ctx, alg, key, keyLen);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_hmacInit failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

/* The input data is HMAC. The data can be input for multiple times. */
unsigned long CacHmacUpdate(WsecHandle ctx, const WsecVoid *data, WsecUint32 dataLen)
{
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(data != NULL);

    if (ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CRYPT_hmacUpdate((CRYPT_CTX)ctx, data, dataLen);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_hmacUpdate failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return WSEC_SUCCESS;
}

/* End the HMAC operation and obtain the HAMC result. */
unsigned long CacHmacFinal(WsecHandle *ctx, WsecVoid *hmacBuff, WsecUint32 *hmacLen)
{
    WsecUint32 ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UCHAR tempHmac[SDP_HMAC_MAX_SIZE] = {0};
    SEC_UINT32 tempHmacLen = sizeof(tempHmac);

    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(hmacBuff != NULL);
    WSEC_ASSERT(hmacLen != NULL);

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_hmacFinal((CRYPT_CTX *)ctx, tempHmac, &tempHmacLen);
#else
    ret = CRYPT_hmacFinal((CRYPT_CTX *)ctx, tempHmac, &tempHmacLen, (SEC_UINT32)sizeof(tempHmac));
#endif
    *ctx = NULL;
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_hmacFinal failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    if (memcpy_s(hmacBuff, (size_t)*hmacLen, tempHmac, (size_t)tempHmacLen) != EOK) {
        WSEC_LOG_E2("CacHmacFinal inLen=%lu tempLen=%lu", (unsigned long)*hmacLen, (unsigned long)tempHmacLen);
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *hmacLen = tempHmacLen;
    return WSEC_SUCCESS;
}

/* Releases the HMAC handle. */
WsecVoid CacHmacReleaseCtx(WsecHandle *ctx)
{
    SEC_UCHAR tempHmac[SDP_HMAC_MAX_SIZE] = {0};
    SEC_UINT32 tempHmacLen = sizeof(tempHmac);

    WSEC_ASSERT(ctx != NULL);

    if ((*ctx) == NULL) {
        return;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    WSEC_UNREFER(CRYPT_hmacFinal((CRYPT_CTX *)ctx, tempHmac, &tempHmacLen));
#else
    WSEC_UNREFER(CRYPT_hmacFinal((CRYPT_CTX *)ctx, tempHmac, &tempHmacLen, (SEC_UINT32)sizeof(tempHmac)));
#endif
    *ctx = NULL;
}

/* Specify the algorithm ID, password, salt value, and iteration times to obtain the derived key. */
unsigned long CacPbkdf2(WsecUint32 kdfAlg,
    const WsecVoid *kdfPassword, WsecUint32 passwordLen,
    const Pbkdf2ParamConst *pbkdf2Param,
    WsecUint32 deriveKeyLen, WsecVoid *derivedKey)
{
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 tempKdfAlg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    if (kdfPassword == NULL || pbkdf2Param == NULL || pbkdf2Param->salt == NULL || pbkdf2Param->saltLen == 0 ||
        derivedKey == NULL) {
        WSEC_LOG_E("firstly check parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (pbkdf2Param->iter <= 0) {
        WSEC_LOG_E("secondly check iter parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    tempKdfAlg = IpsiPbkdf2AlgToIpsiAlg(kdfAlg);
    if (tempKdfAlg == ALGID_UNKNOWN) {
        WSEC_LOG_E("thirdly check tempKsfAlg parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    ret = IpsiPbkdf2BasedOnHmac(tempKdfAlg, kdfPassword,
        passwordLen, pbkdf2Param->salt, pbkdf2Param->saltLen,
        deriveKeyLen, pbkdf2Param->iter,
        derivedKey);
    if (ret != WSEC_SUCCESS) {
        if (ret != WSEC_ERR_MALLOC_FAIL) {
            WSEC_LOG_E1("IpsiPbkdf2BasedOnHmac failed %lu", ret);
        }
        return ret;
    }

    return WSEC_SUCCESS;
}

/* Obtain the block length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmBlockSize(WsecUint32 algId)
{
    WsecUint32 ipsiAlg = IpsiSymmAlgToIpsiAlg(algId);
    return (ipsiAlg != ALGID_UNKNOWN) ? CRYPT_SYM_blockSize(ipsiAlg) : 0;
}

/* Obtains the IV length of the symmetric encryption algorithm based on the algorithm ID. */
WsecUint32 CacSymmIvLen(WsecUint32 algId)
{
    WsecUint32 ipsiAlg = IpsiSymmAlgToIpsiAlg(algId);
    return (ipsiAlg != ALGID_UNKNOWN) ? CRYPT_SYM_ivLen(ipsiAlg) : 0;
}

/* Obtain the key length of the block cipher algorithm based on the algorithm ID. */
WsecUint32 CacSymmKeyLen(WsecUint32 algId)
{
    WsecUint32 ipsiAlg = IpsiSymmAlgToIpsiAlg(algId);
    return (ipsiAlg != ALGID_UNKNOWN) ? CRYPT_SYM_keyLen(ipsiAlg) : 0;
}

/* Specify the algorithm ID, key, IV, and plain text, and encrypt them to obtain the cipher text. */
unsigned long CacEncrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *plaintext, WsecUint32 plaintextLen,
    WsecVoid *ciphertext, WsecUint32 *ciphertextLen)
{
    WsecUint32 alg; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    unsigned long ret;
    WsecUint32 tmpCipherLen = plaintextLen;
    WsecUint32 blockSize; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UINT32 ipsiLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    alg = IpsiSymmAlgToIpsiAlg(algId);
    if (alg == 0) {
        WSEC_LOG_E("Wrong Encryption algId");
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(iv != NULL);
    WSEC_ASSERT(plaintext != NULL);
    WSEC_ASSERT(ciphertext != NULL);
    WSEC_ASSERT(ciphertextLen != NULL);

    /* AES GCM encrypt */
    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (*ciphertextLen < plaintextLen + WSEC_AES_GCM_TAGLEN) {
            WSEC_LOG_E("The Input ciphertext buffer len is not enough for AESGCM");
            return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
        }
        ret = CacEncryptAesGcm(algId, (const unsigned char *)key, keyLen, (const unsigned char *)iv, ivLen,
            (const unsigned char *)plaintext, plaintextLen, (unsigned char *)ciphertext, &tmpCipherLen,
            (unsigned char *)ciphertext + plaintextLen, WSEC_AES_GCM_TAGLEN);
        if (ret != WSEC_SUCCESS || tmpCipherLen != plaintextLen) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *ciphertextLen = plaintextLen + WSEC_AES_GCM_TAGLEN;
        return ret;
    }

    blockSize = CacSymmBlockSize(algId);
    if (*ciphertextLen < (blockSize ? ((plaintextLen / blockSize + 1) * blockSize) : plaintextLen)) {
        WSEC_LOG_E("The Input ciphertext len is not enough, need padding to multipul blockSize");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    CRYPT_SET_PAD_MODE(alg, BLOCK_PADDING_NORMAL);
    ipsiLen = *ciphertextLen;
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = (unsigned long)CRYPT_encrypt(alg, key, keyLen, iv, ivLen, plaintext, plaintextLen, ciphertext, &ipsiLen);
#else
    ret = (unsigned long)CRYPT_encrypt(alg, key, keyLen, iv, ivLen, plaintext, plaintextLen,
        ciphertext, &ipsiLen, ipsiLen);
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_encrypt() fail: %lu", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ciphertextLen = (WsecUint32)ipsiLen;
    return WSEC_SUCCESS;
}

/* Run the decryption action. */
static unsigned long IpsiDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    unsigned long ret;
    unsigned char tag[WSEC_AES_GCM_TAGLEN];
    const unsigned char *tagInCipher = NULL;
    WsecUint32 alg = IpsiSymmAlgToIpsiAlg(algId);
    SEC_UINT32 ipsiRet; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    SEC_UINT32 plainLen = (SEC_UINT32)*plaintextLen;
    if (alg == 0) {
        WSEC_LOG_E1("Wrong Encryption algId value is %u", algId);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    /* Performs AES GCM decryption. */
    if (algId == WSEC_ALGID_AES128_GCM || algId == WSEC_ALGID_AES256_GCM) {
        if (ciphertextLen < WSEC_AES_GCM_TAGLEN) {
            return WSEC_ERR_INVALID_ARG;
        }
        tagInCipher = (const unsigned char *)ciphertext + ciphertextLen - WSEC_AES_GCM_TAGLEN;
        if (memcpy_s(tag, (size_t)WSEC_AES_GCM_TAGLEN, tagInCipher, (size_t)WSEC_AES_GCM_TAGLEN) != EOK) {
            WSEC_LOG_E4MEMCPY;
            return WSEC_ERR_MEMCPY_FAIL;
        }

        ret = CacDecryptAesGcm(algId, (const unsigned char *)key, keyLen, (const unsigned char *)iv, ivLen,
            (const unsigned char *)ciphertext, ciphertextLen - WSEC_AES_GCM_TAGLEN, tag, WSEC_AES_GCM_TAGLEN,
            (unsigned char *)plaintext, plaintextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("IpsiDecrypt failed %lu", ret);
        }
        return ret;
    }

    CRYPT_SET_PAD_MODE(alg, BLOCK_PADDING_NORMAL);
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ipsiRet = (unsigned long)CRYPT_decrypt(alg, key, keyLen, iv, ivLen, ciphertext, ciphertextLen,
        plaintext, &plainLen);
#else
    ipsiRet = (unsigned long)CRYPT_decrypt(alg, key, keyLen, iv, ivLen, ciphertext, ciphertextLen,
        plaintext, &plainLen, plainLen);
#endif
    if (ipsiRet != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_decrypt failed %lu", (unsigned long)ipsiRet);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plaintextLen = (WsecUint32)plainLen;

    return WSEC_SUCCESS;
}

/* Specify the algorithm ID, key, IV, and ciphertext, and decrypt them to obtain the plaintext. */
unsigned long CacDecrypt(WsecUint32 algId,
    const WsecVoid *key, WsecUint32 keyLen,
    const WsecVoid *iv, WsecUint32 ivLen,
    const WsecVoid *ciphertext, WsecUint32 ciphertextLen,
    WsecVoid *plaintext, WsecUint32 *plaintextLen)
{
    unsigned char *plainBuff = NULL;
    WsecUint32 plainBuffLen = ciphertextLen + CAC_CRYRT_BUFF_PROTECT_SIZE;
    unsigned long ret;

    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(iv != NULL);
    WSEC_ASSERT(ciphertext != NULL);
    WSEC_ASSERT(plaintext != NULL);
    WSEC_ASSERT(plaintextLen != NULL);

    /*
     * If the buffer length is less than an integer multiple, a failure message is returned.
     * In this case, the plaintext buffer needs to be enlarged.
     **/
    plainBuff = (unsigned char *)WSEC_MALLOC(plainBuffLen);
    if (plainBuff == NULL) {
        WSEC_LOG_E4MALLOC(plainBuffLen);
        return WSEC_ERR_MALLOC_FAIL;
    }

    ret = IpsiDecrypt(algId, key, keyLen, iv, ivLen, ciphertext, ciphertextLen, plainBuff, &plainBuffLen);
    if (ret == WSEC_SUCCESS) {
        if (memcpy_s(plaintext, (size_t)*plaintextLen, plainBuff, (size_t)plainBuffLen) != EOK) {
            WSEC_LOG_E2("plaintext buffer length not enough %u %u", plainBuffLen, (*plaintextLen));
            ret = WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *plaintextLen = plainBuffLen;
    }
    WSEC_CLEAR_FREE(plainBuff, plainBuffLen);

    return ret;
}

/* Checking the Init Parameter for Stream Encryption and Decryption and Obtaining the Algorithm Type */
static unsigned long CheckEncDecParamAndGetAlgType(const WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, const unsigned char *iv, WsecUint32 *algType)
{
    WSEC_ASSERT(algType != NULL);
    *algType = IpsiSymmAlgToIpsiAlg(algID);
    if (*algType == 0) {
        WSEC_LOG_E1("Unknown AlgID %u", algID);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    if (ctx == NULL || key == NULL || iv == NULL) {
        WSEC_LOG_E("Param check fail in ipsi");
        return WSEC_ERR_INVALID_ARG;
    }
    return WSEC_SUCCESS;
}

static unsigned long CacEncryptAesGcmInit(CacCtx *cacCtx,
    const unsigned char *key, WsecUint32 keyLen, const unsigned char *iv, WsecUint32 ivLen)
{
    SEC_UINT32 ret;
    CRYPT_AEAD_CTX aeadCtx = SEC_NULL;
    IPSI_AEAD_SETUP_DATA_S aeadSetupData = {0, IPSI_SYM_CIPHER_ENCRYPT, NULL, 0, NULL};
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_PARTIAL, NULL, 0, NULL, 0, NULL };
    aeadSetupData.uiAEADAlgID = IpsiSymmAlgToIpsiAlg(cacCtx->algID);
    aeadSetupData.pucKey = key;
    aeadSetupData.uiKeyLen = keyLen;
    /* Initialize the context with session information */
    ret = CRYPT_aeadInitSession(&aeadCtx, &aeadSetupData, SEC_NULL);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadInitSession failed %lu", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    aeadData.ctx = aeadCtx;
    /* For PARTIAL mode, IV should be set for first operation only */
    aeadData.pucIV = iv;
    aeadData.uiIvLen = (size_t)ivLen;

#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, SEC_NULL, 0, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, SEC_NULL, 0, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, SEC_NULL, 0, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        CRYPT_aeadRemoveSession(aeadCtx);
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %lu", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    cacCtx->ctx = (WsecHandle)aeadCtx;
    return WSEC_SUCCESS;
}

/* Symmetric stream encryption (start) */
unsigned long CacEncryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    WsecUint32 algType;
    SEC_UINT32 ret;
    unsigned long result;
    CacCtx *cacCtx = NULL;
    result = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv, &algType);
    if (result != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong encryption check");
        return result;
    }

    cacCtx = (CacCtx *)WSEC_MALLOC(sizeof(CacCtx));
    if (cacCtx == NULL) {
        WSEC_LOG_E1("%s allocate CacCtx memory failed.", __FUNCTION__);
        return WSEC_ERR_MALLOC_FAIL;
    }
    cacCtx->algID = algID;
    /* AES GCM encrypt */
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        result = CacEncryptAesGcmInit(cacCtx, key, keyLen, iv, ivLen);
        if (result != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
        }
        *ctx = (WsecHandle)cacCtx;
        return result;
    }

    CRYPT_SET_PAD_MODE(algType, BLOCK_PADDING_NORMAL);
    ret = CRYPT_encryptInit((CRYPT_CTX *)&cacCtx->ctx, algType,
        (const SEC_UCHAR *)key, (SEC_UINT32)keyLen, (const SEC_UCHAR *)iv, (SEC_UINT32)ivLen);
    if (ret != SEC_SUCCESS) {
        WSEC_FREE(cacCtx);
        WSEC_LOG_E2("%s CRYPT_encryptInit failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ctx = (WsecHandle)cacCtx;
    return WSEC_SUCCESS;
}

static unsigned long CacEncryptAesGcmUpdate(CacCtx *cacCtx,
    const unsigned char *plaintext, WsecUint32 plaintextLen, unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    SEC_UINT32 ret;
    size_t outLen;
    size_t inLen;
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_PARTIAL, NULL, 0, NULL, 0, NULL };
    aeadData.ctx = (CRYPT_AEAD_CTX)cacCtx->ctx;
    inLen = (size_t)plaintextLen;
    outLen = (size_t)*ciphertextLen;

#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen, ciphertext, &outLen, outLen, SEC_NULL, 0, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen, ciphertext, &outLen, SEC_NULL, 0, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, plaintext, inLen, ciphertext, &outLen, outLen, SEC_NULL, 0, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ciphertextLen = (WsecUint32)outLen;
    return WSEC_SUCCESS;
}

/* Symmetric stream encryption (add) */
unsigned long CacEncryptUpdate(WsecHandle ctx,
    const unsigned char *plaintext, WsecUint32 plainLen,
    unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    SEC_UINT32 ret;
    unsigned long result;
    SEC_UINT32 ciphertextLen;
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        WSEC_LOG_E1("%s input param invalid.", __FUNCTION__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (*cipherLen < plainLen) {
        WSEC_LOG_E("The Input ciphertext buffer len for update is not enough");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    cacCtx = (CacCtx *)ctx;
    ciphertextLen = (SEC_UINT32)*cipherLen;
    /* AES GCM encrypt */
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        result = CacEncryptAesGcmUpdate(cacCtx, plaintext, plainLen, ciphertext, (WsecUint32 *)&ciphertextLen);
        if (result != WSEC_SUCCESS || ciphertextLen != plainLen) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *cipherLen = ciphertextLen;
        return result;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_encryptUpdate((CRYPT_CTX)cacCtx->ctx, (const SEC_UCHAR *)plaintext, plainLen,
        ciphertext, &ciphertextLen);
#else
    ret = CRYPT_encryptUpdate((CRYPT_CTX)cacCtx->ctx, (const SEC_UCHAR *)plaintext, plainLen,
        ciphertext, &ciphertextLen, ciphertextLen);
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_encryptUpdate failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }

    *cipherLen = ciphertextLen;
    return WSEC_SUCCESS;
}

static unsigned long CacEncryptAesGcmFinal(CacCtx *cacCtx, unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    SEC_UINT32 ret;
    /* Encrypt pending data as LAST PARTIAL */
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_LAST_PARTIAL, NULL, 0, NULL, 0, NULL };
    SEC_UINT32 tagLen = WSEC_AES_GCM_TAGLEN;
    unsigned char *tag = ciphertext;
    aeadData.ctx = (CRYPT_AEAD_CTX)cacCtx->ctx;
    WSEC_ASSERT(*ciphertextLen >= tagLen);
    /* As this is LAST PARTIAL block, tag buffer & tag length should be passed to API */
#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, tag, tagLen, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, tag, tagLen, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, tag, tagLen, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadOp get tag failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ciphertextLen = (WsecUint32)tagLen;
    return SEC_SUCCESS;
}

/* Symmetric stream encryption (end) */
unsigned long CacEncryptFinal(WsecHandle *ctx, unsigned char *ciphertext, WsecUint32 *cipherLen)
{
    SEC_UINT32 ret;
    unsigned long result;
    SEC_UINT32 ciphertextLen;
    CacCtx *cacCtx = NULL;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL) {
        CacCipherFree(ctx);
        return WSEC_ERR_INVALID_ARG;
    }

    cacCtx = (CacCtx *)*ctx;
    ciphertextLen = (SEC_UINT32)*cipherLen;

    /* AES GCM encrypt */
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        if (ciphertextLen < WSEC_AES_GCM_TAGLEN) {
            WSEC_LOG_E("The Input ciphertext buffer len is not enough for AES_GCM.");
            return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
        }
        result = CacEncryptAesGcmFinal(cacCtx, (unsigned char *)ciphertext, (WsecUint32 *)&ciphertextLen);
        if (result != WSEC_SUCCESS || ciphertextLen != WSEC_AES_GCM_TAGLEN) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *cipherLen = (WsecUint32)ciphertextLen;
        return result;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_encryptFinal((CRYPT_CTX *)&cacCtx->ctx, ciphertext, &ciphertextLen);
#else
    ret = CRYPT_encryptFinal((CRYPT_CTX *)&cacCtx->ctx, ciphertext, &ciphertextLen, ciphertextLen);
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_encryptFinal failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *cipherLen = (WsecUint32)ciphertextLen;
    return WSEC_SUCCESS;
}

static unsigned long CacDecryptAesGcmInit(CacCtx *cacCtx,
    const unsigned char *key, WsecUint32 keyLen, const unsigned char *iv, WsecUint32 ivLen)
{
    SEC_UINT32 ret;
    CRYPT_AEAD_CTX aeadCtx = SEC_NULL;
    IPSI_AEAD_SETUP_DATA_S aeadSetupData = {0, IPSI_SYM_CIPHER_DECRYPT, NULL, 0, NULL};
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_PARTIAL, NULL, 0, NULL, 0, NULL };
    aeadSetupData.uiAEADAlgID = IpsiSymmAlgToIpsiAlg(cacCtx->algID);
    aeadSetupData.pucKey = key;
    aeadSetupData.uiKeyLen = keyLen;
    aeadData.pucIV = iv;  /* For PARTIAL mode, IV & AAD should be set for first operation only */
    aeadData.uiIvLen = ivLen;
    /* Initialize the context with session information */
    ret = CRYPT_aeadInitSession(&aeadCtx, &aeadSetupData, SEC_NULL);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadInitSession failed %u.", __FUNCTION__, ret);
        return ret;
    }
    aeadData.ctx = aeadCtx;

#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, SEC_NULL, 0, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, SEC_NULL, 0, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, SEC_NULL, 0, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        CRYPT_aeadRemoveSession(aeadCtx);
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    cacCtx->ctx = (WsecHandle)aeadCtx;
    return SEC_SUCCESS;
}

/* Symmetric decryption of stream data (start) */
unsigned long CacDecryptInit(WsecHandle *ctx, WsecUint32 algID,
    const unsigned char *key, WsecUint32 keyLen,
    const unsigned char *iv, WsecUint32 ivLen)
{
    WsecUint32 algType;
    SEC_UINT32 ret;
    unsigned long result;
    CacCtx *cacCtx = NULL;
    result = CheckEncDecParamAndGetAlgType(ctx, algID, key, iv, &algType);
    if (result != WSEC_SUCCESS) {
        WSEC_LOG_E("Wrong decryption check");
        return result;
    }

    cacCtx = (CacCtx *)WSEC_MALLOC(sizeof(CacCtx));
    if (cacCtx == NULL) {
        WSEC_LOG_E1("%s allocate CacCtx memory failed.", __FUNCTION__);
        return WSEC_ERR_MALLOC_FAIL;
    }
    cacCtx->algID = algID;
    /* AES GCM Decrypt */
    if (algID == WSEC_ALGID_AES128_GCM || algID == WSEC_ALGID_AES256_GCM) {
        result = CacDecryptAesGcmInit(cacCtx, key, keyLen, iv, ivLen);
        if (result != WSEC_SUCCESS) {
            WSEC_FREE(cacCtx);
            return result;
        }
        *ctx = (WsecHandle)cacCtx;
        return result;
    }

    CRYPT_SET_PAD_MODE(algType, BLOCK_PADDING_NORMAL);
    ret = CRYPT_decryptInit((CRYPT_CTX *)&cacCtx->ctx, algType, key, keyLen, iv, ivLen);
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_decryptInit failed %u", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *ctx = (WsecHandle)cacCtx;
    return WSEC_SUCCESS;
}

static unsigned long CacDecryptAesGcmUpdate(CacCtx *cacCtx,
    const unsigned char *ciphertext, WsecUint32 cipherLen, unsigned char *plaintext, WsecUint32 *plainLen)
{
    SEC_UINT32 ret;
    size_t outLen;
    size_t inLen;
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_PARTIAL, NULL, 0, NULL, 0, NULL };
    aeadData.ctx = (CRYPT_AEAD_CTX)cacCtx->ctx;

    WSEC_ASSERT(*plainLen >= cipherLen);

    inLen = (size_t)cipherLen;
    outLen = (size_t)*plainLen;

#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen, plaintext, &outLen, outLen, SEC_NULL, 0, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen, plaintext, &outLen, SEC_NULL, 0, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, ciphertext, inLen, plaintext, &outLen, outLen, SEC_NULL, 0, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        CRYPT_aeadRemoveSession((CRYPT_AEAD_CTX)cacCtx->ctx);
        cacCtx->ctx = NULL;
        WSEC_LOG_E2("%s CRYPT_aeadOp failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plainLen = (WsecUint32)outLen;
    return SEC_SUCCESS;
}

/* Symmetric decryption of stream data (add) */
unsigned long CacDecryptUpdate(WsecHandle ctx,
    const unsigned char *ciphertext, WsecUint32 cipherLen,
    unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecUint32 ret;
    unsigned long result;
    SEC_UINT32 plaintextLen;
    CacCtx *cacCtx = NULL;
    if (ctx == NULL || ((CacCtx *)ctx)->ctx == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (*plainLen < cipherLen) {
        WSEC_LOG_E("The Input plaintext buffer len is not enough , make sure it is at least cipherLen+blocksize.");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    cacCtx = (CacCtx *)ctx;
    plaintextLen = (SEC_UINT32)*plainLen;

    /* AES GCM encrypt */
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        result = CacDecryptAesGcmUpdate(cacCtx, ciphertext, cipherLen, plaintext, (WsecUint32 *)&plaintextLen);
        if (result != WSEC_SUCCESS) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *plainLen = plaintextLen;
        return result;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_decryptUpdate((CRYPT_CTX)cacCtx->ctx, ciphertext, cipherLen, plaintext, &plaintextLen);
#else
    ret = CRYPT_decryptUpdate((CRYPT_CTX)cacCtx->ctx, ciphertext, cipherLen, plaintext, &plaintextLen, plaintextLen);
#endif
    if (ret == SEC_SUCCESS) {
        *plainLen = (WsecUint32)plaintextLen;
        return WSEC_SUCCESS;
    } else {
        WSEC_LOG_E1("CRYPT_decryptUpdate failed %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
}

static unsigned long CacDecryptAesGcmFinal(CacCtx *cacCtx, unsigned char *tag, unsigned int tagLen)
{
    SEC_UINT32 ret;
    IPSI_AEAD_OP_DATA_S aeadData = { NULL, IPSI_SYM_DATA_TYPE_LAST_PARTIAL, NULL, 0, NULL, 0, NULL };
    aeadData.ctx = (CRYPT_AEAD_CTX)cacCtx->ctx;
#ifdef HERT_RAT_COMPATIBILITY /* This macro uses the VPP:V300R003C29SPC204B040 HERT customized macro. */
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, tag, (SEC_UINT32)tagLen, SEC_NULL);
#else
#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, tag, (SEC_UINT32)tagLen, SEC_NULL);
#else
    ret = CRYPT_aeadOp(&aeadData, SEC_NULL, 0, SEC_NULL, SEC_NULL, 0, tag, (SEC_UINT32)tagLen, SEC_NULL);
#endif
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E2("%s CRYPT_aeadOp check tag failed %u.", __FUNCTION__, ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    return SEC_SUCCESS;
}

/* Symmetric decryption of stream data (end) */
unsigned long CacDecryptFinal(WsecHandle *ctx, unsigned char *plaintext, WsecUint32 *plainLen)
{
    WsecUint32 ret;
    unsigned long result;
    SEC_UINT32 plaintextLen;
    CacCtx *cacCtx = NULL;
    if (*ctx == NULL || ((CacCtx *)*ctx)->ctx == NULL) {
        CacCipherFree(ctx);
        return WSEC_ERR_INVALID_ARG;
    }

    cacCtx = (CacCtx *)*ctx;
    plaintextLen = (SEC_UINT32)*plainLen;

    /* AES GCM encrypt */
    if (cacCtx->algID == WSEC_ALGID_AES128_GCM || cacCtx->algID == WSEC_ALGID_AES256_GCM) {
        WSEC_ASSERT(*plainLen == WSEC_AES_GCM_TAGLEN);  // tagLen is defined by IPSI
        result = CacDecryptAesGcmFinal(cacCtx, plaintext, WSEC_AES_GCM_TAGLEN);
        if (result != WSEC_SUCCESS) {
            return WSEC_ERR_CRPTO_LIB_FAIL;
        }
        *plainLen = WSEC_AES_GCM_TAGLEN;
        return result;
    }

#ifndef WSEC_IPSI_BUFF_WITH_TWO_LEN
    ret = CRYPT_decryptFinal((CRYPT_CTX *)&cacCtx->ctx, (SEC_UCHAR *)plaintext, &plaintextLen);
#else
    ret = CRYPT_decryptFinal((CRYPT_CTX *)&cacCtx->ctx, (SEC_UCHAR *)plaintext, &plaintextLen, plaintextLen);
#endif
    if (ret != SEC_SUCCESS) {
        WSEC_LOG_E1("CRYPT_decryptFinal failed %u.", ret);
        return WSEC_ERR_CRPTO_LIB_FAIL;
    }
    *plainLen = (WsecUint32)plaintextLen;
    return WSEC_SUCCESS;
}

#endif /* WSEC_COMPILE_CAC_IPSI */
