/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component
 * Author: x00102361
 * Create: 2014-06-16
 * Notes: Anti-collision access protection must be implemented for global variables g_keystore and g_kmcCfg.
 * The protection mechanism is as follows: Public functions are locked and unlocked,
 * and private functions are not locked and unlocked.
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * On March 19, 2019, Zhang Jie (employee ID: 00316590) split
 * the original kmcv2_itf.c file into kmcv2_ksf.c/kmcv2_ksm.c/kmcv2_itf.c/kmcv2_cfg.c to
 * meet the requirements of the 2000-line file.
 *                     ITF: interface
 *                     KSM: keystore memory
 *                     KSF: keystore file
 *                     MKF: MK file
 */

#include "kmcv2_ksm.h"
#include "securec.h"
#include "kmcv3_maskinfo.h"
#include "kmcv2_ksf.h"
#include "kmcv3_rk.h"
#include "kmcv2_cfg.h"
#include "kmc_utils.h"

#include "wsecv2_errorcode.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"
#include "wsecv2_order.h"

static KmcKsfMem *g_keystore = NULL;

/* Get memory keystore */
KmcKsfMem* KsmGetKeystore(void)
{
    return g_keystore;
}

/* Set g_keystore */
WsecVoid KsmSetKeystore(KmcKsfMem *newKeystore)
{
    g_keystore = FreeKsfSnapshot(g_keystore);
    g_keystore = newKeystore;
}


/* Check whether the memory KSF and configuration information is ready. */
unsigned long MemCheckKsfMemAndCfg(void)
{
    if (PriKmcSysGetState() != WSEC_RUNNING) {
        WSEC_LOG_E("KMC not running.");
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }
    if (g_keystore == NULL) {
        WSEC_LOG_E("KeyStore memory does not exist.");
        return WSEC_ERR_KMC_KEYSTOREMEM_NOTEXIST;
    }
    if (CfgIsCfgValid() == WSEC_FALSE) {
        WSEC_LOG_E("Config memory does not exist");
        return WSEC_ERR_KMC_KEYCFGMEM_NOTEXIST;
    }
    return WSEC_SUCCESS;
}

/*
 * Check whether the memory KSF and configuration information is ready and whether the operation permission
 * is available. (Only the master is available in V1, and both V2 and V3 are available.)
 */
unsigned long MemCheckKsfMemAndCfgEx(void)
{
    unsigned long ret = MemCheckKsfMemAndCfg();
    WsecUint16 ksfVersion;
    WsecUint32 role = PriKmcSysGetRole();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ksfVersion = g_keystore->rk.rkAttributes.version;
    if (IsKsfV1(ksfVersion) == WSEC_TRUE && role == KMC_ROLE_MASTER) {
        return WSEC_SUCCESS;
    }
    if (IsKsfV2OrV3(ksfVersion) == WSEC_TRUE) {
        return WSEC_SUCCESS;
    }
    WSEC_LOG_E1("role type error %hu", ksfVersion);
    return WSEC_ERR_KMC_INVALID_ROLETYPE;
}

/*
 * Check whether the memory KSF and configuration information is ready
 * and whether the domain operation is valid (in V1, only the master operates the shared domain).
 * Both V2 and V3 masters are supported, but the agent can operate only the local domain.
 */
static unsigned long MemCheckDomainType(unsigned char domainType)
{
    WsecUint16 ksfVersion;
    WsecUint32 role = PriKmcSysGetRole();
    ksfVersion = g_keystore->rk.rkAttributes.version;
    if (IsKsfV1(ksfVersion) == WSEC_TRUE && role == KMC_ROLE_MASTER && domainType == KMC_DOMAIN_TYPE_SHARE) {
        return WSEC_SUCCESS;
    }
    if (IsKsfV2OrV3(ksfVersion) == WSEC_TRUE) {
        if (role == KMC_ROLE_AGENT && domainType != KMC_DOMAIN_TYPE_LOCAL) {
            WSEC_LOG_E("V2/V3 role type error, agent cannot mod share domain");
            return WSEC_ERR_KMC_INVALID_ROLETYPE;
        }
        return WSEC_SUCCESS;
    }
    WSEC_LOG_E2("Role type error, role type %u ksf version %hu", role, ksfVersion);
    return WSEC_ERR_KMC_INVALID_ROLETYPE;
}

/*
 * Check whether the memory KSF and configuration information is ready and whether the domain operation is
 * valid (in V1, only the master operates the shared domain). Both V2 and V3 masters are supported,
 * but the agent can operate only the local domain.
 */
unsigned long MemCheckKsfMemAndCfgDomain(WsecUint32 domain, WsecBool *shareDomain)
{
    unsigned long ret = MemCheckKsfMemAndCfg();
    unsigned char type;
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = CfgGetDomainType(domain, &type);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    if (shareDomain != NULL) {
        *shareDomain = (type == KMC_DOMAIN_TYPE_SHARE ? WSEC_TRUE : WSEC_FALSE);
    }
    return MemCheckDomainType(type);
}

/* Updating the Root Key */
unsigned long MemUpdateRootKey(const unsigned char *entropy, WsecUint32 size, WsecUint16 ksfVersion)
{
    return CfgUpdateRootKey(entropy, size, g_keystore, ksfVersion);
}

/* Load the KSF. If the KSF does not exist, create it. */
unsigned long MemLoadDataEx(WsecUint32 role)
{
    unsigned long ret;
    WSEC_ASSERT(PriKmcSysGetKsf(MASTER_KSF_INDEX) != NULL);
    WSEC_ASSERT(PriKmcSysGetKsf(BACKUP_KSF_INDEX) != NULL);
    ret = CfgDataInitEx();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    ret = CfgReadKsfSafety(role, &g_keystore);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("[KMC] ReadKsfSafety failed %lu.", ret);
        return ret;
    }
    return ConvertToV3IfHardware(g_keystore);
}

/* Obtains the detailed MK information, which corresponds to KmcGetMkDetail. */
unsigned long MemGetMkDetail(WsecUint32 domainId,
    WsecUint32 keyId,
    KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    KmcMemMk *mk = NULL;
    unsigned long errorCode;

    if (!(keyPlaintextBuff != NULL && keyBuffLen != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        errorCode = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("Can not find the matched key. errcode:%lu", errorCode);
            break;
        }

        if (mkInfo != NULL) {
            (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(mk->mkInfo));
        }
        if (*keyBuffLen < mk->mkRear.plaintextLen) {
            WSEC_LOG_E2("Key buffer len must at least %u, but only %u", mk->mkRear.plaintextLen, *keyBuffLen);
            errorCode = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }

        errorCode = UnprotectData(mk->mkRear.key, mk->mkRear.plaintextLen, keyPlaintextBuff, keyBuffLen);
    } while (0);
    return errorCode;
}

