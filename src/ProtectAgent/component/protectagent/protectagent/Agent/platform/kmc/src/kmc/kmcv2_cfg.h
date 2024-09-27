/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * On March 19, 2019, Zhang Jie (employee ID: 00316590) split the original kmcv2_itf.c file
 * into kmcv2_ksf.c/kmcv2_ksm.c/kmcv2_itf.c/kmcv2_cfg.c to meet the requirements of the 2000-line file.
 *                     ITF: interface
 *                     KSM: keystore memory
 *                     KSF: keystore file
 */

#ifndef KMC_SRC_KMC_KMCV2_CFG_H
#define KMC_SRC_KMC_KMCV2_CFG_H

#include "wsecv2_config.h"
#include "wsecv2_type.h"
#include "kmcv2_pri.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Obtains the upper limit of the MK. */
int GetMkCountMax(void);

/* Obtaining the Grace Period of an Expired Key */
int CfgGetGraceDaysAfterKeyExpired(void);

/* Checking the KSF */
unsigned long CfgCheckKeyStore(WsecBool rewriteOnCheckFail, unsigned long *rewriteErrorCode, KmcKsfMem *ksfMem);

/* Updating the KSF Root Key */
unsigned long CfgUpdateRootKey(const unsigned char *entropy, WsecUint32 size,
    KmcKsfMem *ksfMem, WsecUint16 ksfVersion);

/* Load the KSF. If the KSF does not exist, create it. */
unsigned long CfgReadKsfSafety(WsecUint32 role, KmcKsfMem **keystore);

/* Whether the configuration is initialized */
WsecBool CfgIsCfgValid(void);

/* Obtains the domain type. If only a failure is returned, the possible cause is that the domain does not exist. */
unsigned long CfgGetDomainType(WsecUint32 domain, unsigned char *domainType);

/*
 * KMC configuration data initialization: Obtain the KMC configuration from the app through the callback function.
 * If a configuration item fails to be obtained, the default configuration is used.
 */
unsigned long CfgDataInitEx(void);

/* Search for the domain configuration and key type configuration based on domainId and keyType. */
WsecBool CfgSearchDomainKeyTypeCfg(WsecUint32 domainId, WsecUint16 keyType, KmcDomainCfg **domain,
    KmcCfgKeyType **keyTypeResult);

/* Releases g_kmcCfg. */
void CfgFreeKmcCfg(void);

/* Setting the Maximum Number of Master Keys */
void CfgSetMkMaxCount(int count);

/* Setting the Default Root Key */
WsecVoid CfgSetDefaultRootKeyCfg(const KmcCfgRootKey *rkCfg);

/* Setting the Root Key */
WsecVoid CfgSetRootKeyCfg(const KmcCfgRootKey *rkCfg);

/* Obtaining the Root Key Configuration */
WsecVoid CfgGetRootKeyCfg(KmcCfgRootKey *rkCfg);

/* Adding a Domain to the Memory */
unsigned long CfgAddDomainEx(const KmcCfgDomainInfo *domainInfo, const KmcKsfMem *ksfMem);

/* Deleting a Domain from the Memory */
unsigned long CfgRmvDomainEx(WsecUint32 domainId);

/* Adding a Master Key Type to a Specified Domain */
unsigned long CfgAddDomainKeyTypeEx(WsecUint32 domainId, const KmcCfgKeyType *keyTypeCfg);

/* Deleting a Master Key from a Specified Domain */
unsigned long CfgRmvDomainKeyTypeEx(WsecUint32 domainId, WsecUint16 keyType);

/* Obtaining the Number of Master Keys */
int CfgGetDomainCount(void);

/* Obtains the execution index domain info. */
unsigned long CfgGetDomain(int idx, KmcCfgDomainInfo *domainInfo);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMCV2_CFG_H */
