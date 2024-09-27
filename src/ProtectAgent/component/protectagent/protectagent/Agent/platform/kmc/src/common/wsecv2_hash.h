/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_HASH_H
#define KMC_SRC_COMMON_WSECV2_HASH_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hash */
WsecBool WsecCreateHashCode(WsecUint32 hashAlg,
    const WsecBuffConst *buffs, WsecUint32 buffNum,
    WsecBuff *hashCode);

/* Verify the hash. */
unsigned long WsecCheckIntegrity(WsecUint32 hashAlg,
    const WsecBuffConst *checkBuff, WsecUint32 buffNum,
    const WsecVoid *cmpHashCode, WsecUint32 hashCodeLen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_HASH_H */