/* Release the global resource g_kmcCfg g_keystore and remove the hardware root key. */
WsecVoid MemFreeGlobal(void)
{
    /* Releases KmcKsfMem and KmcCfg. */
    g_keystore = FreeKsfSnapshot(g_keystore);
    CfgFreeKmcCfg();
    /* Releasing KmcSys */
    PriKmcSysUninit();
}

/* Release the global resource g_kmcCfg g_keystore and remove the hardware root key. */
WsecVoid MemFreeGlobalAndRemoveHardRk(void)
{
    KmcKsfHardRk hardRk;
    unsigned long ret;
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));

    ret = KmcHardRkCloneMember(&g_keystore->hardRk, &hardRk);
    MemFreeGlobal();
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("MemFreeGlobalAndRemoveHardRk KmcHardRkCloneMember %lu, hardware key can not be removed", ret);
        return;
    }
    KmcKsfHardRkRmvFree(&hardRk);
}

/* Copying the RK Attribute */
WsecVoid MemGetRkAttr(KmcRkAttributes *rkAttr)
{
    WSEC_ASSERT(rkAttr != NULL);
    (void)memcpy_s(rkAttr, sizeof(KmcRkAttributes), &g_keystore->rk.rkAttributes, sizeof(KmcRkAttributes));
}

/* Obtaining the Number of Master Keys */
int MemGetMkCount(void)
{
    return WsecArrGetCount(g_keystore->mkArray);
}

/* Obtaining the Number of MKs in a Specified Domain */
int MemGetMkCountByDomain(WsecUint32 domainId)
{
    int domainKeyCount;
    int i;
    int arrCount;
    KmcMemMk *mk = NULL;

    domainKeyCount = 0;
    arrCount = WsecArrGetCount(g_keystore->mkArray);
    for (i = 0; i < arrCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (mk->mkInfo.domainId == domainId) {
            domainKeyCount++;
        }
    }
    return domainKeyCount;
}

/* Obtain the key content and information using the key hash. */
unsigned long MemGetMkDetailByHash(const unsigned char *hashData, WsecUint32 hashLen, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    int i;
    int mkCount; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcMemMk *mk = NULL;
    unsigned long ret = WSEC_ERR_KMC_MK_MISS;
    WsecUint32 keyLen;
    WSEC_ASSERT(hashLen == WSEC_MK_HASH_REC_LEN);
    mkCount = WsecArrGetCount(g_keystore->mkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (WSEC_MEMCMP(hashData, mk->hashData, hashLen) == 0) {
            keyLen = mk->mkRear.plaintextLen; /* Obtaining a Key */
            if (*keyBuffLen < keyLen) {
                WSEC_LOG_E2("*keyBuffLen must at least given %u, but %u.", keyLen, *keyBuffLen);
                ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
                break;
            }
            ret = UnprotectData(mk->mkRear.key, mk->mkRear.plaintextLen, keyPlaintextBuff, keyBuffLen);

            /* Obtains the key information. */
            if (mkInfo != NULL) {
                (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));
            }
            break;
        }
    }

    return ret;
}

/* Key ID and Hash to obtain the key content */
unsigned long MemGetEndMkId(WsecUint32 domainId, WsecUint32 *minId, WsecUint32 *maxId)
{
    return PriGetEndMkId(g_keystore, domainId, minId, maxId);
}

/* Key ID and Hash to obtain the key content */
unsigned long MemGetMkByIDHash(WsecUint32 domainId, WsecUint32 keyId,
    const unsigned char *hashData, WsecUint32 hashLen, unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    int idx;
    KmcMemMk *mk = NULL;
    unsigned long ret;
    WSEC_ASSERT(hashLen >= WSEC_MK_HASH_REC_LEN);
    /* Search by ID and compare hash results */
    ret = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
    /* Search by hash value if the ID is not found or the hash value does not match. */
    if (!((ret == WSEC_SUCCESS) && (WSEC_MEMCMP(mk->hashData, hashData, WSEC_MK_HASH_REC_LEN) == 0))) {
        idx = SearchMkByKeyHash(g_keystore, hashData, hashLen);
        ret = GetSearchedMk(g_keystore, idx, &mk);
    }

    do {
        if (ret == WSEC_SUCCESS && mk != NULL) {
            if (*keyBuffLen < mk->mkRear.plaintextLen) {
                WSEC_LOG_E2("*keyBuffLen must at least given %u, but %u.", mk->mkRear.plaintextLen, *keyBuffLen);
                ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
                break;
            }
            ret = UnprotectData(mk->mkRear.key, mk->mkRear.plaintextLen, keyPlaintextBuff, keyBuffLen);
        }
    } while (0);
    return ret;
}

/* Removing a Master Key */
unsigned long MemRmvMk(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged, KmcMkInfo *removeMk)
{
    int idx;
    KmcMemMk *mk = NULL;
    KmcMemMk *mkBackup = NULL;
    unsigned long ret;
    WsecBool rollback = WSEC_FALSE;
    WSEC_ASSERT(removeMk != NULL);
    mkBackup = (KmcMemMk *)WSEC_MALLOC(sizeof(KmcMemMk));
    if (mkBackup == NULL) {
        WSEC_LOG_E4MALLOC(sizeof(KmcMemMk));
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        /* 1. Search for the MK. */
        ret = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, &idx);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /* Do not delete active keys. */
        if (mk->mkInfo.status == KMC_KEY_STATUS_ACTIVE) {
            WSEC_LOG_E2("Cannot remove active MK(DomainId=%u, KeyId=%u)", domainId, keyId);
            ret = WSEC_ERR_KMC_CANNOT_RMV_ACTIVE_MK;
            break;
        }

        /* Back up the key to be deleted and copy the key in mask protection mode. */
        (void)memcpy_s(mkBackup, sizeof(KmcMemMk), mk, sizeof(KmcMemMk));

        WsecArrRemoveAt(g_keystore->mkArray, idx); /* Deleting a Node */

        ret = WriteKsfSafety(shareDomainMkChanged, NULL, g_keystore, __FUNCTION__); /* Write to File */
        /* If the key fails to be written into the file, the key in the memory should be retained. */
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_W1("WriteKsfSafety()=%lu", ret);
            if (WsecArrAddOrderly(g_keystore->mkArray, mkBackup) < 0) {
                ret = WSEC_ERR_KMC_ROLLBACK_FAIL;
            } else {
                rollback = WSEC_TRUE;
            }
            break;
        }
        (void)memcpy_s(removeMk, sizeof(KmcMkInfo), &mkBackup->mkInfo, sizeof(KmcMkInfo));
    } while (0);

    if (rollback == WSEC_FALSE) {
        WSEC_FREE(mkBackup);
    }
    WSEC_LOG_I3("Del MK(DomainId=%u, KeyId=%u, KeyType=%u).", removeMk->domainId, removeMk->keyId, removeMk->keyType);

    return ret;
}

