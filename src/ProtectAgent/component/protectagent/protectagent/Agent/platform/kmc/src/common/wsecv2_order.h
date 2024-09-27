/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_ORDER_H
#define KMC_SRC_COMMON_WSECV2_ORDER_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Byte order conversion */
typedef enum {
    WBCHOST2NETWORK, /* Host sequence to network sequence */
    WBCNETWORK2HOST  /* Convert the network sequence to the host sequence. */
} WsecByteOrderCvt;  /* Byte Order Conversion Method */

/* Determine the CPU alignment mode. */
WsecBool WsecIsBigEndianMode(void);

/* Check the CPU byte order. */
WsecVoid WsecCheckCpuEndianMode(void);

/* byte order: swap order */
#define WSEC_SWAP_SHORT(L) (WsecUint16)((((L) & 0x00FF) << 8) | (((L) & 0xFF00) >> 8))
#define WSEC_SWAP_LONG(L) (WsecUint32)((WSEC_SWAP_SHORT((L) & 0xFFFF) << 16) | WSEC_SWAP_SHORT(((L) >> 16) & 0xFFFF))

/* Byte order conversion */
#define WSEC_H2N_L(v) (WsecIsBigEndianMode() == WSEC_TRUE ? (v) : WSEC_SWAP_LONG(v))
#define WSEC_N2H_L(v) (WsecIsBigEndianMode() == WSEC_TRUE ? (v) : WSEC_SWAP_LONG(v))
#define WSEC_H2N_S(v) (WsecIsBigEndianMode() == WSEC_TRUE ? (v) : WSEC_SWAP_SHORT(v))
#define WSEC_N2H_S(v) (WsecIsBigEndianMode() == WSEC_TRUE ? (v) : WSEC_SWAP_SHORT(v))
/* Converts the byte order. The value is a 2-byte or 4-byte integer. */
#define WSEC_BYTE_ORDER_CVT_L(type, v) ((WsecUint32)(((type) == WBCHOST2NETWORK) ? WSEC_H2N_L(v) : WSEC_N2H_L(v)))
#define WSEC_BYTE_ORDER_CVT_S(type, v) ((WsecUint16)(((type) == WBCHOST2NETWORK) ? WSEC_H2N_S(v) : WSEC_N2H_S(v)))

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_ORDER_H */
