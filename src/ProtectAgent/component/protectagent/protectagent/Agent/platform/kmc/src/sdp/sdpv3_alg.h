/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: SDP V3 internal header file, which is not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_SRC_SDP_SDPV3_ALG_H
#define KMC_SRC_SDP_SDPV3_ALG_H

#include "wsecv2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Secure pointer value assignment */
#define WSEC_SAFE_ASSIGN(ptr, val) do { \
    if ((ptr) != NULL) {                \
        *(ptr) = (val);                 \
    }                                   \
} while (0)

/* Query the algorithm type, key length, IV length, and HMAC length based on the cryptographic algorithm ID. */
unsigned long SdpGetAlgPropertyEx(WsecUint32 algId,
    WsecUint32 *algType,
    WsecUint32 *keyLen,
    WsecUint32 *ivLen,
    WsecUint32 *hmacLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_SDP_SDPV3_ALG_H */