/* Removing mk by domainId and count */
static unsigned long RmvMkByDomainAndCount(KmcKsfMem *ksfMem, WsecUint32 domainId, KmcMkInfoArray *rmvMkInfoArray)
{
    WSEC_ASSERT(ksfMem != NULL);
    WSEC_ASSERT(rmvMkInfoArray != NULL);
    int idx = 0;
    unsigned long ret;
    WsecUint32 curKeyId;
    WsecUint32 minKeyId = 0;
    WsecUint32 maxKeyId = 0;
    int rmvedMkCount = 0;
    KmcMemMk *mk = NULL;
    KmcMkInfo *rmvMkInfos = rmvMkInfoArray->mkInfos;

    ret = MemGetEndMkId(domainId, &minKeyId, &maxKeyId);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Get min keyId failed.");
        return ret;
    }
    for (curKeyId = minKeyId; (curKeyId < maxKeyId) && (rmvedMkCount < rmvMkInfoArray->mkCount); curKeyId++) {
        ret = PriGetMemMkByDomainIdKeyId(ksfMem, domainId, curKeyId, &mk, &idx);
        if (ret == WSEC_ERR_KMC_MK_MISS) {  // if keyId is not continuously, skip curKeyId
            continue;
        }
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /* Do not delete active keys. */
        if (mk->mkInfo.status == KMC_KEY_STATUS_ACTIVE || mk->mkInfo.status == KMC_KEY_STATUS_TOBEACTIVE) {
            WSEC_LOG_E2("Cannot remove active MK(DomainId=%u, KeyId=%u)", domainId, curKeyId);
            ret = WSEC_ERR_KMC_CANNOT_RMV_ACTIVE_MK;
            break;
        }

        /* Back up the key to be deleted and copy the key in mask protection mode. */
        if (memcpy_s(&rmvMkInfos[rmvedMkCount], sizeof(KmcMkInfo), &(mk->mkInfo), sizeof(KmcMkInfo)) != EOK) {
            WSEC_LOG_E4MEMCPY;
            return WSEC_ERR_MEMCPY_FAIL;
        }

        WsecArrRemoveAt(ksfMem->mkArray, idx); /* Deleting a Node */
        rmvedMkCount++;
    }
    rmvMkInfoArray->mkCount = rmvedMkCount;
    return ret;
}

/* Delete specific number of oldest inactive mk in memory. */
unsigned long MemRmvMkByCount(WsecUint32 domainId, WsecBool shareDomainMkChanged, KmcMkInfoArray *rmvMkInfoArray)
{
    WSEC_ASSERT(rmvMkInfoArray != NULL);
    unsigned long ret;
    KmcKsfMem *ksfMem = NULL;
    do {
        ret = CloneKsfMem(WSEC_TRUE, WSEC_TRUE, g_keystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("%s CloneKsfMem failed %lu.", __FUNCTION__, ret);
            break;
        }
        if (ksfMem->mkArray == NULL || WsecArrGetCount(ksfMem->mkArray) == 0) {
            WSEC_LOG_E1("%s the number of mk is zero.", __FUNCTION__);
            ret = WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
            break;
        }

        ret = RmvMkByDomainAndCount(ksfMem, domainId, rmvMkInfoArray);
        if (ret != WSEC_SUCCESS && ret != WSEC_ERR_KMC_CANNOT_RMV_ACTIVE_MK) {
            break;
        }
        ret = WriteKsfSafety(shareDomainMkChanged, NULL, ksfMem, __FUNCTION__); /* Write to File */
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WriteKsfSafety failed %lu", ret);
        }
    } while (0);
    if (ret == WSEC_SUCCESS) {
        g_keystore = FreeKsfSnapshot(g_keystore);
        g_keystore = ksfMem;
    } else {
        /* If the key fails to be written into the file, the key in the memory should be retained. */
        (void)FreeKsfSnapshot(ksfMem);
    }
    return ret;
}

/* Obtain the configuration of the specified domain and key type to prepare for creating a key. */
static unsigned long NewMkPrepare(WsecUint32 domainId, WsecUint16 keyType,
    KmcDomainCfg **domainCfg, KmcCfgKeyType **keyTypeCfg)
{
    KmcDomainCfg *foundDomain = NULL;
    KmcCfgKeyType *foundKeyType = NULL;
    if (WsecArrGetCount(g_keystore->mkArray) == GetMkCountMax()) {
        WSEC_LOG_E1("MK num will overflow, curr num %d.", WsecArrGetCount(g_keystore->mkArray));
        return WSEC_ERR_KMC_MK_NUM_OVERFLOW;
    }

    /* 1. Parameter compliance check */
    /*
     * 1) If the domain is not configured or the key source is automatically generated by the system,
     * the value is invalid.
     */
    if (CfgSearchDomainKeyTypeCfg(domainId, keyType, &foundDomain, &foundKeyType) == WSEC_FALSE) {
        WSEC_LOG_E2("Domain keyType (DomainId=%u, KeyType=%u) miss.", domainId, keyType);
        return WSEC_ERR_KMC_DOMAIN_KEYTYPE_MISS;
    }

    if (foundDomain == NULL || foundKeyType == NULL) {
        WSEC_LOG_E2("Found Domain keyType (DomainId=%u, KeyType=%u) is empty.", domainId, keyType);
        return WSEC_ERR_KMC_DOMAIN_KEYTYPE_MISS;
    }
    *domainCfg = foundDomain;
    *keyTypeCfg = foundKeyType;
    return WSEC_SUCCESS;
}

/* Registration check */
static unsigned long CheckForRegisterMk(WsecUint32 domainId, WsecUint32 keyId, WsecUint32 keyLen,
    KmcCfgDomainInfo *domainInfo, KmcCfgKeyType *keyTypeCfg)
{
    KmcDomainCfg *foundDomain = NULL;
    KmcCfgKeyType *foundKeyType = NULL;
    WsecUint16 keyType = KMC_KEY_TYPE_ENCRPT_INTEGRITY;
    unsigned long ret;

    ret = NewMkPrepare(domainId, keyType, &foundDomain, &foundKeyType);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    (void)memcpy_s(domainInfo, sizeof(KmcCfgDomainInfo), &foundDomain->domainInfo, sizeof(KmcCfgDomainInfo));
    if (domainInfo->domainKeyFrom != KMC_MK_GEN_BY_IMPORT) {
        WSEC_LOG_E1("The keys of domain id=%u defined as inner-generated, cannot register.", domainInfo->domainId);
        return WSEC_ERR_KMC_CANNOT_REG_AUTO_KEY;
    }

    /* 2) If the MK already exists, registration is forbidden. */
    if (SearchMkByKeyId(g_keystore, domainId, keyId) >= 0) {
        WSEC_LOG_E2("The MasterKey(DomainId=%u, KeyId=%u) already exist.", domainId, keyId);
        return WSEC_ERR_KMC_REG_REPEAT_MK;
    }

    /* 3. If the plaintext length of the key is obviously not reliable, registration is prohibited. */
    if (!WSEC_IN_SCOPE(keyLen, 1, WSEC_MK_PLAIN_LEN_MAX)) {
        WSEC_LOG_E1("MK len is too long, it must not over %u.", WSEC_MK_PLAIN_LEN_MAX);
        return WSEC_ERR_KMC_MK_LEN_TOO_LONG;
    }
    keyTypeCfg->keyType = keyType;
    keyTypeCfg->keyLen = keyLen;
    keyTypeCfg->keyLifeDays = foundKeyType->keyLifeDays;
    return WSEC_SUCCESS;
}

