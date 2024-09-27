/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Implementation of the hmac data protection module in SDP V1
 * Author: xuhuiyue
 * Create: 2020-11-05
 */

#include "sdpv1_itf.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv2_itf.h"
#include "sdp_utils.h"
#include "sdpv3_alg.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_order.h"
#include "wsecv2_file.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

#define SDP_HMAC_CTX_LEN   sizeof(SdpHmacCtx)

/*
 * Search for the MK in the specified domain based on the encryption algorithm ID,
 * derive the working key based on the MK, and fill the HMAC header.
 */
static unsigned long FillHmacTextHeader(WsecUint32 domain, WsecUint32 algId, SdpHmacHeader * const hmacHeader,
    unsigned char *key, WsecUint32 *keyLen)
{
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 keyLength = 0;
    WsecUint32 keyId = 0;
    Pbkdf2Param pbkdf2Param;
    /* KMC_KEY_TYPE_INTEGRITY must be retained in KMC V1 upgrade scenarios. */
    KmcActiveKeyParam keyParam = {
        DEFAULT_SDP_DOMAIN_ID, {
            SDP_V1_SUPPORT_KEY_TYPE_COUNT, {
                KMC_KEY_TYPE_ENCRPT_INTEGRITY, KMC_KEY_TYPE_INTEGRITY, KMC_KEY_TYPE_INTEGRITY
            }
        }
    };

    /* Verify parameters. */
    if (hmacHeader == NULL || keyLen == NULL || key == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Obtain the key length based on the algorithm. */
    ret = SdpGetAlgPropertyEx(algId, &algType, &keyLength, NULL, NULL);
    if (ret != WSEC_SUCCESS || keyLength > *keyLen) {
        WSEC_LOG_E("[SDP] Get algorithm property failed.");
        return ret;
    }
    if (algType != WSEC_ALGTYPE_HMAC) {
        WSEC_LOG_E1("[SDP] alg(%u) is out of bounds.", algType);
        return WSEC_ERR_SDP_CONFIG_INCONSISTENT_WITH_USE;
    }

    /* Obtaining the Working Key */
    pbkdf2Param.salt = hmacHeader->salt;
    pbkdf2Param.saltLen = SDP_SALT_LEN;
    pbkdf2Param.iter = 0;
    keyParam.domainId = domain;
    ret = GetWorkKey(&keyParam, &keyId, &pbkdf2Param, NULL, 0, key, keyLength);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] Get WK failed.");
        return ret;
    }

    /* Fill in other fields. */
    hmacHeader->version = SDP_HMAC_VER;
    hmacHeader->domain = domain;
    hmacHeader->algId = algId;
    hmacHeader->iter = (WsecUint32)pbkdf2Param.iter;
    hmacHeader->keyId = keyId;

    /* Output Parameters */
    *keyLen = keyLength;

    return WSEC_SUCCESS;
}

