/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Hash public function implementation
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_hash.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

/* Generates hash values for specified data blocks in sequence. */
WsecBool WsecCreateHashCode(WsecUint32 hashAlg,
    const WsecBuffConst *buffs, WsecUint32 buffNum,
    WsecBuff *hashCode)
{
    WsecUint32 i;
    WsecHandle ctx = NULL;
    const WsecBuffConst *readBuff = NULL;
    unsigned long errorCode; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */

    WSEC_ASSERT(buffs != NULL);
    WSEC_ASSERT(buffNum > 0);
    WSEC_ASSERT(hashCode != NULL);
    WSEC_ASSERT(hashCode->buff != NULL);

    errorCode = CacDigestInit(&ctx, hashAlg);
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E1("CacDigestInit failed %lu", errorCode);
        return WSEC_FALSE;
    }

    for (i = 0, readBuff = buffs; i < buffNum; i++, readBuff++) {   /* soter 573 */
        if (readBuff->len == 0 || readBuff->buff == NULL) {
            continue;
        }
        errorCode = CacDigestUpdate(ctx, readBuff->buff, readBuff->len);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
    }

    if (errorCode == WSEC_SUCCESS) {
        errorCode = CacDigestFinal(&ctx, hashCode->buff, &(hashCode->len));
    } else {
        CacDigestReleaseCtx(&ctx);
    }
    return (errorCode == WSEC_SUCCESS) ? WSEC_TRUE : WSEC_FALSE;
}

/* Check whether the hash values of multiple specified data blocks are matched. */
unsigned long WsecCheckIntegrity(WsecUint32 hashAlg,
    const WsecBuffConst *checkBuff, WsecUint32 buffNum,
    const WsecVoid *cmpHashCode, WsecUint32 hashCodeLen)
{
    WsecBuff hashNew = { NULL, 0 };
    unsigned long returnCode;

    WSEC_ASSERT(checkBuff != NULL);
    WSEC_ASSERT(buffNum > 0);
    WSEC_ASSERT(cmpHashCode != NULL);
    WSEC_ASSERT(hashCodeLen > 0);

    WSEC_BUFF_ALLOC(hashNew, hashCodeLen);

    if (hashNew.buff == NULL) {
        WSEC_LOG_E1("Allocate memory(len=%u) fail.", hashNew.len);
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        if (!WsecCreateHashCode(hashAlg, checkBuff, buffNum, &hashNew)) {
            WSEC_LOG_E("Generate hash fail.");
            returnCode = WSEC_ERR_GEN_HASH_CODE_FAIL;
            break;
        }

        returnCode = WSEC_ERR_HASH_NOT_MATCH;
        if (hashNew.len == hashCodeLen) {
            if (WSEC_MEMCMP(hashNew.buff, cmpHashCode, hashCodeLen) == 0) {
                returnCode = WSEC_SUCCESS;
            }
        }
    } while (0);

    WSEC_BUFF_FREE(hashNew);
    return returnCode;
}