/* Registering a Master Key */
unsigned long MemRegisterMkEx(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged,
    const unsigned char *plaintextKey, WsecUint32 keyLen)
{
    KmcCfgKeyType keyTypeCfg = { 0, 0, 0, {0} };
    KmcCfgDomainInfo domainInfo;
    WsecBuffConst plaintextKeyBuff = { NULL, 0 };
    unsigned long ret;

    do {
        ret = CheckForRegisterMk(domainId, keyId, keyLen, &domainInfo, &keyTypeCfg);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /* 2. Create an MK. */
        WSEC_BUFF_ASSIGN(plaintextKeyBuff, plaintextKey, keyLen);

        ret = CreateMkItemEx(g_keystore, &domainInfo, &keyTypeCfg, &plaintextKeyBuff, keyId, WSEC_FALSE);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* The key is successfully registered and the Keystore file needs to be written. */
        ret = WriteKsfSafety(shareDomainMkChanged, NULL, g_keystore, __FUNCTION__);
        /* Rollback after a file fails to be written */
        if (ret != WSEC_SUCCESS) {
            if (PriRmvMkByDomainIdKeyId(g_keystore, domainId, keyId) != WSEC_SUCCESS) {
                WSEC_LOG_E2("Master Key(DomainId=%u, KeyId=%u) can not found", domainId, keyId);
                return WSEC_ERR_KMC_ROLLBACK_FAIL;
            }
            WSEC_LOG_E1("MemRegisterMkEx WriteKsfSafety %lu", ret);
        }
    } while (0);

    return ret;
}

/* Creating a Master Key */
unsigned long MemCreateMkEx(WsecUint32 domainId, WsecUint32 currentMaxKeyId, WsecBool shareDomainMkChanged,
    WsecUint32 *keyId)
{
    KmcDomainCfg *domainCfg = NULL;
    KmcCfgKeyType *keyTypeCfg = NULL;
    /* Only keys with encryption and integrity protection attributes can be added. */
    WsecUint16 keyType = KMC_KEY_TYPE_ENCRPT_INTEGRITY;
    WsecUint32 newKeyId;
    unsigned long ret;
    ret = NewMkPrepare(domainId, keyType, &domainCfg, &keyTypeCfg);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    if (domainCfg->domainInfo.domainKeyFrom != KMC_MK_GEN_BY_INNER) {
        WSEC_LOG_E("Only mk-generated-inner domain can support this oper.");
        return WSEC_ERR_KMC_MK_GENTYPE_REJECT_THE_OPER;
    }

    newKeyId = currentMaxKeyId + 1;
    if (newKeyId == KMC_DOMAIN_OVERFLOWKEYID) {
        WSEC_LOG_E("Cannot CreateMK for the keyID of MK will overflow.");
        return WSEC_ERR_KMC_MKID_OVERFLOW;
    }
    ret = CreateMkItemEx(g_keystore, &domainCfg->domainInfo, keyTypeCfg, NULL, newKeyId, WSEC_FALSE);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = WriteKsfSafety(shareDomainMkChanged, NULL, g_keystore, __FUNCTION__);
    /* Rollback after a file fails to be written */
    if (ret != WSEC_SUCCESS) {
        if (PriRmvMkByDomainIdKeyId(g_keystore, domainId, newKeyId) != WSEC_SUCCESS) {
            WSEC_LOG_E2("Master Key(DomainId=%u, KeyId=%u) can not found", domainId, newKeyId);
            return WSEC_ERR_KMC_ROLLBACK_FAIL;
        }
        WSEC_LOG_E1("MemCreateMkEx WriteKsfSafety %lu", ret);
        return ret;
    }
    *keyId = newKeyId;
    return WSEC_SUCCESS;
}

/* Obtains the master key information of a specified index. */
unsigned long MemGetMkInfo(int idx, KmcMkInfo *mkInfo)
{
    KmcMkInfo *item = NULL;
    WSEC_ASSERT(mkInfo != NULL);
    if (idx >= WsecArrGetCount(g_keystore->mkArray)) {
        WSEC_LOG_E1("MK array overflow, index %d.", idx);
        return WSEC_ERR_INVALID_ARG;
    }
    item = (KmcMkInfo *)WsecArrGetAt(g_keystore->mkArray, idx);
    if (item == NULL) {
        WSEC_LOG_E("MK array memory access fail.");
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }
    (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), item, sizeof(KmcMkInfo));
    return WSEC_SUCCESS;
}

/* Key content obtaining key info */
unsigned long MemGetMkInfoByContent(const unsigned char *keyPlaintextBuff, WsecUint32 keyLen, KmcMkInfo *mkInfo)
{
    int i;
    int mkCount; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcMemMk *mk = NULL;
    unsigned long ret = WSEC_ERR_KMC_MK_MISS;
    unsigned char keyPlain[WSEC_MK_LEN_MAX] = {0};
    WsecUint32 keyPlainLen;

    mkCount = WsecArrGetCount(g_keystore->mkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (keyLen != mk->mkRear.plaintextLen) {
            continue;
        }
        keyPlainLen = keyLen;
        ret = UnprotectData(mk->mkRear.key, mk->mkRear.plaintextLen, keyPlain, &keyPlainLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (WSEC_MEMCMP(keyPlaintextBuff, keyPlain, keyPlainLen) == 0) {
            (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));
            ret = WSEC_SUCCESS;
            break;
        }
        ret = WSEC_ERR_KMC_MK_MISS;
    }
    (void)memset_s(keyPlain, sizeof(keyPlain), 0, sizeof(keyPlain));
    return ret;
}

