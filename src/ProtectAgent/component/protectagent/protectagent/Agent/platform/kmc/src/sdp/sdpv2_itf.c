/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Implementation of the data protection module in SDP V2
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "sdpv2_itf.h"
#include "securec.h"
#include "sdpv3_alg.h"
#include "cacv2_pri.h"
#include "kmcv2_pri.h"
#include "kmcv2_ksm.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_order.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"
#include "kmcv2_itf.h"
#include "sdpv1_itf.h"

#ifndef WSEC_COMPILE_SDP
#error Please defined 'WSEC_COMPILE_SDP' to compile
#endif

typedef struct TagCipherParam{
    unsigned char *key;
    WsecUint32     keyLen;
    WsecUint32     ivLen;
} CipherParam;

typedef struct TagHmacParam{
    unsigned char *hmacKey;
    WsecUint32     hmacKeyLen;
} HmacParam;

#pragma pack(1)
typedef struct TagSdpCipherAndHmacHeaders{
    SdpCipherHeaderEx cipherHeader;
    SdpHmacHeaderEx   hmacHeader;
} SdpCipherAndHmacHeaders;
#pragma pack()

/* Fill the salt and iv fields in SdpCipherHeaderEx and derive the cipher key. */
static unsigned long FillIvSaltAndDeriveCipherKey(WsecUint32 cipherAlgId,
    const unsigned char *mk, WsecUint32 mkLen,
    SdpCipherHeaderEx *cipherHeader,
    CipherParam *cipherParam)
{
    unsigned long returnValue; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    WsecUint32 ivLength = 0;
    WsecUint32 keyLen = 0;
    Pbkdf2ParamConst pbkdf2Param;

    WSEC_ASSERT(mk != NULL);
    WSEC_ASSERT(cipherHeader != NULL);
    WSEC_ASSERT(cipherParam != NULL);
    WSEC_ASSERT(cipherParam->key != NULL);
    /* Check the algorithm ID and obtain the key and IV length. */
    returnValue = SdpGetAlgPropertyEx(cipherAlgId, &algType, &keyLen, &ivLength, NULL);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] Get algorithm property failed.");
        return returnValue;
    }
    if (cipherParam->keyLen < keyLen || (WsecUint32)sizeof(cipherHeader->iv) < ivLength) {
        WSEC_LOG_E3("[SDP] Key or IV length not enough. %u %u %u", cipherParam->keyLen, keyLen, ivLength);
        return WSEC_ERR_INVALID_ARG;
    }
    if (algType != WSEC_ALGTYPE_SYM) {
        WSEC_LOG_E2("[SDP] Alg (%u) type (%u) is not symmetric alg type.", cipherAlgId, algType);
        return WSEC_ERR_SDP_ALG_NOT_SUPPORTED;
    }

    /* Randomly obtain the salt value. */
    returnValue = CacRandom(cipherHeader->salt, (WsecUint32)sizeof(cipherHeader->salt));
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC calculate random failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }
    /* Generate IV */
    returnValue = CacRandom(cipherHeader->iv, ivLength);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC calculate random failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }

    /* Deriving a Working Key Based on the Master Key */
    pbkdf2Param.salt = cipherHeader->salt;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(cipherHeader->salt);
    pbkdf2Param.iter = 1;
    returnValue = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, mk, mkLen, &pbkdf2Param, keyLen, cipherParam->key);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC pbkdf2 derive encrypt key failed.");
        return WSEC_ERR_PBKDF2_FAIL;
    }
    cipherParam->keyLen = keyLen;
    cipherParam->ivLen = ivLength;
    return WSEC_SUCCESS;
}

/* Fills the salt field in SdpHmacHeaderEx and derives the HMAC key. */
static unsigned long FillSaltHmacLenAndDeriveHmacKey(WsecUint32 hmacAlgId,
    WsecUint32 cipherVersion,
    const unsigned char *mk, WsecUint32 mkLen,
    SdpHmacHeaderEx *hmacHeader,
    HmacParam *hmacParam)
{
    unsigned long returnValue; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    WsecUint32 hmacKeyLen;
    WsecUint32 hmacLen = 0;
    Pbkdf2ParamConst pbkdf2Param;

    WSEC_ASSERT(mk != NULL);
    WSEC_ASSERT(hmacHeader != NULL);
    WSEC_ASSERT(hmacParam != NULL);
    WSEC_ASSERT(hmacParam->hmacKey != NULL);

    /* Check the HMAC algorithm and obtain the HMAC key length and HMAC length. */
    returnValue = SdpGetAlgPropertyEx(hmacAlgId, &algType, &hmacKeyLen, NULL, &hmacLen);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] Get algorithm property failed.");
        return returnValue;
    }
    if (algType != WSEC_ALGTYPE_HMAC) {
        WSEC_LOG_E2("[SDP] alg(%u) type(%u) not HMAC.", hmacAlgId, algType);
        return WSEC_ERR_SDP_ALG_NOT_SUPPORTED;
    }

    /* The length of V2 is fixed at 16 bytes, which does not comply with the specifications. */
    if (cipherVersion == SDP_CIPHER_TEXT_VER2) {
        hmacKeyLen = SDP_HMAC_DERIVE_KEY_LEN;
    }

    if (hmacParam->hmacKeyLen < hmacKeyLen) {
        WSEC_LOG_E2("[SDP] hmac length not enough %u - %u.", hmacParam->hmacKeyLen, hmacKeyLen);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Salt calculation */
    returnValue = CacRandom(hmacHeader->salt, (WsecUint32)sizeof(hmacHeader->salt));
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC calculate random failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }

    /* Deriving a Working Key Based on the Master Key */
    pbkdf2Param.salt = hmacHeader->salt;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(hmacHeader->salt);
    pbkdf2Param.iter = 1;
    returnValue = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, mk, mkLen, &pbkdf2Param, hmacKeyLen, hmacParam->hmacKey);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC pbkdf2 derive hmac key failed.");
        return WSEC_ERR_PBKDF2_FAIL;
    }
    hmacHeader->hmacLen = hmacLen;
    hmacParam->hmacKeyLen = hmacKeyLen;

    return WSEC_SUCCESS;
}