/* Length of the encrypted HMAC, including the HMAC header length. */
unsigned long SdpGetHmacLen(WsecUint32 *hmacLen)
{
    if (hmacLen == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    *hmacLen = SDP_HMAC_HEAD_LEN + SDP_HMAC_MAX_SIZE;

    return WSEC_SUCCESS;
}

/* Calculates the HMAC of the specified data using the key in the specified domain and the specified algorithm ID. */
unsigned long SdpHmac(WsecUint32 domain, WsecUint32 algId, const unsigned char *plainText, WsecUint32 plaintextLen,
    unsigned char *hmacData, WsecUint32 *hmacLen)
{
    SdpHmacHeader *hmacHeader = NULL;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 keyLen = sizeof(key);
    WsecUint32 tempHmacLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 hmacMaxLen = 0;

    /* Verify Parameter Value */
    if (plainText == NULL || hmacData == NULL || hmacLen == NULL || plainText == hmacData) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CheckKsfV1();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    ret = SdpGetHmacLen(&hmacMaxLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] The hmac len achievement failed.");
        return ret;
    }

    /* Fill in the header to obtain the working key. */
    if (*hmacLen < hmacMaxLen) {
        WSEC_LOG_E("[SDP] Buffer for cipher text is not enough.");
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    if (memset_s(hmacData, (size_t)hmacMaxLen, 0, (size_t)SDP_HMAC_HEAD_LEN) != EOK) {
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    hmacHeader = (SdpHmacHeader *)(WsecVoid *)hmacData;
    do {
        ret = FillHmacTextHeader(domain, algId, hmacHeader, key, &keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] Fill HMAC text header failed.");
            break;
        }

        tempHmacLen = hmacMaxLen - SDP_HMAC_HEAD_LEN;   /* soter 554 */

        /* Calculates the HMAC. */
        ret = CacHmac(hmacHeader->algId, key, keyLen,
            plainText, plaintextLen,
            hmacData + SDP_HMAC_HEAD_LEN, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CAC calculate hmac failed.");
            break;
        }

        /* Calculate the HMAC length (including the header length). */
        tempHmacLen += SDP_HMAC_HEAD_LEN;

        /* Byte order */
        SdpCvtHmacTextHeaderByteOrder(hmacHeader, WBCHOST2NETWORK);

        /* Output Parameters */
        *hmacLen = tempHmacLen;
    } while (0);
    /* Destroying a Key */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return ret;
}

static unsigned long CheckHmacHeader(WsecUint32 algId, WsecUint32 hmacLen, WsecUint32 *keyLen, WsecUint32 *ivLen,
    WsecUint32 *tempHmacLen)
{
    unsigned long ret;
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;

    /* Obtains the key length and HMAC length. */
    ret = SdpGetAlgPropertyEx(algId, &algType, keyLen, ivLen, tempHmacLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] Get algorithm property failed.");
        return ret;
    }
    if (algType != WSEC_ALGTYPE_HMAC) {
        WSEC_LOG_E1("[SDP] alg(%u) is out of bounds.", algType);
        ret = WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
        return ret;
    }
    if (hmacLen < SDP_HMAC_HEAD_LEN + *tempHmacLen) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is not enough.");
        ret = WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    return ret;
}

/* Preparing for HMAC Verification */
static unsigned long SdpVerifyHmacPrepare(WsecUint32 domain, const unsigned char *plainText,
    const unsigned char *hmacData, WsecUint32 hmacLen, SdpHmacHeader *hmacHeader)
{
    unsigned long ret;
    WSEC_ASSERT(hmacHeader != NULL);
    /* Verify Parameter Value */
    if (hmacData == NULL || plainText == NULL || hmacLen == 0 || plainText == hmacData) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (hmacLen <= SDP_HMAC_HEAD_LEN || hmacLen <= sizeof(SdpHmacHeader)) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is not enough.");
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    if (hmacLen > SDP_HMAC_HEAD_LEN + SDP_PTMAC_MAX_LEN) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is out of bounds.");
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CheckKsfV1();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* Parsing Header */
    if (memcpy_s(hmacHeader, sizeof(SdpHmacHeader), hmacData, sizeof(SdpHmacHeader)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    /* Byte order */
    SdpCvtHmacTextHeaderByteOrder(hmacHeader, WBCNETWORK2HOST);
    if (domain != hmacHeader->domain) {
        WSEC_LOG_E1("[SDP] Cipher text are marked with an unexpected domain %u.", hmacHeader->domain);
        return WSEC_ERR_SDP_DOMAIN_UNEXPECTED;
    }
    return WSEC_SUCCESS;
}

/* Check whether the HMAC result is correct. */
unsigned long SdpVerifyHmac(WsecUint32 domain, const unsigned char *plainText, WsecUint32 plaintextLen,
    const unsigned char *hmacData, WsecUint32 hmacLen)
{
    /* The initialization is mandatory. Otherwise, alarm 644 is reported. */
    SdpHmacHeader hmacHeader = { 0, 0, 0, 0, 0, {0} };
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    unsigned char recalculateHmac[SDP_PTMAC_MAX_LEN];
    unsigned long ret;
    WsecUint32 tempHmacLen = 0;
    WsecUint32 keyLen = 0;

    /* Verify Parameter Value */
    ret = SdpVerifyHmacPrepare(domain, plainText, hmacData, hmacLen, &hmacHeader);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    do {
        ret = CheckHmacHeader(hmacHeader.algId, hmacLen, &keyLen, NULL, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CheckHmacHeader failed.");
            break;
        }

        /* Obtains the working key based on the domain ID and key ID. */
        ret = GetWorkKeyByID(hmacHeader.domain, hmacHeader.keyId, hmacHeader.iter,
            hmacHeader.salt, SDP_SALT_LEN, key, keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] Get WK by KeyID failed.");
            break;
        }

        /* Calculate HMAC */
        ret = CacHmac(hmacHeader.algId, key, keyLen, plainText, plaintextLen,
            recalculateHmac, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CAC calculate hmac failed.");
            ret = WSEC_ERR_HMAC_FAIL;
            break;
        }

        /* Check whether the values are the same. */
        if (WSEC_MEMCMP(recalculateHmac, hmacData + SDP_HMAC_HEAD_LEN, tempHmacLen) != 0) {
            WSEC_LOG_E("[SDP] HMAC failed to pass the verification.");
            ret = WSEC_ERR_HMAC_AUTH_FAIL;
            break;
        }
    } while (0);
    /* Destroying a Stack Key */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));

    return ret;
}