/* +info (Obtaining the Activation Key) */
unsigned long MemGetActiveMk(WsecUint32 domainId, KmcKeyTypesInfo keyTypes, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    unsigned long ret = WSEC_ERR_KMC_CANNOT_FIND_ACTIVEKEY;
    int i;
    int mkCount; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcMemMk *mk = NULL;
    int typeIdx;

    mkCount = WsecArrGetCount(g_keystore->mkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (mk->mkInfo.domainId != domainId) {
            continue;
        }
        if (mk->mkInfo.status != KMC_KEY_STATUS_ACTIVE) {
            continue;
        }
        typeIdx = TypeInSpecifiedTypes(mk->mkInfo.keyType, keyTypes);
        if (typeIdx < 0) {
            continue;
        }
        if (*keyBuffLen < mk->mkRear.plaintextLen) {
            WSEC_LOG_E2("*keyBuffLen must at least given %u, but %u", mk->mkRear.plaintextLen, *keyBuffLen);
            ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }
        if (mkInfo != NULL) {
            (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));
        }
        ret = UnprotectData(mk->mkRear.key, mk->mkRear.plaintextLen, keyPlaintextBuff, keyBuffLen);
        /* Exit if the highest priority is found. */
        if (typeIdx == 0) {
            break;
        }
    }

    return ret;
}

/* Obtaining the Key Hash Value Based on the Key Domain and ID */
unsigned long MemGetMkHash(WsecUint32 domainId, WsecUint32 keyId, unsigned char *hashData, WsecUint32 *hashLen)
{
    unsigned long ret;
    KmcMemMk *mk = NULL;
    WsecUint32 tempLen = WSEC_MK_HASH_REC_LEN;

    ret = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E2("Key(DomainId=%u, KeyId=%u) not found", domainId, keyId);
        return ret;
    }

    /* Hash value of the output key */
    if (*hashLen < WSEC_MK_HASH_REC_LEN) {
        WSEC_LOG_E2("*hashLen must at least given %u, but %u", WSEC_MK_HASH_REC_LEN, *hashLen);
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    if (memcpy_s(hashData, (size_t)*hashLen, mk->hashData, (size_t)WSEC_MK_HASH_REC_LEN) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }
    *hashLen = tempLen;
    return WSEC_SUCCESS;
}

/* Setting the Status of a Specified Master Key */
unsigned long MemSetMkStatus(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged,
    unsigned char status, WsecBool *needNotify, KmcMkInfo *notifyMkInfo)
{
    KmcMemMk *mk = NULL;
    unsigned long errorCode;
    unsigned char tempStatus;
    WSEC_ASSERT(notifyMkInfo != NULL);
    WSEC_ASSERT(needNotify != NULL);
    do {
        errorCode = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E2("Cannot find Key(DomainId=%u, KeyId=%u)", domainId, keyId);
            break;
        }
        tempStatus = mk->mkInfo.status; /* Record the original status. */
        /* The value needs to be changed only when the status is different. */
        if (mk->mkInfo.status != status) {
            mk->mkInfo.status = status;
            WsecArrQuickSort(g_keystore->mkArray); /* Ensure that the MK array is sorted for'half search'. */
            errorCode = WriteKsfSafety(shareDomainMkChanged, NULL, g_keystore, __FUNCTION__);
            /*
             * Rollback operation. The file does not need to be rewritten because
             * the write operation will be rolled back if it fails.
             */
            if (errorCode != WSEC_SUCCESS) {
                mk->mkInfo.status = tempStatus;
                WsecArrQuickSort(g_keystore->mkArray); /* Re-sort to the original status. */
                break;
            }
            (void)memcpy_s(notifyMkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));
            *needNotify = WSEC_TRUE;
        }
    } while (0);
    return errorCode;
}

/* Obtaining the Status of a Specified Master Key */
unsigned long MemGetMkStatus(WsecUint32 domainId, WsecUint32 keyId, unsigned char *status)
{
    KmcMemMk *mk = NULL;
    unsigned long ret;

    do {
        ret = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("Cannot find Key(DomainId=%u, KeyId=%u)", domainId, keyId);
            break;
        }

        *status = mk->mkInfo.status; /* Obtains the original status. */
    } while (0);
    return ret;
}

/* Activate the key. */
static unsigned long DoActive(WsecUint32 domainId, WsecUint32 keyId, const KmcMemMk *toActiveMk,
    WsecBool shareDomainMkChanged, WsecArray changedMkArray)
{
    int i;
    int mkCount;
    int idx;
    KmcMemMk *mk = NULL;
    KmcMkInfo *mkInfo = NULL;
    unsigned long ret = WSEC_SUCCESS;

    mkCount = WsecArrGetCount(g_keystore->mkArray);
    for (i = 0; i < mkCount; i++) {
        mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (mk == NULL) {
            continue;
        }
        if (mk->mkInfo.domainId != domainId) {
            continue;
        }
        /*
         * For KMC V2 and later versions, the key type is KMC_KEY_TYPE_ENCRPT_INTEGRITY,
         * There may be three KMC V1 types, and each type may have an activation key.
         * Therefore, the key type should be used as a granularity instead of a domain.
         */
        if (mk->mkInfo.keyType != toActiveMk->mkInfo.keyType) {
            continue;
        }
        if (mk->mkInfo.status == KMC_KEY_STATUS_INACTIVE && mk->mkInfo.keyId != keyId) {
            continue; /* Invalid keys do not need to be backed up again. */
        }
        /* If the key is not invalid, reset it to invalid and back up it. The key must be set to valid later. */
        mkInfo = (KmcMkInfo *)WSEC_CLONE_BUFF(&mk->mkInfo, sizeof(KmcMkInfo));
        if (mkInfo == NULL) {
            WSEC_LOG_E4MALLOC(sizeof(KmcMkInfo));
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        idx = WsecArrAdd(changedMkArray, mkInfo);
        if (idx < 0) {
            WSEC_LOG_E("failed when add mkInfo to array");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        mk->mkInfo.status = KMC_KEY_STATUS_INACTIVE;
        /* Set the key to valid if it points to a specified key. */
        if (mk->mkInfo.keyId == keyId) {
            mk->mkInfo.status = KMC_KEY_STATUS_ACTIVE;
        }
    }
    if (ret == WSEC_SUCCESS) {
        WsecArrQuickSort(g_keystore->mkArray); /* Ensure that the MK array is sorted for'half search'. */
        ret = WriteKsfSafety(shareDomainMkChanged, NULL, g_keystore, __FUNCTION__);
    }
    return ret;
}

/* Activating a Master Key */
unsigned long MemActivateMk(WsecUint32 domainId, WsecUint32 keyId, WsecBool shareDomainMkChanged,
    WsecArray changedMkArray, KmcMkInfo *mkInfoNotify)
{
    KmcMemMk *mk = NULL;
    unsigned long ret;
    unsigned long tempErr;

    do {
        ret = PriGetMemMkByDomainIdKeyId(g_keystore, domainId, keyId, &mk, NULL);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("MemActivateMk Cannot find MK (DomainId=%u, KeyId=%u)", domainId, keyId);
            break;
        }
        (void)memcpy_s(mkInfoNotify, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));

        ret = DoActive(domainId, keyId, mk, shareDomainMkChanged, changedMkArray);
        /* Rollback Operations */
        if (ret != WSEC_SUCCESS) {
            tempErr = RollbackByChangedArr(g_keystore, changedMkArray);
            if (tempErr != WSEC_SUCCESS) {
                WSEC_LOG_E2("RollbackByChangedArr failed %lu %lu", tempErr, ret);
                ret = tempErr;
            }
        }
    } while (0);
    return ret;
}

