/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: t00449241
 * Create: 2020-05-26
 */

#ifndef KMC_SRC_KMC_KMC_UTILS_H
#define KMC_SRC_KMC_KMC_UTILS_H

#include "wsecv2_type.h"
#include "wsecv2_array.h"
#include "kmcv2_pri.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Allocating Multiple KmcKsfMem */
WsecBool AllocMultiBlock(KmcKsfMem **buffs, size_t buffLen, WsecUint32 count);

/* This interface is used to release the dynamic memory of the KmcKsfMem type. The return value is NULL. */
KmcKsfMem *FreeKsfSnapshot(KmcKsfMem *data);

WsecVoid FreeKsfMemArraySnapshot(KmcKsfMem **readBuff, WsecUint32 buffLen);

/* Cloning KmcKsfMem and Selecting MK and HardRK */
unsigned long CloneKsfMem(WsecBool withMk, WsecBool withHardRk, const KmcKsfMem *src, KmcKsfMem **dest);

/* Clone KmcKsfMem, Optional clone All MK or without special mk, Clone HardRk */
unsigned long CloneKsfMemWithoutDomain(WsecBool withMk, WsecBool withHardRk,
    const KmcKsfMem *src, KmcKsfMem **dest, WsecUint32 domainId);

/* Get minMkId and maxMkId  */
unsigned long PriGetEndMkId(KmcKsfMem *ksfMem, WsecUint32 domainId, WsecUint32 *minId, WsecUint32 *maxId);

/* Get mk by domainId and keyId */
unsigned long PriGetMemMkByDomainIdKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId,
    KmcMemMk **foundMk, int *foundMkIndex);

/* Adds an MK to an array. */
unsigned long AddMkToArray(KmcKsfMem *keystore, KmcMemMk *mk, WsecBool alreadyProtected);

/*
 * Callback function for comparing the sizes of two KmcMemMk elements during quick array sorting or search.
 * The caller must ensure the validity of input parameters.
 */
int CompareMkForArr(const WsecVoid *p1, const WsecVoid *p2);

/* This method is invoked when array elements are removed. */
WsecVoid OnRemoveMkArr(WsecVoid *element, WsecUint32 elementSize);

/* Domain+keyid: Obtain the index location. */
int SearchMkByKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId);

/* Search MK in MK array by mk hash data */
int SearchMkArrayByHash(const WsecArray mkArray, const unsigned char *hashData, WsecUint32 hashLen);

/* Search MK in KmcKsfMem by mk hash data */
int SearchMkByKeyHash(const KmcKsfMem *keystore, const unsigned char *hashData, WsecUint32 hashLen);

/* Search MK by domainId and mk hash */
int SearchMkByDomainIdKeyHash(const KmcKsfMem *keystore, WsecUint32 domainId,
    const unsigned char *hashData, WsecUint32 hashLen);

/* Obtain the MK from the value of SearchMkByKeyId or SearchMkByKeyHash. */
unsigned long GetSearchedMk(const KmcKsfMem *keystore, int idx, KmcMemMk **mk);

/* Get MK by keyId and hash data, if the MK hash data got by keyId not equal input hash data, then get by hash again */
unsigned long GetMkByKeyIdAndHash(const KmcKsfMem *keystore, const KmcDomainKeyPair *keyParam, KmcMemMk **mk);

/* Remove Mk the domainId and keyId */
unsigned long PriRmvMkByDomainIdKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId);

/* Check the input domainId is invalidate */
unsigned long CheckInputDomainIdAndType(WsecUint32 domainId);

/* Check the mkdomain is validate */
unsigned long CheckDomainIsNotLocalDomain(WsecUint32 mkDomainId);

/* Check KmcKeyTypesInfo match the given keyType */
int TypeInSpecifiedTypes(WsecUint16 keyType, KmcKeyTypesInfo keyTypes);

/* Check KmcKsfMem MK Array Contains unique domain id */
unsigned long CheckMkArrayContainUniqueDomain(KmcKsfMem *ksfMem, WsecUint32* uniqueDomainId);

/* Deriving encKey and HmacKey */
unsigned long DeriveKey(const KmcMkfHeaderWithHmac *headerWithHmac, WsecBuffConst passwordBuff,
    WsecBuff encKeyBuff, WsecBuff hmacKeyBuff);

/* Filter the MK list based on domainId. */
unsigned long FilterByDomain(const WsecArray src, WsecUint32 domainId, WsecArray *dest);

/* Filter the MK based on domainId and domainType. */
unsigned long FilterMasterKeys(const WsecArray src, WsecUint32 domainId, WsecUint32 domainType,
    WsecArray *filterByDomainDest, WsecArray *filterDomainTypeDst);

/* Filter the MK based on domainType. */
unsigned long FilterByDomainType(const WsecArray src, WsecBool isAllDomain,
    WsecUint32 domainType, WsecArray *dest);

/* Add and swap mk with KsfMem struct */
unsigned long AddAndSwapMkWithKsfMem(KmcKsfMem *ksfMem, WsecArray addMkArray, WsecArray *dummyMkArray);

/* Create KmcMemMk based on the key information and plaintext key. */
unsigned long CreateMemMkFromInfoAndPlainKey(const KmcMkInfo *mkInfo,
    const unsigned char *key, WsecUint32 keyLen,
    KmcKsfMem *ksfMem);

/*
 * If WsecInitializeHw is invoked, the version is converted to V3.
 * (If this interface is invoked for upgrade, rollback is not considered.)
 */
unsigned long ConvertToV3IfHardware(KmcKsfMem *ksfMem);

/* Compare g_keystore mk existed in specific mkArray by domainId */
WsecBool CompareMkArray(const WsecArray compareMkArr, const WsecArray destCompareMkArr, WsecUint32 domainId);

/* Rollback After Activation Failure */
unsigned long RollbackByChangedArr(KmcKsfMem *ksfMem, WsecArray changedMkArray);
/* Safety init KsfMem array, initing the ksfMem array only it's uninited */
unsigned long InitKsfMemArraySafe(KmcKsfMem **targetKsfMem);

/* Add MK to target KsfMem incremantaly, orignal MK not overwrite. Noting to do if the MK is already exists. */
unsigned long AddMkToKsmByIncremental(KmcKsfMem *targetKsfMem, WsecArray addMkArray);

/* Add Mk to target KsfMem by full replacement, origin Mk will be overwrite by addMkArray */
unsigned long AddMkToKsmByFullReplace(KmcKsfMem *targetKsfMem, WsecArray addMkArray);

/* Creating an MK for a Keystore */
unsigned long CreateMkItemEx(KmcKsfMem *keystore,
    const KmcCfgDomainInfo *domainInfo,
    const KmcCfgKeyType *keyTypeCfg,
    const WsecBuffConst *plainKeyBuff,
    WsecUint32 keyId,
    WsecBool setActive);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMC_UTILS_H */