/* Specify the domain and algorithm ID, and obtain SdpHmacAlgAttributes for SdpHmacInit or SdpFileHmac. */
unsigned long SdpGetHmacAlgAttr(WsecUint32 domain, WsecUint32 algId, SdpHmacAlgAttributes *hmacAlgAttributes)
{
    SdpHmacHeader *hmacHeader = NULL;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    WsecUint32 keyLen = sizeof(key);
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    if (hmacAlgAttributes == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    hmacHeader = &(hmacAlgAttributes->hmacHeader);

    ret = FillHmacTextHeader(domain, algId, hmacHeader, key, &keyLen);
    /* Destroying a Stack Key */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));

    SdpCvtHmacTextHeaderByteOrder(hmacHeader, WBCHOST2NETWORK);
    (void)keyLen;
    return ret;
}

/* Preparations for Starting the Stream Data HMAC */
static unsigned long SdpHmacInitPrepare(WsecUint32 domain, const SdpHmacAlgAttributes *hmacAlgAttributes,
    WsecHandle *ctx, SdpHmacCtx **sdpCtx, SdpHmacHeader **header)
{
    unsigned long ret = WSEC_SUCCESS;
    SdpHmacCtx *temp = NULL;
    SdpHmacHeader *tempHeader = NULL;

    if (hmacAlgAttributes == NULL || ctx == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    temp = (SdpHmacCtx *)WSEC_MALLOC(SDP_HMAC_CTX_LEN);
    if (temp == NULL) {
        WSEC_LOG_E4MALLOC(SDP_HMAC_CTX_LEN);
        return WSEC_ERR_MALLOC_FAIL;
    }
    tempHeader = &temp->hmacHeader;

    do {
        if (memcpy_s(tempHeader, sizeof(SdpHmacHeader), hmacAlgAttributes, sizeof(SdpHmacHeader)) != EOK) {
            ret = WSEC_ERR_MEMCPY_FAIL;
            break;
        }

        SdpCvtHmacTextHeaderByteOrder(tempHeader, WBCNETWORK2HOST);

        /* Parameter check */
        if (tempHeader->version != SDP_HMAC_VER) {
            WSEC_LOG_E2("[SDP] HMAC version incompatible expected %u actually %u.", SDP_HMAC_VER, tempHeader->version);
            ret = WSEC_ERR_SDP_VERSION_INCOMPATIBLE;
            break;
        }
        if (domain != tempHeader->domain) {
            WSEC_LOG_E1("[SDP] Cipher text invalid with unexpected domain %u.", tempHeader->domain);
            ret = WSEC_ERR_SDP_DOMAIN_UNEXPECTED;
            break;
        }
        if (tempHeader->iter != 1) {
            WSEC_LOG_E("[SDP] Iterator count is out of bounds.");
            ret = WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
            break;
        }
        if (CacAlgIdToType(tempHeader->algId) == WSEC_ALGTYPE_UNKNOWN) {
            WSEC_LOG_E("[SDP] CAC Get algorithm types failed.");
            ret = WSEC_ERR_SDP_ALG_NOT_SUPPORTED;
            break;
        }
    } while (0);
    if (ret != WSEC_SUCCESS) {
        WSEC_FREE(temp);
        tempHeader = NULL;
    }
    *ctx = temp;
    *sdpCtx = temp;
    *header = tempHeader;
    return ret;
}

/* Specify a domain, specify SdpHmacAlgAttributes, and obtain ctx for SdpHmacUpdate and SdpHmacFinal. */
unsigned long SdpHmacInit(WsecUint32 domain, const SdpHmacAlgAttributes *hmacAlgAttributes, WsecHandle *ctx)
{
    SdpHmacHeader *header = NULL;
    SdpHmacCtx *sdpCtx = NULL;
    WsecUint32 algType = WSEC_ALGTYPE_UNKNOWN;
    unsigned char key[SDP_KEY_MAX_LEN] = {0};
    WsecUint32 tempHmacLen = 0;
    WsecUint32 keyLen = 0;
    unsigned long ret;

    ret = CheckKsfV1();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    ret = SdpHmacInitPrepare(domain, hmacAlgAttributes, ctx, &sdpCtx, &header);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    do {
        /* Constructing an HMAC key */
        ret = SdpGetAlgPropertyEx(header->algId, &algType, &keyLen, NULL, &tempHmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("SdpGetAlgPropertyEx() = %lu.", ret);
            break;
        }
        if (algType != WSEC_ALGTYPE_HMAC) {
            WSEC_LOG_E1("alg(%u) cannot support HMAC.", algType);
            ret = WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
            break;
        }

        /* Obtains the working key based on the domain ID and key ID. */
        ret = GetWorkKeyByID(header->domain, header->keyId, header->iter, header->salt, SDP_SALT_LEN,
            key, keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("GetWorkKeyByID() = %lu", ret);
            break;
        }

        /* HMAC init */
        ret = CacHmacInit(&(sdpCtx->cacCtx), header->algId, key, keyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("CacHmacInit() = %lu", ret);
            break;
        }
    } while (0);

    *ctx = sdpCtx;
    if (ret != WSEC_SUCCESS) {
        SdpFreeHmacCtx(ctx);
    }

    /* Destroying a Stack Key */
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return ret;
}

/* Performs HMAC on data and can be invoked for multiple times. */
unsigned long SdpHmacUpdate(const WsecHandle *ctx, const unsigned char *plainText, WsecUint32 plaintextLen)
{
    /* Verify Parameter Value */
    if (ctx == NULL || (*ctx) == NULL || plainText == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* HMAC update */
    return CacHmacUpdate(((SdpHmacCtx *)(*ctx))->cacCtx, plainText, plaintextLen);
}

/* Obtains the HMAC of specified data in SdpHmacUpdate. */
unsigned long SdpHmacFinal(WsecHandle *ctx, unsigned char *hmacData, WsecUint32 *hmacLen)
{
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    /* Verify Parameter Value */
    if (ctx == NULL || (*ctx) == NULL || hmacData == NULL || hmacLen == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CacHmacFinal(&(((SdpHmacCtx *)(*ctx))->cacCtx), hmacData, hmacLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] SDP HMAC calculation final failed %lu.", ret);
    }
    WSEC_FREE(*ctx);
    *ctx = NULL;
    return ret;
}

static unsigned long DoSdpFileHmac(WsecUint32 domain, long fileLen, const SdpHmacAlgAttributes *hmacAlgAttributes,
    unsigned char *hmacData, WsecUint32 *hmacLen, unsigned char *readBuff, WsecHandle readStream)
{
    unsigned long ret;
    WsecHandle sdpCtx = {0};
    long readLen;
    unsigned long temp;
    long curLen = 0;
    WsecBool hasRead = WSEC_FALSE;

    do {
        ret = SdpHmacInit(domain, hmacAlgAttributes, &sdpCtx);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("SdpHmacInit() = %lu", ret);
            break;
        }
        while (curLen < fileLen) {
            readLen = (curLen + WSEC_FILE_IO_SIZE_MAX <= fileLen) ? WSEC_FILE_IO_SIZE_MAX : (fileLen - curLen);
            hasRead = WSEC_FREAD(readBuff, readLen, readStream);
            if (hasRead == WSEC_FALSE) {
                WSEC_LOG_E1("Read copy from file fail, %d.", WSEC_FERRNO(readStream));
                ret = WSEC_ERR_READ_FILE_FAIL;
                break;
            }

            ret = SdpHmacUpdate(&sdpCtx, readBuff, (WsecUint32)readLen);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("SdpHmacUpdate() = %lu", ret);
                break;
            }
            curLen += readLen;
        }
        temp = SdpHmacFinal(&sdpCtx, hmacData, hmacLen);
        ret = (ret == WSEC_SUCCESS) ? temp : ret;
    } while (0);

    return ret;
}