/* Update the memory information protection mask. */
unsigned long MemRefreshMkMaskEx(void)
{
    KmcMemMk *memMk = NULL;
    int i;
    int mkCount;
    unsigned long ret;

    mkCount = WsecArrGetCount(g_keystore->mkArray);
    /* 1. Restore the original mask. */
    for (i = 0; i < mkCount; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (memMk == NULL) {
            continue;
        }
        ret = UnprotectDataSameBuf(memMk->mkRear.key, memMk->mkRear.plaintextLen);
        if (ret != WSEC_SUCCESS) {
            return ret;
        }
    }
    ret = KmcUnprotectDataForMaskedKey();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = KmcUnprotectRkMaterials(&g_keystore->rk);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    UninitMaskCode();
    ret = InitMaskCode();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* 3. Mask with a new mask. */
    for (i = 0; i < mkCount; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
        if (memMk == NULL) {
            continue;
        }
        ret = ProtectDataSameBuf(memMk->mkRear.key, memMk->mkRear.plaintextLen);
        if (ret != WSEC_SUCCESS) {
            return ret;
        }
    }
    ret = KmcProtectDataForMaskedKey();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = KmcProtectRkMaterials(&g_keystore->rk);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    return ret;
}

/* Obtains the content ID and hash of the activation key in the current domain. */
unsigned long MemGetActiveMkWithHash(WsecUint32 domainId, unsigned char *keyBuff, WsecUint32 *keyBuffLen,
    WsecUint32 *keyId, unsigned char *keyHash, size_t hashLen)
{
    KmcMemMk mkFind;
    KmcMemMk *mkFound = NULL;
    unsigned long ret;

    (void)memset_s(&mkFind, sizeof(KmcMemMk), 0, sizeof(KmcMemMk));

    mkFind.mkInfo.domainId = domainId;
    mkFind.mkInfo.keyType = KMC_KEY_TYPE_ENCRPT_INTEGRITY;
    mkFind.mkInfo.status = KMC_KEY_STATUS_ACTIVE; /* Search for available MKs. */
    do {
        mkFound = (KmcMemMk *)WsecArrBinarySearch(g_keystore->mkArray, &mkFind);
        if (mkFound == NULL) {
            WSEC_LOG_E1("Cannot find active MK (DomainId=%u)", domainId);
            ret = WSEC_ERR_KMC_MK_MISS;
            break;
        }
        if (*keyBuffLen < mkFound->mkRear.plaintextLen) {
            WSEC_LOG_E2("The buffer-len of keyBuff is too small (%u < %u).", *keyBuffLen, mkFound->mkRear.plaintextLen);
            ret = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }

        *keyId = mkFound->mkInfo.keyId;
        *keyBuffLen = mkFound->mkRear.plaintextLen;
        /* The MK plaintext is stored in the memory in mask mode and needs to be masked. */
        ret = UnprotectData(mkFound->mkRear.key, mkFound->mkRear.plaintextLen, keyBuff, keyBuffLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (memcpy_s(keyHash, hashLen, mkFound->hashData, (size_t)WSEC_MK_HASH_REC_LEN) != EOK) {
            WSEC_LOG_E4MEMCPY;
            ret = WSEC_ERR_MEMCPY_FAIL;
            break;
        }
    } while (0);

    return ret;
}

/* Adding a Domain to the Memory */
unsigned long MemAddDomainEx(const KmcCfgDomainInfo *domainInfo)
{
    return CfgAddDomainEx(domainInfo, g_keystore);
}

/* Generate all current MK KSFs. */
unsigned long MemGenerateKsfAll(const char *keystoreFile)
{
    return GenKsf(g_keystore, keystoreFile, __FUNCTION__);
}

/* Generate an MK KSF in a specified domain. */
unsigned long MemGenerateKsfByDomain(WsecUint32 domainId, const char *keystoreFile)
{
    unsigned long ret;
    KmcKsfMem *ksfMem = NULL;
    int i;
    int mkCount;
    KmcMemMk *mk = NULL;

    do {
        /* The hardware root key handle and KmcKsfRk are required during the write operation. */
        ret = CloneKsfMem(WSEC_FALSE, WSEC_TRUE, g_keystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* The removeElement callback must not exist. Otherwise, FreeKsfSnapshot releases all MKs in ksfMem. */
        ksfMem->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, NULL);
        if (ksfMem->mkArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() fail MemGenerateKsfByDomain");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        mkCount = WsecArrGetCount(g_keystore->mkArray);
        for (i = 0; i < mkCount; i++) {
            mk = (KmcMemMk *)WsecArrGetAt(g_keystore->mkArray, i);
            if (mk == NULL) {
                continue;
            }
            if (mk->mkInfo.domainId != domainId) {
                continue;
            }
            if (WsecArrAddOrderly(ksfMem->mkArray, mk) < 0) {
                WSEC_LOG_E("WsecArrAddOrderly() fail.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }

        if (ret != WSEC_SUCCESS) {
            break;
        }
        if ((WsecArrGetCount(ksfMem->mkArray) == 0)) {
            WSEC_LOG_E1("Can not find the keys in Domain :%u .", domainId);
            ret = WSEC_ERR_KMC_MK_MISS;
            break;
        }
        ret = GenKsf(ksfMem, keystoreFile, __FUNCTION__);
    } while (0);

    (void)FreeKsfSnapshot(ksfMem);
    return ret;
}

/* Generate active and standby KMC KSFs. */
unsigned long MemReGenerateKsf(void)
{
    unsigned long ret;
    /* All keys remain unchanged. Only the hardware key is used for encryption and then written into the KSF. */
    if (IsKsfV3(g_keystore->rk.rkAttributes.version) == WSEC_TRUE) {
        ret = KmcHardRkReEncSrk(&g_keystore->hardRk);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("%s KmcHardRkReEncSrk failed %lu", __FUNCTION__, ret);
            return ret;
        }
    }
    return WriteKsfSafety(WSEC_FALSE, NULL, g_keystore, __FUNCTION__);
}

/* Generating a KSF of a Specified Version */
unsigned long MemGenerateV1V2Ksf(WsecUint16 ksfVersion, const char *keystoreFile)
{
    unsigned long ret;
    KmcKsfMem *ksfMem = NULL;
    if (IsKsfV3(g_keystore->rk.rkAttributes.version) != WSEC_TRUE) {
        WSEC_LOG_E("save as can only be occured from ksf version 3 to ksf v1 or v2");
        return WSEC_ERR_KMC_KSF_VERSION_INVALID;
    }
    do {
        /* The hardware root key handle and KmcKsfRk are required during the write operation. */
        ret = CloneKsfMem(WSEC_TRUE, WSEC_FALSE, g_keystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ksfMem->rk.rkAttributes.version = ksfVersion;
        ret = GenKsf(ksfMem, keystoreFile, __FUNCTION__);
    } while (0);

    (void)FreeKsfSnapshot(ksfMem);
    return ret;
}

/* Exports the memory key to a specified KSF file (internal interface). */
unsigned long MemExportKsf(const char *keystoreFile, KmcExportKsfCfg *exportKsfCfg)
{
    unsigned long ret;
    KmcKsfMem *ksfMem = NULL;
    WsecArray tempArray = NULL;
    WsecArray filterMkArrByDomain = NULL;
    WsecArray filterMkByDomainType = NULL;
    if (IsKsfV1(g_keystore->rk.rkAttributes.version) == WSEC_TRUE) {
        WSEC_LOG_E("V1 could not call MemExportKsf inner func to export mk.");
        return WSEC_ERR_KMC_KSF_VERSION_INVALID;
    }

    do {
        ret = CloneKsfMem(WSEC_TRUE, WSEC_TRUE, g_keystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (ksfMem->mkArray == NULL || WsecArrGetCount(ksfMem->mkArray) == 0) {
            WSEC_LOG_E("read mk from ksf, the number of mk is zero.");
            ret = WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
            break;
        }
        ret = FilterMasterKeys(ksfMem->mkArray, exportKsfCfg->domainId, exportKsfCfg->domainType,
            &filterMkArrByDomain, &filterMkByDomainType);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MemExportKsf failed, ret=%lu", ret);
            break;
        }
        /* add mk to mkArray */
        ret = AddAndSwapMkWithKsfMem(ksfMem, filterMkByDomainType, &tempArray);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* v3 downgrading to v2 */
        if (exportKsfCfg->withHw == WSEC_FALSE) {
            ksfMem->rk.rkAttributes.version = KMC_KSF_VER_V2;
        }
        ret = GenKsf(ksfMem, keystoreFile, __FUNCTION__);
    } while (0);

    (void)WsecArrFinalize(filterMkArrByDomain);
    (void)WsecArrFinalize(filterMkByDomainType);
    (void)WsecArrFinalize(tempArray);
    (void)FreeKsfSnapshot(ksfMem);
    return ret;
}

/* Exports special MKs filter by domainIds and keyIds to keystore file. */
unsigned long MemExportByKeys(const char *keystoreFile, const KmcExportKsfByKeysCfg *exportKsfCfg)
{
    unsigned long ret;
    KmcKsfMem *ksfMem = NULL;
    KmcMemMk *mk = NULL;
    WsecUint32 domainId;
    int i;

    if (IsKsfV1(g_keystore->rk.rkAttributes.version) == WSEC_TRUE) {
        WSEC_LOG_E("V1 could not call MemExportKsf inner func to export mk.");
        return WSEC_ERR_KMC_KSF_VERSION_INVALID;
    }

    do {
        ret = CloneKsfMem(WSEC_FALSE, WSEC_TRUE, g_keystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ksfMem->mkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMemMk), 0, CompareMkForArr, OnRemoveMkArr);
        for (i = 0; i < exportKsfCfg->pairCount; i++) {
            domainId = (exportKsfCfg->pair + i)->domainId;
            if (SearchMkByKeyId(ksfMem, domainId, (exportKsfCfg->pair + i)->keyId) >= 0) {
                WSEC_LOG_W2("The MK(Domain=%u,KeyId=%u) already exist", domainId, (exportKsfCfg->pair + i)->keyId);
                continue;
            }
            ret = GetMkByKeyIdAndHash(g_keystore, exportKsfCfg->pair + i, &mk);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E2("Mk doesn't exist domainId %u, keyId: %u", domainId, (exportKsfCfg->pair + i)->keyId);
                break;
            }

            if (mk->mkInfo.status == KMC_KEY_STATUS_TOBEACTIVE) {
                WSEC_LOG_E("The Mk status is tobeactrive, not support to export");
                ret = WSEC_ERR_KMC_MK_NOT_SUPPORT_EXP_IMP;
                break;
            }

            if (WsecArrCloneAddOrderly(ksfMem->mkArray, mk, sizeof(KmcMemMk)) < 0) {
                WSEC_LOG_E("WsecArrCloneAddOrderly failed when export Ksf by keyIds.");
                ret = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
        }

        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (exportKsfCfg->withHw == WSEC_FALSE) {
            ksfMem->rk.rkAttributes.version = KMC_KSF_VER_V2;
        }
        ret = GenKsf(ksfMem, keystoreFile, __FUNCTION__);
    } while (0);
    (void)FreeKsfSnapshot(ksfMem);
    return ret;
}

static unsigned long CloneMkByAddType(const KmcImportKsfCfg *importKsfCfg,
    KmcKsfMem *originKsfMem, KmcKsfMem **targetKsfMem)
{
    if (importKsfCfg->importMkActionType == IMPORT_MK_ACTION_ADD) {
        return CloneKsfMem(WSEC_TRUE, WSEC_TRUE, originKsfMem, targetKsfMem);
    }
    /* Clone gkeystore under mode IMPORT_MK_ACTION_REPLACE */
    if (importKsfCfg->domainId == KMC_ALL_DOMAIN) {
        return CloneKsfMem(WSEC_FALSE, WSEC_TRUE, originKsfMem, targetKsfMem);
    } else {
        return CloneKsfMemWithoutDomain(WSEC_TRUE, WSEC_TRUE, originKsfMem, targetKsfMem, importKsfCfg->domainId);
    }
}

static unsigned long CloneMkAndAddToKsfMem(KmcImportKsfCfg *importKsfCfg, KmcKsfMem *originKsfMem,
    KmcKsfMem **targetKsfMem, WsecArray addMkArray)
{
    WSEC_ASSERT(targetKsfMem != NULL);
    unsigned long ret;
    int mkCount;
    int predictMkCount;
    mkCount = WsecArrGetCount(addMkArray);
    if (mkCount == 0) {
        WSEC_LOG_E("CloneMkAndAddToKsfMem filter keys from mem ksf, but result is zero.");
        return WSEC_ERR_KMC_FILTER_MK_COUNT_ZERO;
    }
    predictMkCount = WsecArrGetCount(originKsfMem->mkArray) + mkCount;
    if (predictMkCount > GetMkCountMax()) {
        WSEC_LOG_E2("MemImportKsf mkNum(%d) exceed  max number of MK (MAX = %d)", predictMkCount, GetMkCountMax());
        return WSEC_ERR_KMC_IMPORT_MK_NUM_OVERFLOW;
    }
    ret = CloneMkByAddType(importKsfCfg, originKsfMem, targetKsfMem);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Clone keystore failed, ret=%lu.", ret);
        return ret;
    }

    if (InitKsfMemArraySafe(targetKsfMem) != WSEC_SUCCESS) {
        return WSEC_ERR_MALLOC_FAIL;
    }
    if (importKsfCfg->importMkActionType == IMPORT_MK_ACTION_ADD) {
        return AddMkToKsmByIncremental(*targetKsfMem, addMkArray);
    }

    if (importKsfCfg->importMkActionType == IMPORT_MK_ACTION_REPLACE) {
        return AddMkToKsmByFullReplace(*targetKsfMem, addMkArray);
    }

    WSEC_LOG_E1("The import type is not support. type: %hhu .", importKsfCfg->importMkActionType);
    return WSEC_ERR_KMC_INVALID_IMPORT_TYPE;
}

static unsigned long CheckImportKsfSupportVersion(const KmcImportKsfCfg *importKsfCfg)
{
    WSEC_ASSERT(importKsfCfg != NULL);
    unsigned long ret = WSEC_SUCCESS;
    do {
        if (IsKsfV1(g_keystore->rk.rkAttributes.version) == WSEC_TRUE) {
            WSEC_LOG_E("v1 could not call MemImportKsf inner func to import mk from ksf.");
            ret = WSEC_ERR_KMC_KSF_VERSION_INVALID;
            break;
        }
        if (importKsfCfg->ksfVersion > g_keystore->rk.rkAttributes.version) {
            WSEC_LOG_E("not support import mk from high version to low version.");
            ret = WSEC_ERR_KMC_KSF_VERSION_INVALID;
            break;
        }
    } while (0);
    return ret;
}

static unsigned long FilterMkFromKsf(const char *keystoreFile, KmcImportKsfCfg *importKsfCfg, KmcKsfMem *readBuff,
    WsecArray *filterByDomainDest, WsecArray *filterByDomainTypeDest)
{
    unsigned long ret;

    do {
        ret = ReadKsf(keystoreFile, __FUNCTION__, readBuff);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MemImportKsf ReadKsf failed, result = %lu", ret);
            break;
        }
        if (readBuff->rk.rkAttributes.version != importKsfCfg->ksfVersion) {
            WSEC_LOG_E1("ksfVersion from KSF is not equal input version, ret = %lu", ret);
            ret = WSEC_ERR_KMC_KSF_VERSION_INVALID;
            break;
        }

        ret = FilterMasterKeys(readBuff->mkArray, importKsfCfg->domainId, importKsfCfg->domainType,
            filterByDomainDest, filterByDomainTypeDest);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MemImportKsf failed, ret=%lu", ret);
            break;
        }
    } while (0);

    return ret;
}

static unsigned long WriteFilterMksKsf(const KmcImportKsfCfg *importKsfCfg, KmcKsfMem *ksfMem)
{
    WSEC_ASSERT(importKsfCfg != NULL);
    WSEC_ASSERT(ksfMem != NULL);

    unsigned long ret;
    if (WsecArrGetCount(ksfMem->mkArray) == 0) {
        WSEC_LOG_E1("Can not find the keys in readBuff by Domain :%u .", importKsfCfg->domainId);
        return WSEC_ERR_KMC_MK_MISS;
    }

    ret = WriteKsfSafety(WSEC_FALSE, NULL, ksfMem, __FUNCTION__);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WriteFilterMksKsf failed, result = %lu", ret);
    }
    return ret;
}

