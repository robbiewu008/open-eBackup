/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: t00449241
 * Create: 2020-05-26
 * History: 2020-05-26 tongli split util functions from kmcv2_ksm.c
 */

#include "kmc_utils.h"
#include "securec.h"
#include "kmcv2_pri.h"
#include "kmcv2_cfg.h"
#include "kmcv3_maskinfo.h"
#include "kmcv3_rk.h"
#include "cacv2_pri.h"
#include "sdpv3_type.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"
#include "wsecv2_order.h"
#include "wsecv2_datetime.h"
#include "wsecv2_errorcode.h"

/* Allocating Multiple KmcKsfMem */
WsecBool AllocMultiBlock(KmcKsfMem **buffs, size_t buffLen, WsecUint32 count)
{
    WsecUint32 i;
    WsecUint32 j;
    WsecBool returnValue = WSEC_TRUE;
    WSEC_ASSERT(buffs != NULL);
    WSEC_ASSERT(buffLen > 0);
    WSEC_ASSERT(count > 0);
    for (i = 0; i < count; i++) {
        buffs[i] = (KmcKsfMem *)WSEC_MALLOC(buffLen);
        if (buffs[i] == NULL) {
            returnValue = WSEC_FALSE;
            WSEC_LOG_E4MALLOC(buffLen);
            break;
        }
    }
    if (i < count) {
        for (j = 0; j < i; j++) {
            WSEC_FREE(buffs[j]);
            buffs[j] = NULL;
        }
    }
    return returnValue;
}

/* This interface is used to release the dynamic memory of the KmcKsfMem type. The return value is NULL. */
KmcKsfMem *FreeKsfSnapshot(KmcKsfMem *data)
{
    if (data != NULL) {
        data->mkArray = WsecArrFinalize(data->mkArray);
        KmcKsfHardRkUnloadFree(&data->hardRk);
        WSEC_FREE(data);
    }
    return data;
}

WsecVoid FreeKsfMemArraySnapshot(KmcKsfMem **readBuff, WsecUint32 buffLen)
{
    WsecUint32 i;
    for (i = 0; i < buffLen; i++) {
        readBuff[i] = FreeKsfSnapshot(readBuff[i]);
    }
}

