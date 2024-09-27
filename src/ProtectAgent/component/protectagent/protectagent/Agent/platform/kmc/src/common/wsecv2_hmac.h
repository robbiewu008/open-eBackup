/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_HMAC_H
#define KMC_SRC_COMMON_WSECV2_HMAC_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* hmac */
unsigned long WsecCreateHmacCode(WsecUint32 hmacAlg,
    const WsecBuffConst *buffs, WsecUint32 buffNum,
    const WsecBuff *key, WsecBuff *hmacData);

/* Verify the HMAC. */
unsigned long WsecCheckHmacCode(WsecUint32 hmacAlg,
    const WsecBuffConst *buffs, WsecUint32 buffNum,
    const WsecBuff *key, const WsecBuff *hmacData);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_HMAC_H */