/* Check the ciphertext header. */
static unsigned long CheckCipherTextHdr(const SdpCipherHeaderEx *cipherHeader)
{
    WsecBool cipherHeaderInvalid = (cipherHeader == NULL ||
        ((cipherHeader->cipherVersion != SDP_CIPHER_TEXT_VER2) &&
            (cipherHeader->cipherVersion != SDP_CIPHER_TEXT_VER3)) ||
        (!(cipherHeader->hmacFlag == WSEC_FALSE || cipherHeader->hmacFlag == WSEC_TRUE)) ||
        (CacAlgIdToType(cipherHeader->cipherAlgId) == WSEC_ALGTYPE_UNKNOWN) ||
        (cipherHeader->ciphertextLen == 0));
    if (cipherHeaderInvalid) {
        return WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
    }
    return WSEC_SUCCESS;
}

/* Byte order conversion, which is used to convert the ciphertext header */
static WsecVoid CvtByteOrderForCipherHdrEx(SdpCipherHeaderEx *cipherHeader, WsecUint32 direction)
{
    if (direction == WBCHOST2NETWORK) {
        cipherHeader->cipherVersion = WSEC_H2N_L(cipherHeader->cipherVersion);
        cipherHeader->hmacFlag = WSEC_H2N_L(cipherHeader->hmacFlag);
        cipherHeader->domainId = WSEC_H2N_L(cipherHeader->domainId);
        cipherHeader->keyId = WSEC_H2N_L(cipherHeader->keyId);
        cipherHeader->cipherAlgId = WSEC_H2N_L(cipherHeader->cipherAlgId);
        cipherHeader->ciphertextLen = WSEC_H2N_L(cipherHeader->ciphertextLen);
    } else {
        cipherHeader->cipherVersion = WSEC_N2H_L(cipherHeader->cipherVersion);
        cipherHeader->hmacFlag = WSEC_N2H_L(cipherHeader->hmacFlag);
        cipherHeader->domainId = WSEC_N2H_L(cipherHeader->domainId);
        cipherHeader->keyId = WSEC_N2H_L(cipherHeader->keyId);
        cipherHeader->cipherAlgId = WSEC_N2H_L(cipherHeader->cipherAlgId);
        cipherHeader->ciphertextLen = WSEC_N2H_L(cipherHeader->ciphertextLen);
    }
}

/* Converts the byte order and the HMAC header. */
static WsecVoid CvtByteOrderForHmacHdrEx(SdpHmacHeaderEx *hmacHeader, WsecUint32 direction)
{
    if (direction == WBCHOST2NETWORK) {
        hmacHeader->hmacAlgId = WSEC_H2N_L(hmacHeader->hmacAlgId);
        hmacHeader->hmacLen = WSEC_H2N_L(hmacHeader->hmacLen);
    } else {
        hmacHeader->hmacAlgId = WSEC_N2H_L(hmacHeader->hmacAlgId);
        hmacHeader->hmacLen = WSEC_N2H_L(hmacHeader->hmacLen);
    }
}

/* Fill the cipher-text header and return the key, key length, and IV length. */
static unsigned long FillCipherTextHeaderEx(WsecUint32 cipherAlgId,
    WsecUint32 domain,
    SdpCipherHeaderEx *cipherHeader,
    CipherParam *cipherParam)
{
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 keyId = 0;
    WsecUint32 mkLen = WSEC_MK_LEN_MAX;
    unsigned char mk[WSEC_MK_LEN_MAX] = {0};
    unsigned char mkHash[WSEC_MK_HASH_REC_LEN] = {0};

    /* Verify Parameter Value */
    WSEC_ASSERT(cipherParam != NULL);
    WSEC_ASSERT(cipherParam->key != NULL);

    /* Obtaining the Current Effective Master Key */
    ret = KmcGetActiveMkWithHash(domain, mk, &mkLen, &keyId, mkHash, sizeof(mkHash));
    if (ret != WSEC_SUCCESS) {
        (void)memset_s(mk, sizeof(mk), 0, sizeof(mk));
        WSEC_LOG_E1("[SDP] FillCipherTextHeaderEx - get MK failed %lu.", ret);
        return ret;
    }

    /* Deriving a Working Key Based on the Master Key */
    ret = FillIvSaltAndDeriveCipherKey(cipherAlgId, mk, mkLen, cipherHeader, cipherParam);
    /* Destroying the stack master key */
    (void)memset_s(mk, sizeof(mk), 0, sizeof(mk));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] FillIvSaltAndDeriveCipherKey failed %lu", ret);
        return ret;
    }

    /*
     * For encryption without HMAC calculation, SDP_CIPHER_TEXT_VER2 and SDP_CIPHER_TEXT_VER3 are encrypted.
     * Ciphertext calculation is the same. However, for the sake of interconnection scenarios such as rollback,
     * encryption on new nodes, and decryption on old nodes,
     * Use V2. Otherwise, the verification in CheckCipherTextHdr of the earlier version fails.
     */
    cipherHeader->cipherVersion = SDP_CIPHER_TEXT_VER2;
    cipherHeader->hmacFlag = WSEC_FALSE;
    cipherHeader->domainId = domain;
    cipherHeader->cipherAlgId = cipherAlgId;
    cipherHeader->keyId = keyId;
    (void)memcpy_s(cipherHeader->mkHashId, (size_t)WSEC_MK_HASH_REC_LEN, mkHash, (size_t)WSEC_MK_HASH_REC_LEN);

    return WSEC_SUCCESS;
}

