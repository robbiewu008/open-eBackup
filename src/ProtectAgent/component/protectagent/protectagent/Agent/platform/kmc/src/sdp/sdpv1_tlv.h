/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: SDP V3 internal header file, which is not open to external systems.
 * Author: x00102361
 * Create: 2019-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_SRC_SDP_SDPV1_TLV_H
#define KMC_SRC_SDP_SDPV1_TLV_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TLV */
#pragma pack(1)
typedef struct TagWsecTlv {
    WsecUint32  tag;
    WsecUint32  len;
    void       *val;
} WsecTlv;
#pragma pack()

/* Read TLV */
WsecBool WsecReadTlv(WsecHandle stream, WsecVoid *buff, WsecUint32 buffSize, WsecTlv *tlv, unsigned long *errCode);

/* Write TLV */
unsigned long WsecWriteTlv(WsecHandle stream, WsecUint32 tag, WsecUint32 len, const void *val);

/* Convert to TLV */
void WsecCvtByteOrder4Tlv(WsecTlv *tlv, WsecUint32 direction);

/* Check the result and write the TLV. */
unsigned long CheckResultAndWriteTlv(unsigned long result, WsecHandle writeStream,
    const unsigned char *val, WsecUint32 len, WsecUint32 tag);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_SDP_SDPV1_TLV_H */