/* Specify the keystore file, import it to the memory MK, and synchronize it to the active and standby KSFs. */
unsigned long MemImportKsf(const char *keystoreFile, KmcImportKsfCfg *importKsfCfg)
{
    unsigned long ret;
    KmcKsfMem *readBuff = NULL;
    KmcKsfMem *tempMem = NULL;
    WsecArray filterMkByDomain = NULL;
    WsecArray filterMkByDomainType = NULL;
    ret = CheckImportKsfSupportVersion(importKsfCfg);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("MemImportKsf ReadKsf failed, result = %lu", ret);
        return ret;
    }
    readBuff = (KmcKsfMem *)WSEC_MALLOC(sizeof(KmcKsfMem));
    if (readBuff == NULL) {
        WSEC_LOG_E4MALLOC(sizeof(KmcKsfMem));
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        ret = FilterMkFromKsf(keystoreFile, importKsfCfg, readBuff, &filterMkByDomain, &filterMkByDomainType);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (CompareMkArray(filterMkByDomainType, g_keystore->mkArray, importKsfCfg->domainId) == WSEC_TRUE) {
            WSEC_LOG_I("MemImportKsf compare mk array is same with g_keystore mkArray.");
            ret =  WSEC_SUCCESS;
            break;
        }
        ret = CloneMkAndAddToKsfMem(importKsfCfg, g_keystore, &tempMem, filterMkByDomainType);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MemImportKsf CloneMkAndAddToKsfMem failed, ret = %lu", ret);
            break;
        }
        ret = WriteFilterMksKsf(importKsfCfg, tempMem);
    } while (0);
    if (ret != WSEC_SUCCESS) {
        (void)FreeKsfSnapshot(tempMem);
    }
    if (ret == WSEC_SUCCESS && tempMem != NULL) {
        g_keystore = FreeKsfSnapshot(g_keystore);
        g_keystore = tempMem;
    }
    (void)FreeKsfSnapshot(readBuff);
    (void)WsecArrFinalize(filterMkByDomain);
    (void)WsecArrFinalize(filterMkByDomainType);
    return ret;
}

