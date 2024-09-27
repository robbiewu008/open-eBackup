/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: TLV used in the KMC V1 compatibility scenario
 * Author: x00102361
 * Create: 2019-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "sdpv1_tlv.h"
#include "wsecv2_util.h"
#include "wsecv2_order.h"
#include "wsecv2_file.h"
#include "wsecv2_errorcode.h"

#ifndef WSEC_COMPILE_SDP
#error Please defined 'WSEC_COMPILE_SDP' to compile
#endif

/* Read TLV */
WsecBool WsecReadTlv(WsecHandle stream, WsecVoid *buff, WsecUint32 buffSize, WsecTlv *tlv, unsigned long *errCode)
{
    unsigned long ret;
    unsigned long *err = NULL;

    WSEC_ASSERT(stream != NULL);
    WSEC_ASSERT(buff != NULL);
    WSEC_ASSERT(tlv != NULL);
    WSEC_ASSERT(buffSize > 0);

    err = (errCode != NULL) ? errCode : &ret;
    *err = WSEC_SUCCESS;
    if (WSEC_FREAD(tlv, sizeof(tlv->tag) + sizeof(tlv->len), stream) == WSEC_FALSE) {
        /* The file fails to be read. */
        *err = WSEC_ERR_READ_FILE_FAIL;
        return WSEC_FALSE;
    }

    WsecCvtByteOrder4Tlv(tlv, WBCNETWORK2HOST);
    if (buffSize < tlv->len) {
        WSEC_LOG_E2("Cannot write %u bytes to buffer(%u bytes).", tlv->len, buffSize);
        *err = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
        return WSEC_FALSE;
    }

    if (!WSEC_FREAD_MUST(buff, tlv->len, stream)) {
        *err = WSEC_ERR_READ_FILE_FAIL;
        return WSEC_FALSE;
    }

    tlv->val = buff;
    return WSEC_TRUE;
}

/* Write TLV */
unsigned long WsecWriteTlv(WsecHandle stream, WsecUint32 tag, WsecUint32 len, const void *val)
{
    WsecTlv tlv;
    tlv.tag = tag;
    tlv.len = len;
    tlv.val = NULL;
    WSEC_ASSERT(stream != NULL);
    WSEC_ASSERT(val != NULL);
    WsecCvtByteOrder4Tlv(&tlv, WBCHOST2NETWORK);
    if (!WSEC_FWRITE_MUST(&tlv, sizeof(tlv.tag) + sizeof(tlv.len), stream)) {
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    if (!WSEC_FWRITE_MUST(val, len, stream)) {
        return WSEC_ERR_WRI_FILE_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Convert to TLV */
void WsecCvtByteOrder4Tlv(WsecTlv *tlv, WsecUint32 direction)
{
    if (WBCHOST2NETWORK == direction) {
        tlv->tag = WSEC_H2N_L(tlv->tag);
        tlv->len = WSEC_H2N_L(tlv->len);
    } else {
        tlv->tag = WSEC_N2H_L(tlv->tag);
        tlv->len = WSEC_N2H_L(tlv->len);
    }
}

/* Check the result and write the TLV. */
unsigned long CheckResultAndWriteTlv(unsigned long result, WsecHandle writeStream,
    const unsigned char *val, WsecUint32 len, WsecUint32 tag)
{
    unsigned long ret;
    if (result != WSEC_SUCCESS) {
        WSEC_LOG_E1("Operation fail %lu", result);
        return result;
    }
    if (len == 0) {
        return WSEC_SUCCESS;
    }

    ret = WsecWriteTlv(writeStream, tag, len, val);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WsecWriteTlv()=%lu", ret);
        return ret;
    }
    return WSEC_SUCCESS;
}
