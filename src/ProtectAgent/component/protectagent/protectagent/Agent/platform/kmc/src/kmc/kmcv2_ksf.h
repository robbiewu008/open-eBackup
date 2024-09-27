/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * On March 19, 2019, Zhang Jie (employee ID: 00316590) split the original kmcv2_itf.c file into
 * kmcv2_ksf.c/kmcv2_ksm.c/kmcv2_itf.c/kmcv2_cfg.c to meet the requirements of the 2000-line file.
 *                     ITF: interface
 *                     KSM: keystore memory
 *                     KSF: keystore file
 */

#ifndef KMC_SRC_KMC_KMCV2_KSF_H
#define KMC_SRC_KMC_KMCV2_KSF_H

#include "wsecv2_config.h"
#include "wsecv2_type.h"
#include "kmcv2_pri.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Checking the KSF */
unsigned long KsfCheckKeyStore(WsecBool rewriteOnCheckFail,
    const KmcCfg *kmcCfg,
    unsigned long *rewriteErrorCode,
    KmcKsfMem *ksfMem);

/* Safely Erasing KSF */
unsigned long SecureEraseKeystore(void);

/* KSF Get KSF Update Times */
unsigned long GetKsfUpdateNumberFromKeystore(const char *keystoreFile, WsecUint32 *updateCounter);

/* Number of Times the KSF Obtains Shared MK Updates */
unsigned long GetSharedMkUpdateNumberFromKsf(const char *keystoreFile, WsecUint32 *updateCounter);

/* KSM generates KSF */
unsigned long GenKsf(KmcKsfMem *keystore, const char *keystoreFile, const char *callBy);

/* Secure KSF writing (dual-copy DR considered) */
unsigned long WriteKsfSafety(WsecBool shareDomainMkChanged, const KmcKsfRk *rkNew, KmcKsfMem *ksfMem,
    const char *callBy);

/*
 * Securely reads Keystore data.
 * The system supports two Keystore files. If an error occurs during data reading, the system reads another file.
 * 1. Preferentially select the data that can be correctly read from the file.
 * 2. If all the values are correctly read, the number of KSF updates is large.
 * 3. If the master node detects a file exception, it uses the normal file to overwrite the abnormal file.
 * 4. If the file is an agent file, the file is directly read. If one file is normal, the file can be loaded.
 * Note: This function writes the successfully read Keystore data into the global variable keystore.
 */
unsigned long ReadKsfSafety(WsecUint32 role, const KmcCfg *kmcCfg, KmcKsfMem **keystore);
/*
 * Reads data from a specified Keystore file (KSF for short).
 * Note: WSEC_ERR_KMC_READ_MK_FAIL indicates that an error occurs in the process of reading the media key,
 * However, the read MK is correctly output to keystoreData->mkArray.
 */
unsigned long ReadKsf(const char *keystoreFile, const char *callBy, KmcKsfMem *ksfMem);
/* Updating the KSF Root Key */
unsigned long KsfUpdateRootKey(const unsigned char *entropy, WsecUint32 size,
    const KmcCfg *kmcCfg,
    KmcKsfMem *ksfMem,
    WsecUint16 ksfVersion);

/*
 * KmcKsfHardRk indicates the root key. (The root key is removed when Update, RewriteOnCheckfail,
 * or Erase is executed. The root key is not removed when a new key is created because it is empty.)
 */
WsecVoid KmcRmvFreeKsfHardRk(KmcKsfHardRk *hardRk);

/* This interface is used to release the dynamic memory of the KmcKsfMem type. The return value is NULL. */
KmcKsfMem *FreeKsfSnapshot(KmcKsfMem *data); /* Creating a Root Key File */

/* Release KmcKsfMem array */
WsecVoid FreeKsfMemArraySnapshot(KmcKsfMem **readBuff, WsecUint32 buffLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMCV2_KSF_H */