/* Checking the KSF */
unsigned long MemCheckKeyStore(WsecBool rewriteOnCheckFail, unsigned long *rewriteErrorCode)
{
    return CfgCheckKeyStore(rewriteOnCheckFail, rewriteErrorCode, g_keystore);
}

/* Number of KSF updates obtained from the memory */
static WsecVoid MemGetUpdateNumFromMem(WsecBool ksfNumber, WsecUint32 *updateCounter)
{
    *updateCounter = (ksfNumber == WSEC_TRUE) ? g_keystore->updateCounter : g_keystore->sharedMkUpdateCounter;
}

/* Number of KSF updates obtained from the memory */
WsecVoid MemGetKsfUpdateNumFromMem(WsecUint32 *updateCounter)
{
    MemGetUpdateNumFromMem(WSEC_TRUE, updateCounter);
}

/* Obtain the number of shared MK updates from the memory. */
WsecVoid MemGetSharedMkUpdateNumFromMem(WsecUint32 *updateCounter)
{
    MemGetUpdateNumFromMem(WSEC_FALSE, updateCounter);
}

/* Obtaining the Current KSF Version */
unsigned long MemGetKsfVersion(WsecUint16 *ksfVersion)
{
    unsigned long ret = MemCheckKsfMemAndCfg();
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    *ksfVersion = g_keystore->rk.rkAttributes.version;
    return WSEC_SUCCESS;
}
