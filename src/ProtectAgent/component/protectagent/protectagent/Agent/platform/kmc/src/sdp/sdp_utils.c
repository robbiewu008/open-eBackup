/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Implementation of utils of SDP.
 * Author: y00440103
 * Create: 2020-05-27
 */

#include "sdp_utils.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv2_itf.h"
#include "kmcv2_ksm.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_order.h"
#include "wsecv2_file.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

/* Check whether alg id is symmetric encryption alg */
WsecBool CheckIsSymAlg(WsecUint32 algId)
{
    WsecUint32 type = CacAlgIdToType(algId);
    if (type != WSEC_ALGTYPE_SYM) {
        WSEC_LOG_E1("[SDP] algType (%u) is not symmetric encrypt alg.", type);
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

/* Check whether alg id is hamc alg */
WsecBool CheckIsHmcAlg(WsecUint32 algId)
{
    WsecUint32 type = CacAlgIdToType(algId);
    if (type != WSEC_ALGTYPE_HMAC) {
        WSEC_LOG_E1("[SDP] algType (%u) is not hmac alg.", type);
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

WsecBool CheckIsAlgValid(WsecUint32 cipherAlgId, WsecUint32 hmacAlgId)
{
    if (CheckIsSymAlg(cipherAlgId) == WSEC_FALSE) {
        return WSEC_FALSE;
    }

    if (hmacAlgId == WSEC_ALGID_UNKNOWN) {
        return WSEC_TRUE;
    }

    if (CheckIsHmcAlg(hmacAlgId) == WSEC_FALSE) {
        return WSEC_FALSE;
    }

    return WSEC_TRUE;
}

/* Check whether is GCM alg id */
WsecBool CheckIsGcmAlgId(WsecUint32 algId)
{
    if (WSEC_IS2(algId, WSEC_ALGID_AES128_GCM, WSEC_ALGID_AES256_GCM)) {
        return WSEC_TRUE;
    }
    return WSEC_FALSE;
}

/* Check whether is CBC alg id */
WsecBool CheckIsCbcAlgId(WsecUint32 algId)
{
    if (WSEC_IS3(algId, WSEC_ALGID_AES128_CBC, WSEC_ALGID_AES256_CBC, WSEC_ALGID_SM4_CBC)) {
        return WSEC_TRUE;
    }
    return WSEC_FALSE;
}

/* Check whether the current version is correct. */
unsigned long CheckKsfV1(void)
{
    unsigned long ret;
    WsecUint16 ksfVersion;
    /* Check whether the version is matched. */
    ret = MemGetKsfVersion(&ksfVersion);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("get ksf version failed return %lu", ret);
        return ret;
    }
    /* Any valid Ksf version can access SDP v1,
     * In the scenario where the KMC is upgraded from V2 or compatible with V2,
     * the product side must ensure that the SDP V1 interface is not invoked to protect data,
     * Otherwise, after the downgrade, KMC V2 cannot process the SDP V1 interface used in KMC 3.0
     * for data protection because KMC V2 does not support the SDP V1 interface.
     */
    if (IsValidKsfVersion(ksfVersion) == WSEC_FALSE) {
        WSEC_LOG_E1("ksf version invalid(not v1) %hu", ksfVersion);
        return WSEC_ERR_KMC_KSF_VERSION_INVALID;
    }
    return WSEC_SUCCESS;
}

/* Check the result of the previous step and write data. */
unsigned long CheckResultAndWriteIfOk(unsigned long ret, WsecHandle writeStream, const WsecBuff *plainBuff)
{
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Operation fail %lu", ret);
        return ret;
    }
    if (plainBuff->len == 0) {
        return WSEC_SUCCESS;
    }
    if (!WSEC_FWRITE_MUST(plainBuff->buff, plainBuff->len, writeStream)) {
        WSEC_LOG_E("Write plain file  fail.");
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Safety minus two integers */
unsigned long SdpSafeSubTwo(WsecUint32 first, WsecUint32 second, long *remain)
{
    WsecUint32 temp = (WsecUint32)*remain;
    if (temp < first) {
        WSEC_LOG_E2("incorrect  tlv remain %u first length is %u ", temp, first);
        return WSEC_ERR_FILE_FORMAT;
    }
    temp -= first;
    if (temp < second) {
        WSEC_LOG_E2("incorrect tlv remain %u second length is %u", temp, second);
        return WSEC_ERR_FILE_FORMAT;
    }
    temp -= second;
    *remain = (long)temp;
    return WSEC_SUCCESS;
}

/* Searches for the currently effective MK in a specified domain and derives the working key based on the MK. */
unsigned long GetWorkKey(const KmcActiveKeyParam *mkParam, WsecUint32 *keyId, Pbkdf2Param *pbkdf2Param,
    unsigned char *iv, WsecUint32 ivLen, unsigned char *key, WsecUint32 keyLen)
{
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcMkInfo mkInfo = { 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } };
    unsigned char keyPlain[WSEC_MK_LEN_MAX] = {0};
    WsecUint32 keyLength; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    Pbkdf2ParamConst param;
    /* Verify parameters. */
    WSEC_ASSERT(pbkdf2Param != NULL);
    WSEC_ASSERT(pbkdf2Param->salt != NULL);
    WSEC_ASSERT(keyId != NULL);
    WSEC_ASSERT(key != NULL);

    /* Randomly obtain the salt value. */
    ret = CacRandom(pbkdf2Param->salt, pbkdf2Param->saltLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC calculate random failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }
    pbkdf2Param->iter = KMC_WORK_KEY_ITER_COUNT;

    /* Obtaining a Master Key */
    keyLength = sizeof(keyPlain);
    ret = KmcPriGetActiveMk(mkParam->domainId, mkParam->keyTypes, &mkInfo, keyPlain, &keyLength);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] KMC get ActiveMK failed. return = %lu", ret);
        return ret;
    }
    /* Obtaining the IV */
    if ((iv != NULL) && (ivLen > 0)) {
        ret = CacRandom(iv, ivLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("[SDP] CAC calculate random failed.");
            (void)memset_s(keyPlain, sizeof(keyPlain), 0, sizeof(keyPlain));
            return WSEC_ERR_GET_RAND_FAIL;
        }
    }

    /* Deriving a Working Key Based on the Master Key */
    param.salt = pbkdf2Param->salt;
    param.saltLen = pbkdf2Param->saltLen;
    param.iter = pbkdf2Param->iter;
    ret = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, keyPlain, keyLength, &param, keyLen, key);
    /* Destroying the stack master key */
    (void)memset_s(keyPlain, sizeof(keyPlain), 0, sizeof(keyPlain));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("[SDP] CAC pbkdf2 derive WK failed.");
        return WSEC_ERR_PBKDF2_FAIL;
    }

    /* Output Parameters */
    *keyId = mkInfo.keyId;
    return WSEC_SUCCESS;
}