/*
 * Fill in the cipher-text header and HMAC header and return the encryption
 * key and length, IV length, and HMAC key and length.
 */
static unsigned long FillHeaderWithHmacEx(WsecUint32 cipherAlgId,
    WsecUint32 hmacAlgId,
    WsecUint32 cipherVersion,
    WsecUint32 domain,
    SdpCipherAndHmacHeaders *head,
    CipherParam *cipherParam,
    HmacParam *hmacParam)
{
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 keyId = 0;
    WsecUint32 mkLen = WSEC_MK_LEN_MAX;
    unsigned char mk[WSEC_MK_LEN_MAX] = {0};
    unsigned char mkHash[WSEC_MK_HASH_REC_LEN] = {0};

    /* Verify Parameter Value */
    WSEC_ASSERT(head != NULL);
    WSEC_ASSERT(cipherParam != NULL);
    WSEC_ASSERT(cipherParam->key != NULL);
    WSEC_ASSERT(hmacParam != NULL);
    WSEC_ASSERT(hmacParam->hmacKey != NULL);
    WSEC_ASSERT(cipherVersion == SDP_CIPHER_TEXT_VER2 || cipherVersion == SDP_CIPHER_TEXT_VER3);

    /* Obtaining the Current Effective Master Key */
    ret = KmcGetActiveMkWithHash(domain, mk, &mkLen, &keyId, mkHash, sizeof(mkHash));
    if (ret != WSEC_SUCCESS) {
        (void)memset_s(mk, sizeof(mk), 0, sizeof(mk));
        WSEC_LOG_E1("[SDP] FillHeaderWithHmacEx - get MK failed %lu", ret);
        return ret;
    }

    /* Deriving a Working Key Based on the Master Key */
    ret = FillIvSaltAndDeriveCipherKey(cipherAlgId, mk, mkLen, &head->cipherHeader, cipherParam);
    if (ret != WSEC_SUCCESS) {
        /* Destroying the stack master key */
        (void)memset_s(mk, sizeof(mk), 0, sizeof(mk));
        WSEC_LOG_E1("[SDP] FillIvSaltAndDeriveCipherKey failed %lu", ret);
        return ret;
    }

    /* Deriving a Working Key Based on the Master Key */
    ret = FillSaltHmacLenAndDeriveHmacKey(hmacAlgId, cipherVersion, mk, mkLen, &head->hmacHeader, hmacParam);
    /* Destroying the stack master key */
    (void)memset_s(mk, sizeof(mk), 0, sizeof(mk));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] FillSaltHmacLenAndDeriveHmacKey failed");
        return ret;
    }

    /* Fill other fields in the header. */
    head->cipherHeader.cipherVersion = cipherVersion;
    head->cipherHeader.hmacFlag = WSEC_TRUE;
    head->cipherHeader.domainId = domain;
    head->cipherHeader.cipherAlgId = cipherAlgId;
    head->cipherHeader.keyId = keyId;
    (void)memcpy_s(head->cipherHeader.mkHashId, (size_t)WSEC_MK_HASH_REC_LEN, mkHash, (size_t)WSEC_MK_HASH_REC_LEN);

    head->hmacHeader.hmacAlgId = hmacAlgId;
    return WSEC_SUCCESS;
}

/*
 * Search for the MK in the specified domain based on the key ID and hash value,
 * and derive the working key based on the MK.
 */
static unsigned long GetWorkKeyByIDHash(WsecUint32 domain,
    WsecUint32 keyId,
    const unsigned char *hashData,
    WsecUint32 hashLen,
    const Pbkdf2ParamConst *pbkdf2Param,
    unsigned char *key,
    WsecUint32 keyLen)
{
    unsigned char masterKey[WSEC_MK_LEN_MAX];
    unsigned long returnValue; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 mkLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    /* Verify Parameter Value */
    WSEC_ASSERT(pbkdf2Param != NULL);
    WSEC_ASSERT(pbkdf2Param->salt != NULL);
    WSEC_ASSERT(pbkdf2Param->saltLen > 0);
    WSEC_ASSERT(pbkdf2Param->iter > 0);
    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(keyLen > 0);

    /* Obtaining a Master Key */
    mkLen = sizeof(masterKey);
    returnValue = KmcGetMkByIDHash(domain, keyId, hashData, hashLen, masterKey, &mkLen);
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] GetWorkKeyByIDHash get MK failed %lu.", returnValue);
        return returnValue;
    }
    /* Deriving a Working Key Based on the Master Key */
    returnValue = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, masterKey, mkLen, pbkdf2Param, keyLen, key);

    (void)memset_s(masterKey, sizeof(masterKey), 0, sizeof(masterKey));
    if (returnValue != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] GetWorkKeyByIDHash CAC pbkdf2 derive WK failed.");
        return WSEC_ERR_PBKDF2_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Validate the input parameter to obtain the HMAC result key length. */
