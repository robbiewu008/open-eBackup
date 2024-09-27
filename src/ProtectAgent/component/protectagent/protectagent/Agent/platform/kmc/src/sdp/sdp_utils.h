/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Implementation of utils of SDP
 * Author: y00440103
 * Create: 2020-05-27
 */

#ifndef KMC_SRC_SDP_SDP_UTILS_H
#define KMC_SRC_SDP_SDP_UTILS_H

#include "wsecv2_type.h"
#include "kmcv2_pri.h"
#include "sdpv3_type.h"
#include "sdpv1_itf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Indicates the first two members of KmcKeyType: KMC_KEY_TYPE_ENCRPT KMC_KEY_TYPE_INTEGRITY. */
#define SDP_V1_SUPPORT_KEY_TYPE_COUNT 2

/* HMAC context */
typedef struct {
    WsecHandle    cacCtx;     /* Context of the CAC adaptation layer */
    SdpHmacHeader hmacHeader; /* Recorded HMAC header of the HMAC calculated by the big data platform. */
} SdpHmacCtx;

/* Check whether alg id is symmetric encryption alg */
WsecBool CheckIsSymAlg(WsecUint32 algId);

/* Check whether alg id is hamc alg */
WsecBool CheckIsHmcAlg(WsecUint32 algId);

/* Check whether alg id is valid */
WsecBool CheckIsAlgValid(WsecUint32 cipherAlgId, WsecUint32 hmacAlgId);

/* Check whether is GCM alg id */
WsecBool CheckIsGcmAlgId(WsecUint32 algId);

/* Check whether is CBC alg id */
WsecBool CheckIsCbcAlgId(WsecUint32 algId);

/* Check whether the current version is correct. */
unsigned long CheckKsfV1(void);

/* Check the result of the previous step and write data. */
unsigned long CheckResultAndWriteIfOk(unsigned long ret, WsecHandle writeStream, const WsecBuff *plainBuff);

/* Safety minus two integers */
unsigned long SdpSafeSubTwo(WsecUint32 first, WsecUint32 second, long *remain);

/* Searches for the currently effective MK in a specified domain and derives the working key based on the MK. */
unsigned long GetWorkKey(const KmcActiveKeyParam *mkParam, WsecUint32 *keyId, Pbkdf2Param *pbkdf2Param,
    unsigned char *iv, WsecUint32 ivLen, unsigned char *key, WsecUint32 keyLen);

/* Use the keyID to search for the MK in the specified domain and derive the working key based on the MK. */
unsigned long GetWorkKeyByID(WsecUint32 domain, WsecUint32 keyId, WsecUint32 iter,
    const unsigned char *salt, WsecUint32 saltLen, unsigned char *key, WsecUint32 keyLen);

/* Ciphertext Header Byte Sequence Conversion */
void SdpCvtSdpCipherHeaderByteOrder(SdpCipherHeader *header, WsecUint32 direction);

/* Byte order conversion: Convert the HMAC header byte order. */
WsecVoid SdpCvtHmacTextHeaderByteOrder(SdpHmacHeader *header, WsecUint32 direction);

/* Releasing the HMAC context */
void SdpFreeHmacCtx(WsecHandle * const ctx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_SDP_SDP_UTILS_H */