/* Specify the domain, SdpHmacAlgAttributes, and file path to obtain the HMAC result of the file data. */
unsigned long SdpFileHmac(WsecUint32 domain,
    const char *file,
    const SdpHmacAlgAttributes *hmacAlgAttributes,
    WsecVoid *hmacData, WsecUint32 *hmacLen)
{
    unsigned long ret;
    long fileLen = 0;
    unsigned char *readBuff = NULL;
    WsecHandle readStream = NULL;

    if (file == NULL || hmacAlgAttributes == NULL || hmacData == NULL || hmacLen == NULL) {
        WSEC_LOG_E("SdpFileHmac check parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (WsecGetFileLen(file, &fileLen) == WSEC_FALSE) {
        WSEC_LOG_E("Cannot access the file");
        return WSEC_ERR_OPEN_FILE_FAIL;
    }

    readStream = WSEC_FOPEN(file, KMC_FILE_READ_BINARY);
    if (readStream == NULL) {
        return WSEC_ERR_OPEN_FILE_FAIL;
    }
    if ((readBuff = (unsigned char *)WSEC_MALLOC(WSEC_FILE_IO_SIZE_MAX)) == NULL) {
        WSEC_FCLOSE(readStream);
        return WSEC_ERR_MALLOC_FAIL;
    }

    ret = DoSdpFileHmac(domain, fileLen, hmacAlgAttributes, (unsigned char *)hmacData, hmacLen, readBuff, readStream);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("SdpFileHmac-DoSdpFileHmac failed, ret=%lu", ret);
    }

    WSEC_FCLOSE(readStream);
    WSEC_CLEAR_FREE(readBuff, WSEC_FILE_IO_SIZE_MAX);
    return ret;
}