/* Use the keyID to search for the MK in the specified domain and derive the working key based on the MK. */
unsigned long GetWorkKeyByID(WsecUint32 domain, WsecUint32 keyId, WsecUint32 iter,
    const unsigned char *salt, WsecUint32 saltLen, unsigned char *key, WsecUint32 keyLen)
{
    unsigned char masterKey[WSEC_MK_LEN_MAX] = {0};
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 mkLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    Pbkdf2ParamConst pbkdf2Param;
    pbkdf2Param.salt    = salt;
    pbkdf2Param.saltLen = saltLen;
    pbkdf2Param.iter    = (int)iter;
    /* Check the parameter value. */
    if (iter == 0 || salt == NULL || saltLen == 0 || key == NULL || keyLen == 0) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Obtaining a Master Key */
    mkLen = sizeof(masterKey);
    ret = KmcGetMkDetail(domain, keyId, NULL, masterKey, &mkLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] KMC get MK by keyId failed %lu.", ret);
        return ret;
    }

    /* Deriving a Working Key */
    ret = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, masterKey, mkLen, &pbkdf2Param, keyLen, key);
    /* Destroying the stack master key */
    (void)memset_s(masterKey, sizeof(masterKey), 0, sizeof(masterKey));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[SDP] GetWorkKeyByID CAC pbkdf2 derive WK failed %lu.", ret);
        return WSEC_ERR_PBKDF2_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Ciphertext Header Byte Sequence Conversion */
WsecVoid SdpCvtSdpCipherHeaderByteOrder(SdpCipherHeader *header, WsecUint32 direction)
{
    WSEC_ASSERT(header);
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));

    if (WBCHOST2NETWORK == direction) {
        header->version = WSEC_H2N_L(header->version);
        header->hmacFlag = WSEC_H2N_L(header->hmacFlag);
        header->domain = WSEC_H2N_L(header->domain);
        header->algId = WSEC_H2N_L(header->algId);
        header->keyId = WSEC_H2N_L(header->keyId);
        header->iter = WSEC_H2N_L(header->iter);
        header->cipherLen = WSEC_H2N_L(header->cipherLen);
    } else {
        header->version = WSEC_N2H_L(header->version);
        header->hmacFlag = WSEC_N2H_L(header->hmacFlag);
        header->domain = WSEC_N2H_L(header->domain);
        header->algId = WSEC_N2H_L(header->algId);
        header->keyId = WSEC_N2H_L(header->keyId);
        header->iter = WSEC_N2H_L(header->iter);
        header->cipherLen = WSEC_N2H_L(header->cipherLen);
    }
}

/* Byte order conversion: Convert the HMAC header byte order. */
WsecVoid SdpCvtHmacTextHeaderByteOrder(SdpHmacHeader *header, WsecUint32 direction)
{
    WSEC_ASSERT(header != NULL);
    WSEC_ASSERT(direction == WBCHOST2NETWORK || direction == WBCNETWORK2HOST);

    if (direction == WBCHOST2NETWORK) {
        header->version = WSEC_H2N_L(header->version);
        header->domain = WSEC_H2N_L(header->domain);
        header->algId = WSEC_H2N_L(header->algId);
        header->keyId = WSEC_H2N_L(header->keyId);
        header->iter = WSEC_H2N_L(header->iter);
    } else {
        header->version = WSEC_N2H_L(header->version);
        header->domain = WSEC_N2H_L(header->domain);
        header->algId = WSEC_N2H_L(header->algId);
        header->keyId = WSEC_N2H_L(header->keyId);
        header->iter = WSEC_N2H_L(header->iter);
    }
}

/* Releasing the HMAC context */
void SdpFreeHmacCtx(WsecHandle * const ctx)
{
    SdpHmacCtx *hmacCtx = NULL;
    if (ctx == NULL || *ctx == NULL) {
        return;
    }
    hmacCtx = (SdpHmacCtx *)(*ctx);
    if (hmacCtx->cacCtx != NULL) {
        CacHmacReleaseCtx(&hmacCtx->cacCtx);
    }
    WSEC_FREE(hmacCtx);
    *ctx = NULL;
}