static unsigned long VerifyGetHmacLengthAndKeyLength(const unsigned char *hmacData,
    WsecUint32 hmacLen,
    SdpHmacHeaderEx *hmacHeader,
    WsecUint32 *keyLen, WsecUint32 *tempHmacLen)
{
    unsigned long ret;
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    WSEC_ASSERT(hmacHeader != NULL);
    /* Verify Parameter Value */
    if (hmacData == NULL || hmacLen == 0) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (hmacLen <= sizeof(SdpHmacHeaderEx)) {
        WSEC_LOG_E1("[SDP] Buffer for hmac text %u is not enough.", hmacLen);
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    if (hmacLen > (sizeof(SdpHmacHeaderEx) + SDP_PTMAC_MAX_LEN)) {
        WSEC_LOG_E1("[SDP] Buffer for hmac text %u is out of bounds.", hmacLen);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Parsing Header */
    if (memcpy_s(hmacHeader, sizeof(SdpHmacHeaderEx), hmacData, sizeof(SdpHmacHeaderEx)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }

    /* Byte order */
    CvtByteOrderForHmacHdrEx(hmacHeader, WBCNETWORK2HOST);

    /* MAC length */
    ret = SdpGetAlgPropertyEx(hmacHeader->hmacAlgId, &algType, keyLen, NULL, tempHmacLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] Get algorithm property failed.");
        return ret;
    }
    if (algType != WSEC_ALGTYPE_HMAC) {
        WSEC_LOG_E1("[SDP] alg(%u) is out of bounds.", algType);
        return WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
    }
    if (hmacLen < sizeof(SdpHmacHeaderEx) + (*tempHmacLen)) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is not enough.");
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    return WSEC_SUCCESS;
}

/* Verifying the HMAC */
static unsigned long VerifyHmacEx(const SdpCipherHeaderEx *cipherHeader,
    const unsigned char *plainText, WsecUint32 plaintextLen,
    const unsigned char *hmacData, WsecUint32 hmacLen)
{
    /* The initialization is mandatory. Otherwise, alarm 644 is reported. */
    SdpHmacHeaderEx hmacHeader = { 0, {0}, {0}, 0 };
    unsigned char key[SDP_KEY_MAX_LEN];
    unsigned char recalculateHmac[SDP_PTMAC_MAX_LEN];
    unsigned long ret;
    WsecUint32 tempHmacLen = 0;
    WsecUint32 keyLen;
    Pbkdf2ParamConst pbkdf2Param;
    pbkdf2Param.salt = hmacHeader.salt;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(hmacHeader.salt);
    pbkdf2Param.iter = 1;

    /* Verify Parameter Value */
    if (CheckCipherTextHdr(cipherHeader) != WSEC_SUCCESS) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (plainText == NULL || plainText == hmacData) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        ret = VerifyGetHmacLengthAndKeyLength(hmacData, hmacLen, &hmacHeader, &keyLen, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (cipherHeader->cipherVersion == SDP_CIPHER_TEXT_VER2) {
            keyLen = SDP_HMAC_DERIVE_KEY_LEN;
        }
        /* Obtains the working key based on the domain ID and key ID. */
        ret = GetWorkKeyByIDHash(cipherHeader->domainId, cipherHeader->keyId,
            cipherHeader->mkHashId, (WsecUint32)sizeof(cipherHeader->mkHashId), &pbkdf2Param, key, keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] Get WK by KeyID failed.");
            break;
        }

        /* HMAC calculation */
        ret = CacHmac(hmacHeader.hmacAlgId, key, keyLen, plainText, plaintextLen,
            recalculateHmac, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CAC calculate hmac failed.");
            ret = WSEC_ERR_HMAC_FAIL;
            break;
        }

        /* Compare whether they are the same. */
        if (WSEC_MEMCMP(recalculateHmac, hmacData + sizeof(SdpHmacHeaderEx), tempHmacLen) != 0) {
            WSEC_LOG_E("[SDP] HMAC failed to pass the verification.");
            ret = WSEC_ERR_HMAC_AUTH_FAIL;
            break;
        }
    } while (0);
    /* Destroys a key. */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return ret;
}

/* Obtains the header structure of the input ciphertext. */
unsigned long SdpGetCipherHeaderEx(const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    SdpCipherHeaderEx *cipherHeader)
{
    WsecUint32 headerLen = sizeof(SdpCipherHeaderEx);
    SdpCipherHeaderEx header;
    unsigned long returnValue;

    if (!(ciphertext != NULL && cipherHeader != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (ciphertextLen < headerLen) {
        WSEC_LOG_E2("[SDP] SDPV2 cipher header length should be %u, but %u.", headerLen, ciphertextLen);
        return WSEC_ERR_SDP_CIPHER_LENGTH_NOT_ENOUGH;
    }

    do {
        if (memcpy_s(&header, sizeof(SdpCipherHeaderEx), ciphertext, sizeof(SdpCipherHeaderEx)) != EOK) {
            WSEC_LOG_E4MEMCPY;
            returnValue = WSEC_ERR_MEMCPY_FAIL;
            break;
        }

        /* Byte order */
        CvtByteOrderForCipherHdrEx(&header, WBCNETWORK2HOST);

        /* check head */
        returnValue = CheckCipherTextHdr(&header);
        if (returnValue != WSEC_SUCCESS) {
            break;
        }
        if (memcpy_s(cipherHeader, sizeof(SdpCipherHeaderEx), &header, sizeof(SdpCipherHeaderEx)) != EOK) {
            WSEC_LOG_E4MEMCPY;
            returnValue = WSEC_ERR_MEMCPY_FAIL;
            break;
        }
    } while (0);

    return returnValue;
}

/* Obtains the length of the ciphertext data corresponding to the plaintext data when HMAC is not calculated. */
unsigned long SdpGetCipherDataLenEx(WsecUint32 plaintextLen, WsecUint32 *ciphertextLenOut)
{
    WsecUint32 tempLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    if (ciphertextLenOut == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    tempLen = (WsecUint32)sizeof(SdpCipherHeaderEx) + plaintextLen + SDP_SYM_MAX_BLOCK_SIZE;
    if (tempLen < (SDP_SYM_MAX_BLOCK_SIZE + (WsecUint32)sizeof(SdpCipherHeaderEx))) {
        WSEC_LOG_E1("Plaintext len %u parameter invalid.", plaintextLen);
        return WSEC_ERR_INVALID_ARG;
    }
    *ciphertextLenOut = tempLen;

    return WSEC_SUCCESS;
}

/* Obtains the length of the ciphertext data corresponding to the plaintext data when HMAC is calculated. */
unsigned long SdpGetCipherDataLenWithHmacEx(WsecUint32 plaintextLen, WsecUint32 *ciphertextLenOut)
{
    WsecUint32 tempLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    if (ciphertextLenOut == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    tempLen = (WsecUint32)sizeof(SdpCipherHeaderEx) + plaintextLen + SDP_SYM_MAX_BLOCK_SIZE +
        (WsecUint32)sizeof(SdpHmacHeaderEx) + SDP_HMAC_MAX_SIZE;
    if (tempLen < (sizeof(SdpCipherHeaderEx) + SDP_SYM_MAX_BLOCK_SIZE + sizeof(SdpHmacHeaderEx) + SDP_HMAC_MAX_SIZE)) {
        WSEC_LOG_E1("Invalid plainlen input %u.", plaintextLen);
        return WSEC_ERR_INVALID_ARG;
    }
    *ciphertextLenOut = tempLen;

    return WSEC_SUCCESS;
}

/* Check whether the current version is correct. */
static unsigned long CheckKsfVersion(void)
{
    unsigned long ret;
    WsecUint16 ksfVersion;
    /* Check whether the version is matched. */
    ret = MemGetKsfVersion(&ksfVersion);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Get ksf version failed %lu", ret);
        return ret;
    }

    /*
     * Program judgment: In the V2-compatible scenario, the scenario where V2 is upgraded to V3 is valid.
     * Only the V1-compatible scenario is invalid. That is, V2 and V3 can be used, but V1 cannot.
     * User guarantee: SDP V2 is not applicable to the scenario where V1 is upgraded to V3, but
     * there is no restriction on the program. In this case, the program cannot
     * distinguish whether V3 or V1 is upgraded to V3,
     * Users must ensure that SDP V2 interfaces cannot be used in the scenario where V1 is upgraded to V3.
     * Otherwise, decryption cannot be performed when the version is downgraded to V1.
     */
    if (IsKsfV3(ksfVersion) != WSEC_TRUE && IsKsfV2(ksfVersion) != WSEC_TRUE) {
        WSEC_LOG_E1("ksf version invalid(can not v1) %hu", ksfVersion);
        return WSEC_ERR_KMC_KSF_VERSION_INVALID;
    }
    return WSEC_SUCCESS;
}

/* Specify the domain and encryption algorithm ID, encrypt the specified plaintext, and obtain the ciphertext. */
unsigned long SdpEncryptEx(WsecUint32 domain, WsecUint32 algId,
    const unsigned char *plainText, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    SdpCipherHeaderEx *cipherHeader = (SdpCipherHeaderEx *)(WsecVoid *)ciphertext;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    CipherParam cipherParam = { NULL, SDP_KEY_MAX_LEN, 0 };
    WsecUint32 tempCiphertextLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 ciphertextMaxLen = 0;
    cipherParam.key = key;

    /* Verify Parameter Value */
    if (!(plainText != NULL && ciphertextLen != NULL && ciphertext != NULL && (plainText != ciphertext))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CheckKsfVersion();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    WSEC_UNREFER(SdpGetCipherDataLenEx(plaintextLen, &ciphertextMaxLen));
    if (*ciphertextLen < ciphertextMaxLen) {
        WSEC_LOG_E1("[SDP] Ciphertext buffer is not enough, ciphertextLen %u.", *ciphertextLen);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    if (memset_s(ciphertext, (size_t)ciphertextMaxLen, 0, sizeof(SdpCipherHeaderEx)) != EOK) {
        WSEC_LOG_E("[SDP] Buffer memset failed.");
        return WSEC_ERR_MEMSET_FAIL;
    }

    do {
        /* Fill in the ciphertext header to obtain the key and IV. */
        ret = FillCipherTextHeaderEx(algId, domain, cipherHeader, &cipherParam);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] Fill cipher text header failed.");
            break;
        }
        /* Encrypting Input Data */
        tempCiphertextLen = ciphertextMaxLen - (WsecUint32)sizeof(SdpCipherHeaderEx);   /* soter 554 */
        ret = CacEncrypt(cipherHeader->cipherAlgId,
            cipherParam.key, cipherParam.keyLen,
            cipherHeader->iv, cipherParam.ivLen,
            plainText, plaintextLen,
            (ciphertext + sizeof(SdpCipherHeaderEx)), &tempCiphertextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CAC encrypt failed.");
            break;
        }

        /* Actual length of the ciphertext */
        cipherHeader->ciphertextLen = tempCiphertextLen;

        /* Calculate the length of the ciphertext (including the header). */
        tempCiphertextLen += (WsecUint32)sizeof(SdpCipherHeaderEx);

        /* Byte order */
        CvtByteOrderForCipherHdrEx(cipherHeader, WBCHOST2NETWORK);

        /* Length of the output ciphertext */
        *ciphertextLen = tempCiphertextLen;
    } while (0);
    /* Destroys the stack working key. */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return ret;
}

/* Encrypting Data */
static unsigned long EncryptData(const CipherParam *cipherParam,
    const unsigned char *plaintext,
    WsecUint32 plaintextLen,
    SdpCipherHeaderEx *cipherHeader,
    unsigned char *cipherBuff,
    WsecUint32 *cipherLen)
{
    unsigned long ret;
    WsecUint32 encDataLen;
    WsecUint32 remainLen;
    unsigned char *encData = NULL;
    WSEC_ASSERT(cipherHeader != NULL);
    WSEC_ASSERT(cipherLen != NULL);
    remainLen = *cipherLen;

    if ((WsecUint32)sizeof(SdpCipherHeaderEx) > remainLen) {
        WSEC_LOG_E1("[SDP] Buffer for cipher text is not enough %u.", remainLen);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    encDataLen = remainLen - (WsecUint32)sizeof(SdpCipherHeaderEx);
    encData = cipherBuff + sizeof(SdpCipherHeaderEx);

    /*
     * Encryption: The encryption algorithm checks whether the ciphertext buffer is sufficient.
     * If the buffer is insufficient, a failure message is returned.
     * If the buffer is sufficient, the size of the returned ciphertext buffer cannot be greater than the input size.
     */
    ret = CacEncrypt(cipherHeader->cipherAlgId,
        cipherParam->key, cipherParam->keyLen,
        cipherHeader->iv, cipherParam->ivLen,
        plaintext, plaintextLen,
        encData, &encDataLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] CAC encrypt failed %lu.", ret);
        return ret;
    }

    /* Ciphertext length of the padding header */
    cipherHeader->ciphertextLen = encDataLen;

    *cipherLen = (WsecUint32)sizeof(SdpCipherHeaderEx) + encDataLen;
    /* Byte order */
    CvtByteOrderForCipherHdrEx(cipherHeader, WBCHOST2NETWORK);
    ret = (memcpy_s(cipherBuff, sizeof(SdpCipherHeaderEx), cipherHeader, sizeof(SdpCipherHeaderEx)) == EOK)
        ? WSEC_SUCCESS : WSEC_ERR_MEMCPY_FAIL;
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] CAC encrypt failed while copy buff data: %lu.", ret);
        return ret;
    }
    return WSEC_SUCCESS;
}

/* Data HMAC */
static unsigned long HmacData(const HmacParam *hmacParam,
    const unsigned char *plain,
    WsecUint32 plainLen,
    unsigned char *hmacBuff,
    WsecUint32 *hmacLen,
    SdpHmacHeaderEx *hmacHeader)
{
    unsigned long ret;
    WsecUint32 hmacDataLen;
    WsecUint32 remainLen;
    unsigned char *hmacData = NULL;
    WSEC_ASSERT(hmacHeader != NULL);
    WSEC_ASSERT(hmacLen != NULL);
    remainLen = *hmacLen;
    if ((WsecUint32)sizeof(SdpHmacHeaderEx) > remainLen) {
        WSEC_LOG_E1("[SDP] Buffer for hmac is not enough %u.", remainLen);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    hmacDataLen = remainLen - (WsecUint32)sizeof(SdpHmacHeaderEx);
    hmacData = hmacBuff + (WsecUint32)sizeof(SdpHmacHeaderEx);
    ret = CacHmac(hmacHeader->hmacAlgId,
        hmacParam->hmacKey, hmacParam->hmacKeyLen,
        plain, plainLen,
        hmacData, &hmacDataLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] Cac Hmac failed %lu.", ret);
        return ret;
    }

    if (hmacHeader->hmacLen != hmacDataLen) {
        WSEC_LOG_E2("[SDP] Cac Hmac length invalid %u %u.", hmacHeader->hmacLen, hmacDataLen);
        return WSEC_ERR_HMAC_FAIL;
    }

    *hmacLen = (WsecUint32)sizeof(SdpHmacHeaderEx) + hmacDataLen;
    CvtByteOrderForHmacHdrEx(hmacHeader, WBCHOST2NETWORK);
    ret = (memcpy_s(hmacBuff, sizeof(SdpHmacHeaderEx), hmacHeader, sizeof(SdpHmacHeaderEx)) == EOK)
        ? WSEC_SUCCESS : WSEC_ERR_MEMCPY_FAIL;
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP copy buff error: %lu.", ret);
        return ret;
    }
    return WSEC_SUCCESS;
}

static unsigned long CheckCipherTextLen(const WsecBuffConst *plainBuff, WsecUint32 *tempCipherLen,
    WsecUint32 ciphertextLen)
{
    unsigned long ret;
    do {
        /* DTS2017042407037: The return value of SdpGetCipherDataLenWithHmacEx is not checked. */
        ret = SdpGetCipherDataLenWithHmacEx(plainBuff->len, tempCipherLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] SdpGetCipherDataLenWithHmacEx fail. return = %lu", ret);
            break;
        }
        /* Construct the header and obtain the working key. */
        if (ciphertextLen < *tempCipherLen) {
            WSEC_LOG_E1("[SDP] Ciphertext buffer is not enough, ciphertextLen %u.", ciphertextLen);
            ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }
    } while (0);

    return ret;
}