/* Clone KmcKsfMem, Optional clone All MK or without special mk, Clone HardRk */
unsigned long CloneKsfMemWithoutDomain(WsecBool withMk, WsecBool withHardRk,
    const KmcKsfMem *src, KmcKsfMem **dest, WsecUint32 domainId)
{
    unsigned long ret = WSEC_SUCCESS;
    int i;
    int mkCount;
    KmcMemMk *mk = NULL;
    do {
        *dest = (KmcKsfMem *)WSEC_MALLOC(sizeof(KmcKsfMem));
        if (*dest == NULL) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        (void)memcpy_s((*dest), sizeof(KmcKsfMem), src, sizeof(KmcKsfMem));
        (void)memset_s(&(*dest)->hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
        (*dest)->mkArray = NULL;
        /* Handling HardRk Clones */
        if (withHardRk == WSEC_TRUE) {
            ret = KmcHardRkCloneMember(&src->hardRk, &(*dest)->hardRk);
            if (ret != WSEC_SUCCESS) {
                break;
            }
        }
        /* No need to return clone */
        if (withMk == WSEC_FALSE) {
            break;
        }
        /* Processing the MK Clone */
        (*dest)->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, OnRemoveMkArr);
        if ((*dest)->mkArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() fail CloneKsfMem");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        mkCount = WsecArrGetCount(src->mkArray);
        for (i = 0; i < mkCount; i++) {
            mk = (KmcMemMk *)WsecArrGetAt(src->mkArray, i);
            if (mk == NULL) {
                continue;
            }
            /* The sepcified domain needn't clone */
            if (KmcCompareDomain(mk->mkInfo.domainId, domainId) == KMC_COMPARE_EQ) {
                continue;
            }

            if (WsecArrCloneAddOrderly((*dest)->mkArray, mk, sizeof(KmcMemMk)) < 0) {
                WSEC_LOG_E("WsecArrCloneAddOrderly() fail.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }
    } while (0);
    if (ret != WSEC_SUCCESS) {
        (void)FreeKsfSnapshot(*dest);
    }
    return ret;
}

/* Cloning KmcKsfMem and Selecting MK and HardRK */
unsigned long CloneKsfMem(WsecBool withMk, WsecBool withHardRk, const KmcKsfMem *src, KmcKsfMem **dest)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    return CloneKsfMemWithoutDomain(withMk, withHardRk, src, dest, KMC_ALL_DOMAIN);
}

/* Domain+keyid: Obtain the index location. */
int SearchMkByKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId)
{
    int i;
    int count;
    KmcMemMk *mk = NULL;

    if (keystore == NULL) {
        return -1;
    }
    count = WsecArrGetCount(keystore->mkArray);
    for (i = (count - 1); i >= 0; i--) {    /* soter 554 */
        mk = (KmcMemMk *)WsecArrGetAt(keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if ((mk->mkInfo.domainId == domainId) && (mk->mkInfo.keyId == keyId)) {
            return i;
        }
    }

    return -1;
}

/* Get minMkId and maxMkId  */
unsigned long PriGetEndMkId(KmcKsfMem *ksfMem, WsecUint32 domainId, WsecUint32 *minId, WsecUint32 *maxId)
{
    int i;
    WsecUint32 maxKeyId = 0;
    WsecUint32 minKeyId = 0;
    WsecBool domainFound = WSEC_FALSE;
    WsecBool firstFound = WSEC_TRUE;
    const KmcMemMk *item = NULL;
    unsigned long ret = WSEC_ERR_KMC_MK_MISS;
    int mkCount; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    WSEC_ASSERT(minId != NULL);
    WSEC_ASSERT(maxId != NULL);
    mkCount = WsecArrGetCount(ksfMem->mkArray);
    for (i = 0; i < mkCount; i++) {
        item = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, i);
        if (item == NULL) {
            WSEC_LOG_E("Mk Array memory access fail.");
            domainFound = WSEC_FALSE;
            break;
        }

        if (domainFound == WSEC_TRUE && item->mkInfo.domainId != domainId) {
            /*
             * The MKs are arranged by domain. If a domain is scanned and enters another domain,
             * the loop does not need to be performed.
             */
            break;
        }

        if (item->mkInfo.domainId == domainId) {
            domainFound = WSEC_TRUE;
            if (firstFound) {
                minKeyId = item->mkInfo.keyId;
                firstFound = WSEC_FALSE;
            }
            if (maxKeyId < item->mkInfo.keyId) {
                maxKeyId = item->mkInfo.keyId;
            }
            if (minKeyId > item->mkInfo.keyId) {
                minKeyId = item->mkInfo.keyId;
            }
        }
    }

    if (domainFound == WSEC_TRUE) {
        *minId = minKeyId;
        *maxId = maxKeyId;
        ret = WSEC_SUCCESS;
    }
    return ret;
}

/* Get mk by domainId and keyId */
unsigned long PriGetMemMkByDomainIdKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId,
    KmcMemMk **foundMk, int *foundMkIndex)
{
    WSEC_ASSERT(foundMk != NULL);
    KmcMemMk *mk = NULL;
    int idx;
    idx = SearchMkByKeyId(keystore, domainId, keyId);
    if (idx < 0) {
        WSEC_LOG_E2("Key (DomainId=%u, KeyId=%u) not found", domainId, keyId);
        return WSEC_ERR_KMC_MK_MISS;
    }
    mk = (KmcMemMk *)WsecArrGetAt(keystore->mkArray, idx);
    if (mk == NULL) {
        WSEC_LOG_E("Get mk memory from mk array failed.");
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }
    *foundMk = mk;
    if (foundMkIndex != NULL) {
        *foundMkIndex = idx;
    }
    return WSEC_SUCCESS;
}

/*
 * Callback function for comparing the sizes of two KmcMemMk elements during quick array sorting or search.
 * The caller must ensure the validity of input parameters.
 */
int CompareMkForArr(const WsecVoid *p1, const WsecVoid *p2)
{
    const KmcMemMk *memMkA = NULL;
    const KmcMemMk *memMkB = NULL;
    const KmcMkInfo *mkA = NULL;
    const KmcMkInfo *mkB = NULL;

    WSEC_ASSERT(p1 != NULL);
    WSEC_ASSERT(p2 != NULL);
    memMkA = (const KmcMemMk *)(*(const WsecVoid * const *)p1);
    memMkB = (const KmcMemMk *)(*(const WsecVoid * const *)p2);

    mkA = &memMkA->mkInfo;
    mkB = &memMkB->mkInfo;

    if (mkA->domainId > mkB->domainId) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (mkA->domainId < mkB->domainId) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    /* If the values of ulDomainId are the same, check whether the key types of usType and V2 are the same, */
    if (mkA->keyType > mkB->keyType) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (mkA->keyType < mkB->keyType) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    /* If the values of domainId and usType are the same, check the value of ucStatus. */
    if (mkA->status > mkB->status) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (mkA->status < mkB->status) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    return WSEC_CMP_RST_EQUAL;
}

/* This method is invoked when array elements are removed. */
WsecVoid OnRemoveMkArr(WsecVoid *element, WsecUint32 elementSize)
{
    /*
     * There are plaintext MKs,
     * which need to be securely released (the memory needs to be erased before being released).
     */
    WSEC_CLEAR_FREE(element, elementSize);
}

/* Adds an MK to an array. */
unsigned long AddMkToArray(KmcKsfMem *keystore, KmcMemMk *mk, WsecBool alreadyProtected)
{
    unsigned long ret;
    int count;

    if (!(keystore != NULL && mk != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (keystore->mkArray != NULL) {
        count = WsecArrGetCount(keystore->mkArray);
        if (count >= WSEC_MK_NUM_MAX) {
            WSEC_LOG_E2("MkNum(%d) cannot over %u", count, WSEC_MK_NUM_MAX);
            return WSEC_ERR_KMC_MK_NUM_OVERFLOW;
        }
    } else {
        keystore->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, OnRemoveMkArr);
        if (keystore->mkArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() fail");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
    }

    if (SearchMkByKeyId(keystore, mk->mkInfo.domainId, mk->mkInfo.keyId) >= 0) {
        WSEC_LOG_E2("The MK(Domain=%u, KeyId=%u) exist.", mk->mkInfo.domainId, mk->mkInfo.keyId);
        return WSEC_ERR_KMC_ADD_REPEAT_MK;
    }

    /* Mask the plaintext before placing the array. */
    if (alreadyProtected == WSEC_FALSE) {
        ret = ProtectDataSameBuf(mk->mkRear.key, mk->mkRear.plaintextLen);
        if (ret != WSEC_SUCCESS) {
            return ret;
        }
    }
    if (WsecArrAddOrderly(keystore->mkArray, mk) < 0) {
        WSEC_LOG_E("WsecArrAddOrderly() fail.");
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }

    return WSEC_SUCCESS;
}

/*
 * If -1 is returned, the MK is not found.
 * If a non-negative value is returned, the MK index in the WsecArray is found.
 */
int SearchMkArrayByHash(const WsecArray mkArray, const unsigned char *hashData, WsecUint32 hashLen)
{
    int i;
    int count;
    KmcMemMk *mk = NULL;
    if (hashLen < WSEC_MK_HASH_REC_LEN) {
        return -1;
    }

    count = WsecArrGetCount(mkArray);
    for (i = (count - 1); i >= 0; i--) {    /* soter 554 */
        mk = (KmcMemMk *)WsecArrGetAt(mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (WSEC_MEMCMP(hashData, mk->hashData, WSEC_MK_HASH_REC_LEN) == 0) {
            return i;
        }
    }
    return -1;
}

/* Check the mkdomain is not local domain */
unsigned long CheckDomainIsNotLocalDomain(WsecUint32 mkDomainId)
{
    unsigned long ret;
    unsigned char domainType;
    ret = CfgGetDomainType(mkDomainId, &domainType);
    if (ret == WSEC_ERR_KMC_DOMAIN_MISS) {
        /*
         * We strongly recommend create domain before using domain, but for compatible history version,
         * the missing domain will be treat as share domain temporarily.
         */
        domainType = KMC_DOMAIN_TYPE_SHARE;
    }
    if (domainType == KMC_DOMAIN_TYPE_LOCAL) {
        return WSEC_ERR_KMC_DOMAIN_TYPE_ERROR;
    }
    if (mkDomainId != KMC_ALL_DOMAIN && mkDomainId >= KMC_PRI_DOMAIN_ID_MIN) {
        return WSEC_ERR_KMC_DOMAIN_NUM_OVERFLOW;
    }
    return WSEC_SUCCESS;
}

/* check the input domainId is validate */
unsigned long CheckInputDomainIdAndType(WsecUint32 domainId)
{
    if (domainId == KMC_ALL_DOMAIN) {
        return WSEC_SUCCESS;
    }
    return CheckDomainIsNotLocalDomain(domainId);
}

/* Check KmcKeyTypesInfo match the given keyType */
int TypeInSpecifiedTypes(WsecUint16 keyType, KmcKeyTypesInfo keyTypes)
{
    int ret = -1;
    int i;
    for (i = 0; i < keyTypes.typeCount; i++) {
        if (keyType == keyTypes.keyTypes[i]) {
            ret = i;
            break;
        }
    }
    return ret;
}

/*
 * If -1 is returned, the MK is not found.
 * If a non-negative value is returned, the MK index in the KmcKsfMem array is found.
 */
int SearchMkByKeyHash(const KmcKsfMem *keystore, const unsigned char *hashData, WsecUint32 hashLen)
{
    if (keystore == NULL || hashData == NULL || hashLen < WSEC_MK_HASH_REC_LEN) {
        return -1;
    }
    return SearchMkArrayByHash(keystore->mkArray, hashData, hashLen);
}

int SearchMkByDomainIdKeyHash(const KmcKsfMem *keystore, WsecUint32 domainId,
    const unsigned char *hashData, WsecUint32 hashLen)
{
    int i;
    int count;
    KmcMemMk *mk = NULL;
    if (keystore == NULL || hashData == NULL || hashLen < WSEC_MK_HASH_REC_LEN) {
        WSEC_LOG_E1("Invalid parameters, keystore or hashdata is null or hashlen(%u) too short", hashLen);
        return -1;
    }

    count = WsecArrGetCount(keystore->mkArray);
    for (i = (count - 1); i >= 0; i--) {    /* soter 554 */
        mk = (KmcMemMk *)WsecArrGetAt(keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (mk->mkInfo.domainId == domainId && WSEC_MEMCMP(hashData, mk->hashData, hashLen) == 0) {
            return i;
        }
    }
    return -1;
}

/* Check KmcKsfMem MK Array Contains unique domain id */
unsigned long CheckMkArrayContainUniqueDomain(KmcKsfMem *ksfMem, WsecUint32 *uniqueDomainId)
{
    WSEC_LOG_I("start check the received domain id validation.");
    unsigned long ret = WSEC_SUCCESS;
    KmcMemMk *recvMemMk = NULL;
    int recvMkCount;
    int i;

    recvMkCount = WsecArrGetCount(ksfMem->mkArray);
    if (recvMkCount > 0) {
        recvMemMk = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, 0);
        if (recvMemMk == NULL) {
            WSEC_LOG_E("unexpected error when get mk from mem ksf, key index:0");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
        *uniqueDomainId = recvMemMk->mkInfo.domainId;
    }

    for (i = 1; i < recvMkCount; i++) {
        recvMemMk = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, i);
        if (recvMemMk == NULL) {
            WSEC_LOG_E1("unexpected error when get mk from mem ksf, key index :%d", i);
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
        if (*uniqueDomainId != recvMemMk->mkInfo.domainId) {
            WSEC_LOG_E2("unexpected domain id, first domainID:%u,current:%u",
                *uniqueDomainId, recvMemMk->mkInfo.domainId);
            ret = WSEC_ERR_KMC_IMPORT_MK_CONFLICT_DOMAIN;
            break;
        }
    }
    return ret;
}

/* Deriving encKey and HmacKey */
unsigned long DeriveKey(const KmcMkfHeaderWithHmac *headerWithHmac, WsecBuffConst passwordBuff,
    WsecBuff encKeyBuff, WsecBuff hmacKeyBuff)
{
    Pbkdf2ParamConst pbkdf2Param;
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(passwordBuff.buff != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(hmacKeyBuff.buff != NULL);
    /* 1) Construct an MK encryption key. */
    pbkdf2Param.salt = headerWithHmac->mkfHeader.saltForEncKey;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(headerWithHmac->mkfHeader.saltForEncKey);
    pbkdf2Param.iter = (int)headerWithHmac->mkfHeader.iterForEncKey;
    if (CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, passwordBuff.buff, passwordBuff.len,
        &pbkdf2Param, encKeyBuff.len, encKeyBuff.buff) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacPbkdf2() fail.");
        return WSEC_ERR_PBKDF2_FAIL;
    }

    /* 2) Construct an HMAC key. */
    pbkdf2Param.salt = headerWithHmac->mkfHeader.saltForHmacKey;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(headerWithHmac->mkfHeader.saltForHmacKey);
    pbkdf2Param.iter = (int)headerWithHmac->mkfHeader.iterForHmacKey;
    if (CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256, passwordBuff.buff, passwordBuff.len,
        &pbkdf2Param, hmacKeyBuff.len, hmacKeyBuff.buff) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacPbkdf2() fail.");
        return WSEC_ERR_PBKDF2_FAIL;
    }

    return WSEC_SUCCESS;
}

/**
 * Filter the MK list to be exported or imported based on domainId and domainType.
*/
unsigned long FilterByDomain(const WsecArray src, WsecUint32 domainId, WsecArray *dest)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    unsigned long ret = WSEC_SUCCESS;
    KmcMemMk *mk = NULL;
    int i;
    do {
        /**
        * assign remove callback NULL, cause that add by linked orderly, may remove orginal data
        * call func WsecArrFinalize to free init (*dest) memory
        */
        (*dest) = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, NULL);
        if ((*dest) == NULL) {
            WSEC_LOG_E("filter by domain, allocate memory failed.");
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        int mkCount = WsecArrGetCount(src);
        for (i = 0; i < mkCount; i++) {
            mk = (KmcMemMk *)WsecArrGetAt(src, i);
            if (mk == NULL) {
                continue;
            }
            /* Except all domains, other domain IDs must be matched. */
            if (domainId != KMC_ALL_DOMAIN && mk->mkInfo.domainId != domainId) {
                continue;
            }
            if (WsecArrAddOrderly(*dest, mk) < 0) {
                WSEC_LOG_E("WsecArrAddOrderly() fail.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }
    } while (0);

    return ret;
}

/* Filter the MK based on domainId and domainType. */
unsigned long FilterMasterKeys(const WsecArray src, WsecUint32 domainId, WsecUint32 domainType,
    WsecArray *filterByDomainDst, WsecArray *filterDomainTypeDst)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(filterByDomainDst != NULL);
    WSEC_ASSERT(filterDomainTypeDst != NULL);
    unsigned long ret;

    do {
        ret = FilterByDomain(src, domainId, filterByDomainDst);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("filter master keys by domainId from mkArray failed.");
            break;
        }
        if (WsecArrGetCount(*filterByDomainDst) == 0) {
            WSEC_LOG_E("filter keys ksf by domain, but count result is zero.");
            ret = WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
            break;
        }
        ret = FilterByDomainType(*filterByDomainDst,
            (domainId == KMC_ALL_DOMAIN) ? WSEC_TRUE : WSEC_FALSE,
            domainType, filterDomainTypeDst);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("filter master keys by domainType from mkArray failed.");
            break;
        }
        if (WsecArrGetCount(*filterDomainTypeDst) == 0) {
            WSEC_LOG_E("filter keys ksf by domain type, but count result is zero.");
            ret = WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
            break;
        }
    } while (0);

    return ret;
}

static unsigned long GetDomainTypeWithMiss(WsecUint32 domainId, WsecUint32 filterDomainType, unsigned char *type)
{
    unsigned long ret = CfgGetDomainType(domainId, type);
    if (ret == WSEC_ERR_KMC_DOMAIN_MISS && filterDomainType != KMC_DOMAIN_TYPE_LOCAL) {
        *type = KMC_DOMAIN_TYPE_SHARE;
        ret = WSEC_SUCCESS;
    }
    return ret;
}

/**
 * Filter the MK list to be exported or imported based on domainId and domainType.
*/
unsigned long FilterByDomainType(const WsecArray src, WsecBool isAllDomain,
    WsecUint32 domainType, WsecArray *dest)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    unsigned long ret = WSEC_SUCCESS;
    unsigned char type;
    KmcMemMk *mk = NULL;
    int i;
    do {
        /**
         * assign remove callback NULL, cause that add by linked orderly, may remove orginal data
        *  call func WsecArrFinalize to free init (*dest) memory
        */
        (*dest) = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, NULL);
        if ((*dest) == NULL) {
            WSEC_LOG_E("filter by domain type, allocate memory failed.");
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        int mkCount = WsecArrGetCount(src);
        for (i = 0; i < mkCount; i++) {
            mk = (KmcMemMk *)WsecArrGetAt(src, i);
            if (mk == NULL) {
                continue;
            }

            if (GetDomainTypeWithMiss(mk->mkInfo.domainId, domainType, &type) != WSEC_SUCCESS) {
                WSEC_LOG_E("The target domain is not exsist");
                continue;
            }

            /* KMC_ALL_DOMAIN Domain Type Mismatch */
            if (isAllDomain == WSEC_TRUE && domainType != KMC_DOMAIN_TYPE_IGNORE && type != domainType) {
                WSEC_LOG_E("isAllDomain and input domainType not match with type.");
                continue;
            }
            /*
             * Single-domain && non-ignorable domain type.
             * Otherwise, the domain type corresponding to the key does not match the domainType parameter.
             */
            if (isAllDomain == WSEC_FALSE && domainType != KMC_DOMAIN_TYPE_IGNORE &&  type != domainType) {
                WSEC_LOG_E("type not match with input domainType, and input domainType is not ignore.");
                ret = WSEC_ERR_KMC_DOMAIN_TYPE_ERROR;
                break;
            }
            if (WsecArrAddOrderly(*dest, mk) < 0) {
                WSEC_LOG_E("filter by domain type WsecArrAddOrderly() fail.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }
    } while (0);

    return ret;
}

/* Add and swap mk with KsfMem struct */
unsigned long AddAndSwapMkWithKsfMem(KmcKsfMem *ksfMem, WsecArray addMkArray, WsecArray *dummyMkArray)
{
    WSEC_ASSERT(ksfMem != NULL);
    unsigned long ret = WSEC_SUCCESS;
    KmcMemMk *mk = NULL;
    int i;
    do {
        if (WsecArrGetCount(addMkArray) == 0) {
            WSEC_LOG_E("filter keys from mem ksf, but count result is zero.");
            ret = WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
            break;
        }

        (*dummyMkArray) = ksfMem->mkArray;
        ksfMem->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, NULL);
        if (ksfMem->mkArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() fail AddAndSwapMkWithKsfMem");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        int mkCount = WsecArrGetCount(addMkArray);
        for (i = 0; i < mkCount; i++) {
            mk = (KmcMemMk *)WsecArrGetAt(addMkArray, i);
            if (mk == NULL) {
                continue;
            }
            if (WsecArrAddOrderly(ksfMem->mkArray, mk) < 0) {
                WSEC_LOG_E("AddAndSwapMkWithKsfMem WsecArrAddOrderly() fail.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }
    } while (0);
    return ret;
}

/* Constructing a Plaintext Structure */
static unsigned long MakeMkRear(const WsecBuffConst *plainKeyBuff, const KmcCfgKeyType *keyTypeCfg, KmcMkRear *mkRear)
{
    /* 2) Key */
    if (plainKeyBuff != NULL) {
        if (!WSEC_IN_SCOPE(plainKeyBuff->len, 1, WSEC_MK_PLAIN_LEN_MAX)) {
            WSEC_LOG_E("The input key length  is too long.");
            return WSEC_ERR_KMC_MK_LEN_TOO_LONG;
        }
        if (memcpy_s(mkRear->key, sizeof(mkRear->key), plainKeyBuff->buff, (size_t)plainKeyBuff->len) != EOK) {
            WSEC_LOG_E4MEMCPY;
            return WSEC_ERR_MEMCPY_FAIL;
        }
        mkRear->plaintextLen = plainKeyBuff->len;
    } else {
        if (!WSEC_IN_SCOPE(keyTypeCfg->keyLen, 1, WSEC_MK_PLAIN_LEN_MAX)) {
            WSEC_LOG_E("The config key length is too long.");
            return WSEC_ERR_KMC_MK_LEN_TOO_LONG;
        }
        /* Generate a random number as the key. */
        if (CacRandom(mkRear->key, keyTypeCfg->keyLen) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacRandom() fail.");
            return WSEC_ERR_GET_RAND_FAIL;
        }
        mkRear->plaintextLen = keyTypeCfg->keyLen;
    }
    return WSEC_SUCCESS;
}

/*
 * An MK is generated based on the domain and key type information configured in the KMC.
 * The setActive parameter determines the initial status of the key,
 * If setActive is WSEC_FALSE, the MK status is KMC_KEY_STATUS_TOBEACTIVE,
 * Otherwise, the value is KMC_KEY_STATUS_ACTIVE.
 */
static unsigned long MakeMkEx(const KmcCfgDomainInfo *domainInfo, const KmcCfgKeyType *keyTypeCfg,
    const WsecBuffConst *plainKeyBuff, WsecUint32 keyId, KmcMemMk *mk, WsecBool setActive)
{
    KmcMkInfo *mkMainInfo = NULL;
    KmcMkRear *mkRear = NULL;
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    unsigned char hashData[KMC_HASH_SHA256_LEN] = {0};
    WsecUint32 hashLen = KMC_HASH_SHA256_LEN;

    if (!(domainInfo != NULL && keyTypeCfg != NULL && mk != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    mkMainInfo = &mk->mkInfo;
    mkRear = &mk->mkRear;

    /* (1) Set the key creation and expiration time. */
    ret = SetLifeTime(keyTypeCfg->keyLifeDays, &mkMainInfo->mkCreateTimeUtc, &mkMainInfo->mkExpiredTimeUtc);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("SetLifeTime failed %lu", ret);
        return ret;
    }

    /* 2) Fill in mkRear. */
    ret = MakeMkRear(plainKeyBuff, keyTypeCfg, mkRear);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* 3) Other information */
    mkMainInfo->domainId = domainInfo->domainId;
    mkMainInfo->keyId = keyId;
    mkMainInfo->keyType = keyTypeCfg->keyType;
    mkMainInfo->status = (unsigned char)(setActive ? KMC_KEY_STATUS_ACTIVE : KMC_KEY_STATUS_TOBEACTIVE);
    mkMainInfo->generateType = domainInfo->domainKeyFrom;

    /* Calculate the unique matching value of the mk plaintext. */
    if (CacDigest(WSEC_ALGID_SHA256, mkRear->key, mkRear->plaintextLen, hashData, &hashLen) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacDigest failed.");
        return WSEC_ERR_GEN_HASH_CODE_FAIL;
    }

    if (memcpy_s(mk->hashData, (size_t)WSEC_MK_HASH_REC_LEN, hashData, (size_t)WSEC_MK_HASH_REC_LEN) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }

    WSEC_LOG_I3("Master Key (DomainId=%u, KeyId= %u, KeyType=%u) generated",
        domainInfo->domainId, keyId, keyTypeCfg->keyType);

    return ret;
}

/* Create KmcMemMk based on the key information and plaintext key. */
unsigned long CreateMemMkFromInfoAndPlainKey(const KmcMkInfo *mkInfo,
    const unsigned char *key, WsecUint32 keyLen,
    KmcKsfMem *ksfMem)
{
    unsigned long errorCode;
    KmcCfgDomainInfo domainInfo;
    KmcCfgKeyType keyTypeCfg;
    WsecBuffConst plainKey = { NULL, 0 };
    KmcMemMk memMk;
    KmcMemMk *mkForAdd = NULL;

    WSEC_ASSERT(mkInfo != NULL);
    WSEC_ASSERT(key != NULL);
    WSEC_ASSERT(ksfMem != NULL);
    (void)memset_s(&domainInfo, sizeof(KmcCfgDomainInfo), 0, sizeof(KmcCfgDomainInfo));
    domainInfo.domainId = mkInfo->domainId;
    domainInfo.domainKeyFrom = mkInfo->generateType;

    (void)memset_s(&keyTypeCfg, sizeof(KmcCfgKeyType), 0, sizeof(KmcCfgKeyType));
    keyTypeCfg.keyLen = keyLen;
    keyTypeCfg.keyType = mkInfo->keyType;
    /*
     * Ensure that the data structure is valid. Set this parameter to 180 days,
     * which is meaningless because the actual decryption result will be copied later.
     * All fields of KmcMkInfo and KmcMkRear must be the same as the decryption result.
     * The root key is used to calculate the hash value.
     */
    /* Set this parameter to 180.The value will be determined based on the read date. */
    keyTypeCfg.keyLifeDays = DEFAULT_KEY_LIFE_DAYS;

    WSEC_BUFF_ASSIGN(plainKey, key, keyLen);
    errorCode = MakeMkEx(&domainInfo, &keyTypeCfg, &plainKey, mkInfo->keyId, &memMk, WSEC_TRUE);
    if (errorCode != WSEC_SUCCESS) {
        return errorCode;
    }

    /* The following information is subject to the data in the imported file */
    memMk.mkInfo.status = mkInfo->status;
    if (WsecDateTimeCopy(&mkInfo->mkCreateTimeUtc, &memMk.mkInfo.mkCreateTimeUtc) == WSEC_FALSE) {
        WSEC_LOG_E("WsecDateTimeCopy() fail.");
        (void)memset_s(&memMk, sizeof(KmcMemMk), 0, sizeof(KmcMemMk));
        return WSEC_ERR_MEMCPY_FAIL;
    }
    if (WsecDateTimeCopy(&mkInfo->mkExpiredTimeUtc, &memMk.mkInfo.mkExpiredTimeUtc) == WSEC_FALSE) {
        WSEC_LOG_E("WsecDateTimeCopy() fail.");
        (void)memset_s(&memMk, sizeof(KmcMemMk), 0, sizeof(KmcMemMk));
        return WSEC_ERR_MEMCPY_FAIL;
    }
    /* Add Array */
    mkForAdd = (KmcMemMk *)WSEC_CLONE_BUFF(&memMk, sizeof(KmcMemMk));
    (void)memset_s(&memMk, sizeof(KmcMemMk), 0, sizeof(KmcMemMk));
    if (mkForAdd == NULL) {
        WSEC_LOG_E("WSEC_CLONE_BUFF() fail.");
        return WSEC_ERR_MEMCLONE_FAIL;
    }
    errorCode = AddMkToArray(ksfMem, mkForAdd, WSEC_FALSE);
    if (errorCode != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(mkForAdd, sizeof(KmcMemMk));
    }
    return errorCode;
}

/*
 * If WsecInitializeHw is invoked, the version is converted to V3.
 * (If this interface is invoked for upgrade, rollback is not considered.)
 */
unsigned long ConvertToV3IfHardware(KmcKsfMem *ksfMem)
{
    if (ksfMem == NULL) {
        WSEC_LOG_E("Convert to V3 Hardware param check failed.");
        return WSEC_ERR_INVALID_ARG;
    }
    WsecUint16 ksfVersion = ksfMem->rk.rkAttributes.version;
    /* Not based on hardware protection and does not need to be converted to OceanStor V3. */
    if (PriKmcSysGetIsHardware() == WSEC_FALSE) {
        return WSEC_SUCCESS;
    }
    /* The version is already V3 and does not need to be converted to V3. */
    if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
        return WSEC_SUCCESS;
    }

    /* Conversion */
    WSEC_LOG_W1("Convert ksf version %hu to ksf version 3", ksfVersion);
    return CfgUpdateRootKey(NULL, 0, ksfMem, KMC_KSF_VER_V3);
}

/* Compare g_keystore mk existed in specific mkArray by domainId */
WsecBool CompareMkArray(const WsecArray compareMkArr, const WsecArray destCompareMkArr, WsecUint32 domainId)
{
    WSEC_ASSERT(compareMkArr != NULL);
    int i;
    int compareMkCount;
    int gMkCount;
    KmcMemMk *mk = NULL;
    KmcMemMk *findMk = NULL;
    int idx;

    gMkCount = WsecArrGetCount(destCompareMkArr);
    compareMkCount = WsecArrGetCount(compareMkArr);
    for (i = 0; i < gMkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(destCompareMkArr, i);
        if (mk == NULL) {
            continue;
        }
        if (domainId != KMC_ALL_DOMAIN && mk->mkInfo.domainId != domainId) {
            continue;
        }

        compareMkCount -= 1;
        idx = SearchMkArrayByHash(compareMkArr, mk->hashData, sizeof(mk->hashData));
        if (idx < 0) {
            WSEC_LOG_I1("CompareMkArray prepare to clear g_keystore domainId=%u", mk->mkInfo.domainId);
            return WSEC_FALSE;
        } else {
            findMk = (KmcMemMk *)WsecArrGetAt(compareMkArr, idx);
            if (findMk != NULL && findMk->mkInfo.status != mk->mkInfo.status) {
                WSEC_LOG_I1("CompareMkArray is not same with g_keystore domainId=%u", mk->mkInfo.domainId);
                return WSEC_FALSE;
            }
        }
    }

    /* indicating that mk existed in g_keystore mkArray */
    if (compareMkCount == 0) {
        return WSEC_TRUE;
    }

    return WSEC_FALSE;
}

/* Rollback After Activation Failure */
unsigned long RollbackByChangedArr(KmcKsfMem *ksfMem, WsecArray changedMkArray)
{
    int idx;
    int i;
    int mkChangedMkCount;
    KmcMemMk *mk = NULL;
    KmcMkInfo *mkInfo = NULL;
    unsigned long ret = WSEC_SUCCESS;

    mkChangedMkCount = WsecArrGetCount(changedMkArray);
    for (i = 0; i < mkChangedMkCount; i++) {
        mkInfo = (KmcMkInfo *)WsecArrGetAt(changedMkArray, i);
        if (mkInfo == NULL) {
            continue;
        }
        idx = SearchMkByKeyId(ksfMem, mkInfo->domainId, mkInfo->keyId);
        if (idx < 0) {
            WSEC_LOG_E2("Cannot find key(DomainId=%u, KeyId=%u)", mkInfo->domainId, mkInfo->keyId);
            ret = WSEC_ERR_KMC_ROLLBACK_FAIL;
            break;
        }
        mk = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, idx);
        if (mk == NULL) {
            WSEC_LOG_E2("Cannot find key(DomainId=%u, KeyId=%u)", mkInfo->domainId, mkInfo->keyId);
            ret = WSEC_ERR_KMC_ROLLBACK_FAIL;
            break;
        }
        mk->mkInfo.status = mkInfo->status;
    }
    WsecArrQuickSort(ksfMem->mkArray); /* Re-sort to the original status. */
    return ret;
}
/* Safety init KsfMem array, initing the ksfMem array only it's uninited */
unsigned long InitKsfMemArraySafe(KmcKsfMem **targetKsfMem)
{
    if ((*targetKsfMem)->mkArray == NULL) {
        (*targetKsfMem)->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0,
            CompareMkForArr, OnRemoveMkArr);
        if ((*targetKsfMem)->mkArray == NULL) {
            WSEC_LOG_E("KsfMem array initialize failed.");
            return WSEC_ERR_MALLOC_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/* Add MK to target KsfMem incremantaly, orignal MK not overwrite. Noting to do if the MK is already exists. */
unsigned long AddMkToKsmByIncremental(KmcKsfMem *targetKsfMem, WsecArray addMkArray)
{
    int idx;
    int i;
    KmcMemMk *mk = NULL;
    WsecUint32 maxId = 0;
    WsecUint32 minId = 0;
    int mkCount;

    mkCount = WsecArrGetCount(addMkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(addMkArray, i);
        if (mk == NULL) {
            continue;
        }

        if (mk->mkInfo.status == KMC_KEY_STATUS_TOBEACTIVE) {
            WSEC_LOG_E("Not support the MK with TOBEACTIVE status.");
            return WSEC_ERR_KMC_MK_NOT_SUPPORT_EXP_IMP;
        }

        /* If Mk exsit in target domain; should not add to the keystore */
        idx = SearchMkByDomainIdKeyHash(targetKsfMem, mk->mkInfo.domainId, mk->hashData, sizeof(mk->hashData));
        if (idx >= 0) {
            WSEC_LOG_W2("The mk already exsit in domainId:%u, keyId:%u", mk->mkInfo.domainId, mk->mkInfo.keyId);
            continue;
        }

        KmcMemMk *cloneElement = (KmcMemMk *)WSEC_CLONE_BUFF(mk, sizeof(KmcMemMk));
        if (cloneElement == NULL) {
            WSEC_LOG_E("Clone mk failed");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
        cloneElement->mkInfo.status = KMC_KEY_STATUS_INACTIVE;
        if (PriGetEndMkId(targetKsfMem, cloneElement->mkInfo.domainId, &minId, &maxId) != WSEC_SUCCESS) {
            maxId = 0;
        }
        cloneElement->mkInfo.keyId = maxId + 1;

        if (WsecArrAddOrderly(targetKsfMem->mkArray, cloneElement) < 0) {
            WSEC_LOG_E("AddMkToKsmByIncremental WsecArrAddOrderly failed.");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/* Add Mk to target KsfMem by full replacement, origin Mk will be overwrite by addMkArray */
unsigned long AddMkToKsmByFullReplace(KmcKsfMem *targetKsfMem, WsecArray addMkArray)
{
    int i;
    KmcMemMk *mk = NULL;
    int mkCount;

    mkCount = WsecArrGetCount(addMkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(addMkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (WsecArrCloneAddOrderly(targetKsfMem->mkArray, mk, sizeof(KmcMemMk)) < 0) {
            WSEC_LOG_E("AddMkToKsmByFullReplace WsecArrAddOrderly failed.");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/* Obtain the MK from the value of SearchMkByKeyId or SearchMkByKeyHash. */
unsigned long GetSearchedMk(const KmcKsfMem *keystore, int idx, KmcMemMk **mk)
{
    WSEC_ASSERT(mk != NULL);
    if (idx < 0) {
        return WSEC_ERR_KMC_MK_MISS;
    }

    *mk = (KmcMemMk *)WsecArrGetAt(keystore->mkArray, idx);
    if (*mk == NULL) {
        WSEC_LOG_E("memory access fail.");
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Get MK by keyId and hash data, if the MK hash data got by keyId not equal input hash data, then get by hash again */
unsigned long GetMkByKeyIdAndHash(const KmcKsfMem *keystore, const KmcDomainKeyPair *keyParam, KmcMemMk **mk)
{
    unsigned long ret;
    int idx;
    WsecUint32 domainId = keyParam->domainId;
    WsecUint32 keyId = keyParam->keyId;
    unsigned char *hashData = keyParam->hashData;
    WsecUint32 hashLen = keyParam->hashLen;

    ret = PriGetMemMkByDomainIdKeyId(keystore, domainId, keyId, mk, NULL);
    if (keyParam->hashData != NULL) {
        /* Search by hash value if the ID is not found or the hash value does not match. */
        if (!((ret == WSEC_SUCCESS) && (WSEC_MEMCMP((*mk)->hashData, hashData, hashLen) == 0))) {
            idx = SearchMkByKeyHash(keystore, hashData, hashLen);
            ret = GetSearchedMk(keystore, idx, mk);
        }
    }
    return ret;
}

unsigned long PriRmvMkByDomainIdKeyId(const KmcKsfMem *keystore, WsecUint32 domainId, WsecUint32 keyId)
{
    int idx;
    idx = SearchMkByKeyId(keystore, domainId, keyId);
    if (idx < 0) {
        WSEC_LOG_E2("Key (DomainId=%u, KeyId=%u) not found", domainId, keyId);
        return WSEC_ERR_KMC_MK_MISS;
    }
    WsecArrRemoveAt(keystore->mkArray, idx); /* Deleting a Node */
    return WSEC_SUCCESS;
}

/* Creating an MK for a Keystore */
unsigned long CreateMkItemEx(KmcKsfMem *keystore, const KmcCfgDomainInfo *domainInfo, const KmcCfgKeyType *keyTypeCfg,
    const WsecBuffConst *plainKeyBuff, WsecUint32 keyId, WsecBool setActive)
{
    KmcMemMk *mk = NULL;
    unsigned long errorCode;
    KmcMkChangeNotify notifyData;

    mk = (KmcMemMk *)WSEC_MALLOC(sizeof(KmcMemMk));
    if (mk == NULL) {
        WSEC_LOG_E4MALLOC(sizeof(KmcMemMk));
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        errorCode = MakeMkEx(domainInfo, keyTypeCfg, plainKeyBuff, keyId, mk, setActive);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("MakeMkEx failed %lu", errorCode);
            break;
        }

        errorCode = AddMkToArray(keystore, mk, WSEC_FALSE);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("AddMkToArray failed %lu", errorCode);
            break;
        }
    } while (0);

    if (errorCode == WSEC_SUCCESS) {
        (void)memcpy_s(&notifyData.mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));
        notifyData.type = (setActive == WSEC_TRUE ? (WsecUint32)KMC_KEY_ACTIVATED : (WsecUint32)KMC_KEY_TOBEACTIVATED);
        WSEC_NOTIFY(WSEC_KMC_NTF_MK_CHANGED, &notifyData, sizeof(notifyData));
    } else {
        /*
         * If the operation is successful, the memory resources are managed by the array.
         * Otherwise, the memory resources need to be released.
         */
        WSEC_CLEAR_FREE(mk, sizeof(KmcMemMk));
    }
    return errorCode;
}
