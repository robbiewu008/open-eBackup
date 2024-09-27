/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Obtains algorithm attributes of SDP V3.
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "sdpv3_alg.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_util.h"

#ifndef WSEC_COMPILE_SDP
#error Please defined 'WSEC_COMPILE_SDP' to compile
#endif

/* Query the algorithm type, key length, IV length, and HMAC length based on the cryptographic algorithm ID. */
unsigned long SdpGetAlgPropertyEx(WsecUint32 algId,
    WsecUint32 *algType,
    WsecUint32 *keyLen,
    WsecUint32 *ivLen,
    WsecUint32 *hmacLen)
{
    WsecUint32 type; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 tempKeyLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 tempIvLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WsecUint32 tempHmacLen; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    type = CacAlgIdToType(algId);
    if (type == WSEC_ALGTYPE_UNKNOWN) {
        WSEC_LOG_E("[SDP] CAC Get algorithm types failed.");
        return WSEC_ERR_SDP_ALG_NOT_SUPPORTED;
    }

    tempKeyLen = (type == WSEC_ALGTYPE_SYM) ? CacSymmKeyLen(algId) : SDP_KEY_MAX_LEN;
    if (tempKeyLen == 0) {
        tempKeyLen = SDP_KEY_MAX_LEN;
    }
    if (tempKeyLen > SDP_KEY_MAX_LEN) {
        WSEC_LOG_E1("[SDP] Length of key exceeds the limit, Actually %u.", tempKeyLen);
        return WSEC_ERR_INVALID_ARG;
    }

    tempIvLen = CacSymmIvLen(algId);
    if (tempIvLen > SDP_IV_MAX_LEN) {
        WSEC_LOG_E1("[SDP] Length of IV exceeds the limit, Actually %u.", tempIvLen);
        return WSEC_ERR_INVALID_ARG;
    }

    tempHmacLen = CacHMACSize(algId);
    if (tempHmacLen > SDP_PTMAC_MAX_LEN) {
        WSEC_LOG_E2("[SDP] Length of MAC exceeds the limit %d, Actually %u.", SDP_PTMAC_MAX_LEN, tempHmacLen);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Output Parameters */
    WSEC_SAFE_ASSIGN(algType, type);
    WSEC_SAFE_ASSIGN(keyLen, tempKeyLen);
    WSEC_SAFE_ASSIGN(ivLen, tempIvLen);
    WSEC_SAFE_ASSIGN(hmacLen, tempHmacLen);

    return WSEC_SUCCESS;
}