/* Encrypt and calculate HMAC */
static unsigned long EncryptWithHmac(WsecUint32 domain, WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    WsecUint32 cipherVersion, const WsecBuffConst *plainBuff, unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    unsigned long ret;
    SdpCipherAndHmacHeaders head;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    unsigned char hmacKey[SDP_KEY_MAX_LEN] = {0};
    CipherParam cipherParam = { NULL, SDP_KEY_MAX_LEN, 0 };
    HmacParam hmacParam = { NULL, SDP_KEY_MAX_LEN };
    WsecUint32 temp = 0;
    WsecUint32 tempLen;
    cipherParam.key = key;
    hmacParam.hmacKey = hmacKey;

    /* Verify Parameter Value */
    WSEC_ASSERT(plainBuff != NULL);
    if (!(plainBuff->buff != NULL && ciphertextLen != NULL && ciphertext != NULL && (plainBuff->buff != ciphertext))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        ret = CheckCipherTextLen(plainBuff, &temp, *ciphertextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("CheckCipherTextLen failed, ret=%lu", ret);
            break;
        }
        (void)memset_s(&head, sizeof(SdpCipherAndHmacHeaders), 0, sizeof(SdpCipherAndHmacHeaders));
        ret = FillHeaderWithHmacEx(cipherAlgId, hmacAlgId, cipherVersion, domain, &head, &cipherParam, &hmacParam);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] Fill cipher text header failed %lu", ret);
            break;
        }
        tempLen = temp;
        /* Encryption */
        ret = EncryptData(&cipherParam, (const unsigned char *)plainBuff->buff, plainBuff->len, &head.cipherHeader, ciphertext, &tempLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        if (temp < tempLen) {
            WSEC_LOG_E1("[SDP] Ciphertext buffer is not enough, temp cipherLen %u.", tempLen);
            ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }
        temp -= tempLen;    /* soter 554 */
        ret = HmacData(&hmacParam, (const unsigned char *)plainBuff->buff, plainBuff->len, ciphertext + tempLen, &temp, &head.hmacHeader);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /* Output Parameters */
        *ciphertextLen = tempLen + temp;
    } while (0);

    /* Destroys the stack master key and stack working key. */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    (void)memset_s(hmacKey, sizeof(hmacKey), 0, sizeof(hmacKey));

    return ret;
}

