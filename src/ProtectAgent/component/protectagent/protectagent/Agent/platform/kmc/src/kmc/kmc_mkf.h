/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component - export & import MKF
 * Author: yangdingfu
 * Create: 20-11-03
 * Notes: This File split from kmcv2_ksm.c since the original file is near 2000 lines.
 */

#ifndef KMC_SRC_KMC_KMC_MKF_H
#define KMC_SRC_KMC_KMC_MKF_H

#include "wsecv2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Encrypt each master key from the password derivation key and export the encrypted master key to a file. */
unsigned long MemExportMkFileEx(WsecUint16 mkfVersion,
    const char *destFile,
    const unsigned char * const password, const WsecUint32 passwordLen,
    WsecUint32 iter);

/* Importing All CMKs from a CMK Export File */
unsigned long MemImportMkFileEx(const char *fromFile,
    const unsigned char * const password, const WsecUint32 passwordLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMC_MKF_H */
