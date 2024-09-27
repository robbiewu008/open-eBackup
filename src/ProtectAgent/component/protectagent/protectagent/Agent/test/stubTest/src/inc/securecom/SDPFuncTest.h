#ifndef _SIGNTEST_H_
#define _SIGNTEST_H_

#include "gtest/gtest.h"
#include "stub.h"
#define private public

#include <string>
#include <sstream>
#include <iomanip>
#include "common/Log.h"
#include "common/Path.h"
#include "include/wsecv2_type.h"
#include "wsecv2_itf.h"
#include "src/sdp/sdpv1_itf.h"
#include "include/kmcv2_itf.h"
#include "securec.h"
#include "include/wsecv2_errorcode.h"
#include "securecom/KmcCallback.h"
#include "securecom/SDPFunc.h"

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string &, const mp_string &, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void *pthis);
mp_void StubSDPFuncLogVoid(mp_void *pthis);

class SDPFuncTest : public testing::Test {
public:
    Stub stub;
};

// *******************************************************************************
// typedef class WSEC_ERR_T (*SdpGetCipherDataLenEx)(mp_uint32 inLen, mp_uint32 *outLen);
// typedef class WSEC_ERR_T (*StubSDP_GetCipherDataLenType)(mp_uint32 inLen, mp_uint32 *outLen);


// // mp_int32 ret1 = memset_s(inBuf, inLen, 0, inLen);
// /* Typedef  mp_int32 (*memset_sType)(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3);
// typedef  mp_int32 (*StubSDP_memset_sType)(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3); */


// // qinlang
// typedef WSEC_ERR_T (*SDP_GetHmacAlgAttrType)(WSEC_UINT32 len, SDP_HMAC_ALG_ATTR *stHmacAlgAttr);
// typedef WSEC_ERR_T (*StubSDP_GetHmacAlgAttrType)(WSEC_UINT32 len, SDP_HMAC_ALG_ATTR *stHmacAlgAttr);

// typedef WSEC_ERR_T (*SDP_EncryptType)(WSEC_UINT32 len, const WSEC_BYTE *inBuf, WSEC_UINT32 inLen, WSEC_BYTE *outBuf,
//     WSEC_UINT32 *outLen);
// typedef WSEC_ERR_T (*StubSDP_EncryptType)(WSEC_UINT32 len, const WSEC_BYTE *inBuf, WSEC_UINT32 inLen, WSEC_BYTE *outBuf,
//     WSEC_UINT32 *outLen);

// typedef WSEC_ERR_T (*SDP_FileHmacType)(WSEC_UINT32 len, const WSEC_CHAR *filePath, const WSEC_PROGRESS_RPT_STRU *i,
//     const SDP_HMAC_ALG_ATTR *stHmacAlgAttr, WSEC_VOID *pvHmacData, WSEC_UINT32 *ulHDLen);
// typedef WSEC_ERR_T (*StubSDP_FileHmacType)(WSEC_UINT32 len, const WSEC_CHAR *filePath, const WSEC_PROGRESS_RPT_STRU *i,
//     const SDP_HMAC_ALG_ATTR *stHmacAlgAttr, WSEC_VOID *pvHmacData, WSEC_UINT32 *ulHDLen);


// // *******************************************************************************
// mp_void StubSDPFuncLogVoid(mp_void *pthis)
// {
//     return;
// }

// WSEC_ERR_T StubSDP_GetCipherDataLen(mp_uint32 inLen, mp_uint32 *outLen)
// {
//     return WSEC_SUCCESS;
// }

// WSEC_ERR_T StubSDP_GetCipherDataLen1(mp_uint32 inLen, mp_uint32 *outLen)
// {
//     return WSEC_FAILURE;
// }


// // QINLANG

// /* mp_int32 StubSDP_memset_s(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3)
// {
//     return 1;
// } */


// WSEC_ERR_T StubSDP_GetHmacAlgAttr(WSEC_UINT32 len, SDP_HMAC_ALG_ATTR *stHmacAlgAttr)
// {
//     return WSEC_SUCCESS;
// }

// WSEC_ERR_T StubSDP_Encrypt(WSEC_UINT32 len, const WSEC_BYTE *inBuf, WSEC_UINT32 inLen, WSEC_BYTE *outBuf,
//     WSEC_UINT32 *outLen)
// {
//     *outLen = 5; // GDB调出来的，不然参数校验不通过，会失败。
//     return WSEC_SUCCESS;
// }

// WSEC_ERR_T StubSDP_FileHmac(WSEC_UINT32 len, const WSEC_CHAR *filePath, const WSEC_PROGRESS_RPT_STRU *i,
//     const SDP_HMAC_ALG_ATTR *stHmacAlgAttr, WSEC_VOID *pvHmacData, WSEC_UINT32 *ulHDLen)

// {
//     return WSEC_SUCCESS;
// }
#endif