/*
 * Specify the domain and encryption/MAC algorithm ID,
 * encrypt the specified plaintext, calculate the MAC, and obtain the ciphertext.
 */
unsigned long SdpEncryptWithHmacEx(WsecUint32 domain,
    WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    const unsigned char *plainText, WsecUint32 plaintextLen, unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    unsigned long ret;
    WsecBuffConst plainBuff;
    WSEC_BUFF_ASSIGN(plainBuff, plainText, plaintextLen);

    ret = CheckKsfVersion();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    return EncryptWithHmac(domain, cipherAlgId, hmacAlgId, SDP_CIPHER_TEXT_VER2,
        &plainBuff,
        ciphertext, ciphertextLen);
}

/*
 * Specify the domain and encryption/MAC algorithm ID, encrypt the specified plaintext,
 * calculate the MAC, and obtain the ciphertext.
 */
unsigned long SdpEncryptWithHmacV3(WsecUint32 domain,
    WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    const unsigned char *plainText, WsecUint32 plaintextLen, unsigned char *ciphertext, WsecUint32 *ciphertextLen)
{
    unsigned long ret;
    WsecBuffConst plainBuff;
    WSEC_BUFF_ASSIGN(plainBuff, plainText, plaintextLen);

    ret = CheckKsfVersion();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    return EncryptWithHmac(domain, cipherAlgId, hmacAlgId, SDP_CIPHER_TEXT_VER3,
        &plainBuff,
        ciphertext, ciphertextLen);
}

