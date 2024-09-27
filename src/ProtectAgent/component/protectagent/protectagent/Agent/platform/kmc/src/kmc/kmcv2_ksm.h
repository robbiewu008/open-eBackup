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

#ifndef KMC_SRC_KMC_KMCV2_KSM_H
#define KMC_SRC_KMC_KMCV2_KSM_H

#include "wsecv2_type.h"
#include "kmcv2_pri.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Get memory keystore */
KmcKsfMem *KsmGetKeystore(void);

/* set g_keystore */
WsecVoid KsmSetKeystore(KmcKsfMem *newKeystore);

/* Obtains the detailed MK information, which corresponds to KmcGetMkDetail. */
unsigned long MemGetMkDetail(WsecUint32 domainId,
    WsecUint32 keyId,
    KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Check whether the memory KSF and configuration information is ready. */
unsigned long MemCheckKsfMemAndCfg(void);

/*
 * Check whether the memory KSF and configuration information is ready and whether the domain operation is
 * valid (in V1, only the master operates the shared domain).
 * Both V2 and V3 masters are supported, but the agent can operate only the local domain.
 */
unsigned long MemCheckKsfMemAndCfgDomain(WsecUint32 domain, WsecBool *shareDomainMkChanged);

/*
 * Check whether the memory KSF and configuration information is ready and whether the operation
 * permission is available. (Only the master is available in V1, and both V2 and V3 are available.)
 */
unsigned long MemCheckKsfMemAndCfgEx(void);

/* Release the global resource g_kmcCfg g_keystore and remove the hardware root key. */
WsecVoid MemFreeGlobal(void);

/* Release the global resource g_kmcCfg g_keystore and remove the hardware root key. */
WsecVoid MemFreeGlobalAndRemoveHardRk(void);

/* Updating the Root Key */
unsigned long MemUpdateRootKey(const unsigned char *entropy, WsecUint32 size, WsecUint16 ksfVersion);

/* Copying the RK Attribute */
WsecVoid MemGetRkAttr(KmcRkAttributes *rkAttr);

/* Obtains the maximum or minimum domain key ID. */
unsigned long MemGetEndMkId(WsecUint32 domainId, WsecUint32 *minId, WsecUint32 *maxId);

/* Key ID and Hash to obtain the key content */
unsigned long MemGetMkByIDHash(WsecUint32 domainId,
    WsecUint32 keyId,
    const unsigned char *hashData, WsecUint32 hashLen,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Load the KSF. If the KSF does not exist, create it. */
unsigned long MemLoadDataEx(WsecUint32 role);

/* Removing a Master Key */
unsigned long MemRmvMk(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged, KmcMkInfo *removeMk);

/* Delete specific number of oldest inactive mk in memory. */
unsigned long MemRmvMkByCount(WsecUint32 domainId, WsecBool shareDomainMkChanged, KmcMkInfoArray *rmvMkInfoArray);

/* Registering a Master Key */
unsigned long MemRegisterMkEx(WsecUint32 domainId,
    WsecUint32 keyId, WsecBool shareDomainMkChanged,
    const unsigned char *plaintextKey, WsecUint32 keyLen);

/* Creating a Master Key */
unsigned long MemCreateMkEx(WsecUint32 domainId, WsecUint32 currentMaxKeyId, WsecBool shareDomainMkChanged,
    WsecUint32 *keyId);

/* Obtaining the Number of Master Keys */
int MemGetMkCount(void);

/* Obtains the master key information of a specified index. */
unsigned long MemGetMkInfo(int idx, KmcMkInfo *memMk);

/* Obtaining the Number of MKs in a Specified Domain */
int MemGetMkCountByDomain(WsecUint32 domainId);

/* Obtain the key content and information using the key hash. */
unsigned long MemGetMkDetailByHash(const unsigned char *hashData, WsecUint32 hashLen,
    KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Key content obtaining key info */
unsigned long MemGetMkInfoByContent(const unsigned char *keyPlaintextBuff, WsecUint32 keyLen, KmcMkInfo *mkInfo);

/* +info (Obtaining the Activation Key) */
unsigned long MemGetActiveMk(WsecUint32 domainId, KmcKeyTypesInfo keyTypes, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Obtaining the Key Hash Value Based on the Key Domain and ID */
unsigned long MemGetMkHash(WsecUint32 domainId, WsecUint32 keyId, unsigned char *hashData, WsecUint32 *hashLen);

/* Setting the Status of a Specified Master Key */
unsigned long MemSetMkStatus(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged,
    unsigned char status, WsecBool *needNotify, KmcMkInfo *notifyMkInfo);

/* Obtaining the Status of a Specified Master Key */
unsigned long MemGetMkStatus(WsecUint32 domainId, WsecUint32 keyId, unsigned char *status);

/* Activating a Master Key */
unsigned long MemActivateMk(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged,
    WsecArray changedMkArray, KmcMkInfo *mkInfoNotify);

/* Update the memory information protection mask. */
unsigned long MemRefreshMkMaskEx(void);

/* Obtains the content ID and hash of the activation key in the current domain. */
unsigned long MemGetActiveMkWithHash(WsecUint32 domainId, unsigned char *keyBuff, WsecUint32 *keyBuffLen,
    WsecUint32 *keyId, unsigned char *keyHash, size_t hashLen);

/* Adding a Domain to the Memory */
unsigned long MemAddDomainEx(const KmcCfgDomainInfo *domainInfo);

/* Generate all current MK KSFs. */
unsigned long MemGenerateKsfAll(const char *keystoreFile);

/* Generate an MK KSF in a specified domain. */
unsigned long MemGenerateKsfByDomain(WsecUint32 domainId, const char *keystoreFile);

/* Generate active and standby KMC KSFs. */
unsigned long MemReGenerateKsf(void);

/* Generating a KSF of a Specified Version */
unsigned long MemGenerateV1V2Ksf(WsecUint16 ksfVersion, const char *keystoreFile);

/* Exports the memory key to a specified KSF file (internal interface). */
unsigned long MemExportKsf(const char *keystoreFile, KmcExportKsfCfg *exportKsfCfg);

/* Exports special MKs filter by domainIds and keyIds to keystore file. */
unsigned long MemExportByKeys(const char *keystoreFile, const KmcExportKsfByKeysCfg *exportKsfCfg);

/* Specify the keystore file, import it to the memory MK, and synchronize it to the active and standby KSFs. */
unsigned long MemImportKsf(const char *keystoreFile, KmcImportKsfCfg *importKsfCfg);

/* Checking the KSF */
unsigned long MemCheckKeyStore(WsecBool rewriteOnCheckFail, unsigned long *rewriteErrorCode);

/* Number of KSF updates obtained from the memory */
WsecVoid MemGetKsfUpdateNumFromMem(WsecUint32 *updateCounter);

/* Obtain the number of shared MK updates from the memory. */
WsecVoid MemGetSharedMkUpdateNumFromMem(WsecUint32 *updateCounter);

/* Obtaining the Current KSF Version */
unsigned long MemGetKsfVersion(WsecUint16 *ksfVersion);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMCV2_KSM_H */
