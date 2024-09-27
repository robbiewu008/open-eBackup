/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: HMAC public function implementation
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_hmac.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

/* Specifies the sequence of multiple data buffers to calculate their HMACs. */
unsigned long WsecCreateHmacCode(WsecUint32 hmacAlg, const WsecBuffConst *buffs, WsecUint32 buffNum,
    const WsecBuff *key, WsecBuff *hmacData)
{
    WsecHandle ctx = NULL;
    WsecUint32 i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    const WsecBuffConst *oneBuff = NULL;
    unsigned long errorCode; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(buffs != NULL);
    WSEC_ASSERT(buffNum > 0);
    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(key->buff != NULL);
    WSEC_ASSERT(hmacData != NULL);
    WSEC_ASSERT(hmacData->buff != NULL);

    /* 1. Initialize */
    errorCode = CacHmacInit(&ctx, (WsecUint32)hmacAlg, key->buff, key->len);
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E1("CacHmacInit failed %lu", errorCode);
        return errorCode;
    }

    /* 2. Update */
    for (i = 0, oneBuff = buffs; i < buffNum; i++, oneBuff++) { /* soter 573 */
        WSEC_ASSERT(oneBuff->buff != NULL);
        errorCode = CacHmacUpdate(ctx, oneBuff->buff, oneBuff->len);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("CacHmacUpdate failed %lu", errorCode);
            break;
        }
    }
    if (errorCode != WSEC_SUCCESS) {
        CacHmacReleaseCtx(&ctx);
        return errorCode;
    }

    /* 3. Finalize */
    return CacHmacFinal(&ctx, hmacData->buff, &hmacData->len);
}

/* To verify the function of calculating the HMAC results of multiple specified buffers in sequence */
unsigned long WsecCheckHmacCode(WsecUint32 hmacAlg, const WsecBuffConst *buffs, WsecUint32 buffNum,
    const WsecBuff *key, const WsecBuff *hmacData)
{
    WsecBuff hmacNew = { NULL, 0 };
    unsigned long errorCode;

    if (!(buffs != NULL && buffNum && key != NULL && hmacData != NULL)) {
        WSEC_LOG_E("WsecCheckHmacCode firstly check parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (!(hmacData->buff != NULL && (hmacData->len > 0))) {
        WSEC_LOG_E("WsecCheckHmacCode secondly check parameter invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    WSEC_BUFF_ALLOC(hmacNew, hmacData->len);

    do {
        if (hmacNew.buff == NULL) {
            WSEC_LOG_E4MALLOC(hmacNew.len);
            errorCode = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        errorCode = WsecCreateHmacCode(hmacAlg, buffs, buffNum, key, &hmacNew);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("Above function return %lu", errorCode);
            break;
        }

        if (WSEC_MEMCMP(hmacData->buff, hmacNew.buff, hmacNew.len) != 0) {
            errorCode = WSEC_ERR_HMAC_AUTH_FAIL;
            break;
        }
    } while (0);

    WSEC_BUFF_FREE(hmacNew);
    return errorCode;
}