/* Parsing cipher header */
static unsigned long ParseCipherHeader(const unsigned char *ciphertext,
    WsecUint32 ciphertextLen, SdpCipherHeaderEx *cipherHeader)
{
    unsigned long ret;
    /* Parsing Header */
    if (ciphertextLen < sizeof(SdpCipherHeaderEx)) {
        WSEC_LOG_E1("[SDP] Ciphertext buffer is not enough, ciphertextLen %u.", ciphertextLen);
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    do {
        (void)memcpy_s(cipherHeader, sizeof(SdpCipherHeaderEx), ciphertext, sizeof(SdpCipherHeaderEx));
        /* Byte order */
        CvtByteOrderForCipherHdrEx(cipherHeader, WBCNETWORK2HOST);
        /* check head */
        ret = CheckCipherTextHdr(cipherHeader);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] Ciphertext header check failed %lu.", ret);
            break;
        }
        /* If the ciphertext is different from the entered domain, the key can be matched based on the hash. */
        if (ciphertextLen < (sizeof(SdpCipherHeaderEx) + cipherHeader->ciphertextLen)) {
            WSEC_LOG_E1("[SDP] Ciphertext buffer is not enough, ciphertextLen %u.", ciphertextLen);
            ret = WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
            break;
        }
    } while (0);
    return ret;
}

/* Decrypting Data */
static unsigned long DecryptData(const unsigned char *ciphertext,
    WsecUint32 ciphertextLen,
    unsigned char *plainText,
    WsecUint32 *plaintextLen, SdpCipherHeaderEx *cipherHeader)
{
    unsigned long ret;
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    WsecUint32 keyLen = 0;
    WsecUint32 ivLen = 0;
    Pbkdf2ParamConst pbkdf2Param;
    pbkdf2Param.salt = cipherHeader->salt;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(cipherHeader->salt);
    pbkdf2Param.iter = 1;

    do {
        ret = ParseCipherHeader(ciphertext, ciphertextLen, cipherHeader);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] parse cipher header failed: %lu", ret);
            break;
        }
        /* Calculate the key and IV length. */
        ret = SdpGetAlgPropertyEx(cipherHeader->cipherAlgId, &algType, &keyLen, &ivLen, NULL);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] Get algorithm property failed %lu.", ret);
            break;
        }
        if (algType != WSEC_ALGTYPE_SYM) {
            WSEC_LOG_E1("[SDP] Alg (%u) is out of bounds.", algType);
            ret = WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
            break;
        }

        /* Obtains the working key based on the domain ID and key ID. */
        ret = GetWorkKeyByIDHash(cipherHeader->domainId, cipherHeader->keyId,
            cipherHeader->mkHashId, (WsecUint32)sizeof(cipherHeader->mkHashId), &pbkdf2Param, key, keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] DecryptData get WK failed %lu.", ret);
            break;
        }

        /* Decrypting Input Data */
        ret = CacDecrypt(cipherHeader->cipherAlgId, key, keyLen, cipherHeader->iv, ivLen,
            (ciphertext + sizeof(SdpCipherHeaderEx)), cipherHeader->ciphertextLen, plainText, plaintextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("[SDP] CAC decrypt failed %lu.", ret);
            break;
        }
    } while (0);
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return ret;
}