/* Specify the domain, SdpHmacAlgAttributes, file path, file data HMAC, and HMAC authentication. */
unsigned long SdpVerifyFileHmac(WsecUint32 domain,
    const char *file,
    const SdpHmacAlgAttributes *hmacAlgAttributes,
    const WsecVoid *hmacData, WsecUint32 hmacLen)
{
    WsecBuff hmacNew = { NULL, 0 };
    unsigned long ret;

    if (file == NULL || hmacAlgAttributes == NULL || hmacData == NULL || hmacLen == 0) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    WSEC_BUFF_ALLOC(hmacNew, SDP_HMAC_MAX_SIZE);
    if (hmacNew.buff == NULL) {
        WSEC_LOG_E4MALLOC(hmacNew.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        ret = SdpFileHmac(domain, file, hmacAlgAttributes, hmacNew.buff, &hmacNew.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("SdpFileHmac() = %lu", ret);
            break;
        }

        if (hmacLen != hmacNew.len) {
            ret = WSEC_ERR_HMAC_AUTH_FAIL;
            break;
        }
        if (WSEC_MEMCMP(hmacData, hmacNew.buff, hmacLen) != 0) {
            ret = WSEC_ERR_HMAC_AUTH_FAIL;
            break;
        }
    } while (0);

    WSEC_BUFF_FREE(hmacNew);
    return ret;
}

static unsigned long SdpGetHmacHeader(const unsigned char *hmacData, SdpHmacHeader *hmacHeader)
{
    SdpHmacHeader header;
    if (memcpy_s(&header, sizeof(SdpHmacHeader), hmacData, sizeof(SdpHmacHeader)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    /* Byte order */
    SdpCvtHmacTextHeaderByteOrder(&header, WBCNETWORK2HOST);
    /* Parameter check */
    if (header.version != SDP_HMAC_VER) {
        WSEC_LOG_E2("HMAC version is incompatible, %u expected, %u actually.", SDP_HMAC_VER, header.version);
        return WSEC_ERR_SDP_VERSION_INCOMPATIBLE;
    }
    if (header.iter != KMC_WORK_KEY_ITER_COUNT) {
        WSEC_LOG_E("Iterator count is out of bounds.");
        return WSEC_ERR_SDP_INVALID_CIPHER_TEXT;
    }
    if (CacAlgIdToType(header.algId) == WSEC_ALGTYPE_UNKNOWN) {
        WSEC_LOG_E("CAC Get algorithm types failed.");
        return WSEC_ERR_SDP_ALG_NOT_SUPPORTED;
    }
    if (memcpy_s(hmacHeader, sizeof(SdpHmacHeader), &header, sizeof(SdpHmacHeader)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    return WSEC_SUCCESS;
}

unsigned long SdpGetMkDetailByHmacData(WsecVoid *hmacData, WsecUint32 hmacLen, KmcMkInfo *mkInfo)
{
    unsigned long ret;
    SdpHmacHeader header = { 0, 0, 0, 0, 0, {0} };
    WsecUint32 tempHmacLen;
    unsigned char keyPlaintextBuff[SDP_KEY_MAX_LEN] = { 0 };
    WsecUint32 keyBuffLen = SDP_KEY_MAX_LEN;

    if (hmacData == NULL || mkInfo == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (hmacLen <= SDP_HMAC_HEAD_LEN || hmacLen <= sizeof(SdpHmacHeader)) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is not enough.");
        return WSEC_ERR_INPUT_BUFF_NOT_ENOUGH;
    }
    if (hmacLen > SDP_HMAC_HEAD_LEN + SDP_PTMAC_MAX_LEN) {
        WSEC_LOG_E("[SDP] Invalid parameter. Buffer for hmac text is out of bounds.");
        return WSEC_ERR_INVALID_ARG;
    }
    ret = SdpGetHmacHeader((const unsigned char *)hmacData, &header);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = CheckHmacHeader(header.algId, hmacLen, NULL, NULL, &tempHmacLen);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    ret = KmcGetMkDetail(header.domain, header.keyId, mkInfo, keyPlaintextBuff, &keyBuffLen);
    (void)memset_s(keyPlaintextBuff, sizeof(keyPlaintextBuff), 0, sizeof(keyPlaintextBuff));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E2("%s - %d, Get the KmcMkInfo failed.", __FUNCTION__, __LINE__);
    }
    return ret;
}