/* Decrypts the specified ciphertext data. The data with or without the HMAC can be decrypted. */
unsigned long SdpDecryptEx(WsecUint32 domain,
    const unsigned char *ciphertext,
    WsecUint32 ciphertextLen,
    unsigned char *plainText,
    WsecUint32 *plaintextLen)
{
    SdpCipherHeaderEx cipherHeader = {0, 0, 0, 0, 0, {0}, {0}, {0}, {0}, 0};
    unsigned long ret;
    /* Verify Parameter Value */
    if (ciphertext == NULL || plainText == NULL || plaintextLen == NULL ||
        ciphertextLen == 0 || plainText == ciphertext) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CheckKsfVersion();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    do {
        ret = DecryptData(ciphertext, ciphertextLen, plainText, plaintextLen, &cipherHeader);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* If the ciphertext is different from the entered domain, the key can be matched based on the hash. */
        WSEC_UNREFER(domain);
        /* Verification */
        if (cipherHeader.hmacFlag == WSEC_TRUE) {
            ret = VerifyHmacEx(&cipherHeader, plainText, *plaintextLen,
                (ciphertext + sizeof(SdpCipherHeaderEx) + cipherHeader.ciphertextLen),
                ((ciphertextLen - (WsecUint32)sizeof(SdpCipherHeaderEx)) - cipherHeader.ciphertextLen)); /* soter 554 */
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("[SDP] Cipher text hmac verify failed %lu", ret);
                break;
            }
        }
    } while (0);

    return ret;
}

/* Check the ciphertext version and obtain the ciphertext header of the corresponding version. */
static unsigned long SdpGetCipherHeader(const unsigned char *ciphertext, WsecUint32 ciphertcextLen,
    SdpCipherHeaderEx *cipherHeaderEx, SdpCipherHeader *cipherHeader, WsecUint32 *sdpVersion)
{
    unsigned long returnValue = WSEC_SUCCESS;
    WsecUint32 version;
    if ((ciphertext == NULL) || (cipherHeaderEx == NULL) || (cipherHeader == NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    /* avoid out-of-bounds reading ciphertext buffer when memcpy version */
    if (ciphertcextLen < sizeof(WsecUint32)) {
        WSEC_LOG_E("[SDP] Invalid parameter, buffer for ciphertext is not enough.");
        return WSEC_ERR_SDP_CIPHER_LENGTH_NOT_ENOUGH;
    }

    if (memcpy_s(&version, sizeof(WsecUint32), ciphertext, sizeof(WsecUint32)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }

    version = WSEC_N2H_L(version);
    if (version == SDP_CIPHER_TEXT_VER1) {
        returnValue = SdpGetCipherHeaderV1(ciphertext, ciphertcextLen, cipherHeader);
        if (returnValue != WSEC_SUCCESS) {
            return returnValue;
        }
    } else if (version == SDP_CIPHER_TEXT_VER2 || version == SDP_CIPHER_TEXT_VER3) {
        returnValue = SdpGetCipherHeaderEx(ciphertext, ciphertcextLen, cipherHeaderEx);
        if (returnValue != WSEC_SUCCESS) {
            return returnValue;
        }
    } else {
        WSEC_LOG_E("[SDP] Ciphertext version error.");
        return WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
    }
    *sdpVersion = version;

    return returnValue;
}

/* Obtain the MkInfo of the encryption key based on the ciphertext information. */
unsigned long SdpGetMkDetailByCipher(const unsigned char *cipherData, WsecUint32 cipherDataLen, KmcMkInfo *mkInfo)
{
    unsigned long errorCode;
    SdpCipherHeaderEx cipherHeaderEx = {0, 0, 0, 0, 0, {0}, {0}, {0}, {0}, 0};
    SdpCipherHeader cipherHeader = {0, 0, 0, 0, 0, 0, {0}, {0}, 0};
    WsecUint32 sdpVersion;
    WsecUint32 domainId;
    WsecUint32 keyId;
    unsigned char keyPlaintextBuff[SDP_KEY_MAX_LEN] = {0};
    WsecUint32 keyBuffLen = SDP_KEY_MAX_LEN;

    if (cipherData == NULL || mkInfo == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    errorCode = SdpGetCipherHeader(cipherData, cipherDataLen, &cipherHeaderEx, &cipherHeader, &sdpVersion);
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E3("[SDP] %s - %d, Get the cipher header failed %lu.", __FUNCTION__, __LINE__, errorCode);
        return errorCode;
    }

    if (sdpVersion == SDP_CIPHER_TEXT_VER1) {
        domainId = cipherHeader.domain;
        keyId = cipherHeader.keyId;
    } else {
        domainId = cipherHeaderEx.domainId;
        keyId = cipherHeaderEx.keyId;
    }

    errorCode = KmcGetMkDetail(domainId, keyId, mkInfo, keyPlaintextBuff, &keyBuffLen);
    (void)memset_s(keyPlaintextBuff, sizeof(keyPlaintextBuff), 0, sizeof(keyPlaintextBuff));
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E3("[SDP] %s - %d, Get the KmcMkInfo failed %lu.", __FUNCTION__, __LINE__, errorCode);
    }
    return errorCode;
}
