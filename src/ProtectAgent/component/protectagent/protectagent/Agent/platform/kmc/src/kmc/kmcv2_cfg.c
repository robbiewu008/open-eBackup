/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component (key management component) - CFG
 * Author: x00102361
 * Create: 2014-06-16
 * Notes: Anti-collision access protection must be implemented for global variables g_keystore and g_kmcCfg.
 * The protection mechanism is as follows: Public functions are locked and unlocked,
 * and private functions are not locked and unlocked.
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * On March 19, 2019, Zhang Jie (employee ID: 00316590) split the original kmcv2_itf.c file into
 * kmcv2_ksf.c/kmcv2_ksm.c/kmcv2_itf.c/kmcv2_cfg.c to meet the requirements of the 2000-line file.
 *                     ITF: interface
 *                     KSM: keystore memory
 *                     KSF: keystore file
 *                     MKF: MK file
 */

#include "kmcv2_cfg.h"
#include "securec.h"
#include "wsecv2_datetime.h"
#include "kmcv2_ksf.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"

static int g_maxMkCount = WSEC_MK_NUM_MAX;

static KmcCfgRootKey g_kmcDefaultRkCfg = { DEFAULT_ROOT_KEY_VALIDITY, DEFAULT_ROOT_KEY_RMK_ITERATION, {0} };

static KmcCfg *g_kmcCfg = NULL;

#define KMC_WARNING_BEFORE_KEY_EXPIRED_DAYS 30
#define KMC_GRACEDAYS_FOR_USE_EXPIRED_KEY   60
#define KMC_AUTO_UPDATE_KEY_TIME_MINUTE     10
#define KMC_PWD_DEFAULT_ITERATION_COUNT     2000

/* Releases g_kmcCfg. */
static KmcCfg *FreeKmcCfg(KmcCfg *kmcCfg)
{
    if (kmcCfg != NULL) {
        kmcCfg->domainCfgArray = WsecArrFinalize(kmcCfg->domainCfgArray);
        WSEC_FREE(kmcCfg);
    }
    return NULL;
}

int GetMkCountMax(void)
{
    return g_maxMkCount;
}

/* Checking the KSF */
unsigned long CfgCheckKeyStore(WsecBool rewriteOnCheckFail, unsigned long *rewriteErrorCode, KmcKsfMem *ksfMem)
{
    return KsfCheckKeyStore(rewriteOnCheckFail, g_kmcCfg, rewriteErrorCode, ksfMem);
}

/* Load the KSF. If the KSF does not exist, create it. */
unsigned long CfgReadKsfSafety(WsecUint32 role, KmcKsfMem **keystore)
{
    return ReadKsfSafety(role, g_kmcCfg, keystore);
}

/* Updating the KSF Root Key */
unsigned long CfgUpdateRootKey(const unsigned char *entropy, WsecUint32 size,
    KmcKsfMem *ksfMem, WsecUint16 ksfVersion)
{
    return KsfUpdateRootKey(entropy, size, g_kmcCfg, ksfMem, ksfVersion);
}

/* Whether the configuration is initialized */
WsecBool CfgIsCfgValid(void)
{
    return (g_kmcCfg == NULL) ? WSEC_FALSE : WSEC_TRUE;
}

/* Check whether the memory configuration information is ready. */
static unsigned long CfgCheckCfg(void)
{
    if (PriKmcSysGetState() != WSEC_RUNNING) {
        WSEC_LOG_E("KMC not running.");
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }
    if (g_kmcCfg == NULL) {
        WSEC_LOG_E("The config memory does not exist");
        return WSEC_ERR_KMC_KEYCFGMEM_NOTEXIST;
    }
    return WSEC_SUCCESS;
}

/* Search for Domain in the KMC configuration information. */
static KmcDomainCfg *SearchDomain(const KmcCfg *kmcCfg, WsecUint32 domainId)
{
    KmcDomainCfg domainInfo;

    if (kmcCfg == NULL) {
        return NULL;
    }
    domainInfo.domainInfo.domainId = domainId;
    return (KmcDomainCfg *)WsecArrBinarySearch(kmcCfg->domainCfgArray, &domainInfo);
}

/* Obtains the domain type. If only a failure is returned, the possible cause is that the domain does not exist. */
unsigned long CfgGetDomainType(WsecUint32 domain, unsigned char *domainType)
{
    KmcDomainCfg *domainConfig = NULL;
    WSEC_ASSERT(domainType != NULL);
    domainConfig = SearchDomain(g_kmcCfg, domain);
    if (domainConfig == NULL) {
        WSEC_LOG_E1("Domain %u not found.", domain);
        return WSEC_ERR_KMC_DOMAIN_MISS;
    }
    *domainType = domainConfig->domainInfo.domainType;
    return WSEC_SUCCESS;
}

/*
 * Callback function used to compare the sizes of two elements
 * when the KmcDomainCfg array is sorted or searched quickly.
 */
static int CompareDomainForArr(const WsecVoid *p1, const WsecVoid *p2)
{
    const KmcDomainCfg *domainA = NULL;
    const KmcDomainCfg *domainB = NULL;

    WSEC_ASSERT(p1 != NULL);
    WSEC_ASSERT(p2 != NULL);

    domainA = (const KmcDomainCfg *)(*(const WsecVoid * const *)p1);
    domainB = (const KmcDomainCfg *)(*(const WsecVoid * const *)p2);

    if (domainA->domainInfo.domainId > domainB->domainInfo.domainId) {
        return WSEC_CMP_RST_BIG_THAN;
    }

    if (domainA->domainInfo.domainId < domainB->domainInfo.domainId) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    return WSEC_CMP_RST_EQUAL;
}

/* When the KmcDomainCfg array element is deleted, the data stored in the element is processed. */
static WsecVoid OnRemoveDomainArr(WsecVoid *element, WsecUint32 elementSize)
{
    KmcDomainCfg *data = NULL;

    WSEC_ASSERT(elementSize == sizeof(KmcDomainCfg));

    data = (KmcDomainCfg *)element;
    if (data != NULL) {
        data->keyTypeCfgArray = WsecArrFinalize(data->keyTypeCfgArray);
        WSEC_CLEAR_FREE(data, elementSize);
    }
}

/* Callback function for comparing the sizes of two elements
 * when the KmcCfgKeyType array is quickly sorted or searched. */
static int CmpDomainKeyTypeForArr(const WsecVoid *p1, const WsecVoid *p2)
{
    const KmcCfgKeyType *keyTypeA = NULL;
    const KmcCfgKeyType *keyTypeB = NULL;

    WSEC_ASSERT(p1 != NULL);
    WSEC_ASSERT(p2 != NULL);
    /* If the scope is not changed, the PCLint reports alarm 838. */
    keyTypeA = (const KmcCfgKeyType *)(*(const WsecVoid * const *)p1);
    /* If the scope is not changed, the PCLint reports alarm 838. */
    keyTypeB = (const KmcCfgKeyType *)(*(const WsecVoid * const *)p2);
    if (keyTypeA->keyType > keyTypeB->keyType) {
        return WSEC_CMP_RST_BIG_THAN;
    }

    if (keyTypeA->keyType < keyTypeB->keyType) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    return WSEC_CMP_RST_EQUAL;
}

/* Ready to add */
static unsigned long AddDomainToArrayPrepare(KmcCfg *kmcCfg)
{
    int count;
    if (kmcCfg->domainCfgArray != NULL) {
        count = WsecArrGetCount(kmcCfg->domainCfgArray);
        if (count >= WSEC_DOMAIN_NUM_MAX) {
            WSEC_LOG_E2("DomainNum (%d) cannot over %d", count, WSEC_DOMAIN_NUM_MAX);
            return WSEC_ERR_KMC_DOMAIN_NUM_OVERFLOW;
        }
    } else {
        kmcCfg->domainCfgArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcDomainCfg), 0,
            CompareDomainForArr, OnRemoveDomainArr);
        if (kmcCfg->domainCfgArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() domaincfg array init failed.");
            return WSEC_ERR_OPER_ARRAY_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/* Add domain information to the KMC configuration information. */
static unsigned long AddDomainToArray(KmcCfg *kmcCfg, const KmcCfgDomainInfo *domainInfo)
{
    KmcDomainCfg *element = NULL;
    unsigned long ret;

    if (!(kmcCfg != NULL && domainInfo != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = AddDomainToArrayPrepare(kmcCfg);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* Constructs the elements to be added to the array. */
    element = (KmcDomainCfg *)WSEC_MALLOC(sizeof(KmcDomainCfg));
    if (element == NULL) {
        WSEC_LOG_E4MALLOC(sizeof(KmcDomainCfg));
        return WSEC_ERR_MALLOC_FAIL;
    }

    do {
        element->keyTypeCfgArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcCfgKeyType), 0,
            CmpDomainKeyTypeForArr, WsecArrStdRemoveElement);
        if (element->keyTypeCfgArray == NULL) {
            WSEC_LOG_E("WsecArrInitialize() domain key type array init failed.");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        (void)memcpy_s(&element->domainInfo, sizeof(element->domainInfo), domainInfo, sizeof(KmcCfgDomainInfo));

        /* Check uniqueness */
        if (WsecArrBinarySearch(kmcCfg->domainCfgArray, element) != NULL) {
            WSEC_LOG_E1("The Domain (ID=%u) already existed.", domainInfo->domainId);
            ret = WSEC_ERR_KMC_ADD_REPEAT_DOMAIN;
            break;
        }

        /* Add to Array */
        if (WsecArrAddOrderly(kmcCfg->domainCfgArray, element) < 0) {
            WSEC_LOG_E("WsecArrAddOrderly failed.");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
    } while (0);

    if (ret != WSEC_SUCCESS) {
        element->keyTypeCfgArray = WsecArrFinalize(element->keyTypeCfgArray);
        WSEC_FREE(element);
    }

    return ret;
}

/* Construct the default configuration of key life cycle management. */
static WsecVoid MakeKeyManageDefaultCfg(KmcCfgKeyManagement *kmCfg)
{
    if (kmCfg == NULL) {
        return;
    }

    kmCfg->warningBeforeKeyExpiredDays = KMC_WARNING_BEFORE_KEY_EXPIRED_DAYS;
    kmCfg->graceDaysForUseExpiredKey = KMC_GRACEDAYS_FOR_USE_EXPIRED_KEY;
    kmCfg->keyAutoUpdate = WSEC_TRUE;
    kmCfg->autoUpdateKeyTime.kmcHour = 1;
    kmCfg->autoUpdateKeyTime.kmcMinute = KMC_AUTO_UPDATE_KEY_TIME_MINUTE;
    kmCfg->autoUpdateKeyTime.kmcWeek = KMC_TIME_SUNDAY;
}

/* Adding the KeyType Configuration to the Domain Configuration */
static unsigned long AddDomainKeyTypeToArray(const KmcCfg *kmcCfg, WsecUint32 domainId, const KmcCfgKeyType *keyType)
{
    KmcDomainCfg *domainCfg = NULL;
    WsecArray keyTypeArray = NULL;
    KmcCfgKeyType *element = NULL;
    int count;
    if (!(kmcCfg != NULL && keyType != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    /* 1. Check whether the domain exists. */
    domainCfg = SearchDomain(kmcCfg, domainId);
    if (domainCfg == NULL) {
        WSEC_LOG_E1("The Domain(ID=%u) not exist", domainId);
        return WSEC_ERR_KMC_DOMAIN_MISS;
    }

    keyTypeArray = domainCfg->keyTypeCfgArray;
    WSEC_ASSERT(keyTypeArray != NULL);

    /* 2. The number of key types in each domain cannot exceed the maximum. */
    count = WsecArrGetCount(keyTypeArray);
    if (count >= WSEC_DOMAIN_KEY_TYPE_NUM_MAX) {
        WSEC_LOG_E2("Each Domain's KeyType num(%d) cannot over %u", count, WSEC_DOMAIN_KEY_TYPE_NUM_MAX);
        return WSEC_ERR_KMC_KEYTYPE_NUM_OVERFLOW;
    }

    /* 3. Ensure that the key type in the domain is unique. */
    if (WsecArrBinarySearch(keyTypeArray, keyType) != NULL) {
        WSEC_LOG_E2("The KeyType(DomainId=%u, KeyType=%u) already existed.", domainId, keyType->keyType);
        return WSEC_ERR_KMC_ADD_REPEAT_KEY_TYPE;
    }

    /* 4. Put in the array. */
    element = (KmcCfgKeyType *)WSEC_CLONE_BUFF(keyType, sizeof(KmcCfgKeyType));
    if (element == NULL) {
        WSEC_LOG_E("WSEC_CLONE_BUFF() failed.");
        return WSEC_ERR_MEMCLONE_FAIL;
    }

    /* Inserting failed. The cloned data needs to be released. */
    if (WsecArrAddOrderly(keyTypeArray, element) < 0) {
        WSEC_FREE(element);
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Default domains and their key types */
static WsecBool MakeDomainDefaultCfg(KmcCfg *kmcCfg)
{
    /*
     * A: If the default domain 0 is a local domain, one key is generated by default.
     * In the upgrade scenario, the original key cannot be shared by default after the upgrade.
     * If default domain 0 is a shared domain, one key is generated by default.
     * In the upgrade scenario, after the upgrade:
     * In the i single-server and single-process scenario, set this parameter to M (MASTER) after the upgrade.
     * In this case, keys can be managed and used properly.
     * ii. In the MA (MASTER+AGENT) scenario, A (AGENT) can be used, and MA can manage and use keys.
     * Problem: After the upgrade, A does not continue to share KSFs because of
     * V3 deployment requirements.A creates KSFs by itself,
     * By default, a new key is generated in shared domain 0. In this case, if the key of shared domain 0
     * is used by A before M is aligned with the key of shared domain 0 before the upgrade, A uses the new key.
     * If the key of domain 0 is encrypted or decrypted, the key of domain 0 created by domain A will be overwritten
     * after the key of domain 0 is aligned with the key of the version earlier than M,
     * As a result, the data encrypted by A using its own key cannot be decrypted.
     * If default domain C 0 is a shared domain, one key is generated by default.
     * In this case, the new device scenario is as follows:
     * In the i single-node system single-process scenario, set this parameter to M (MASTER).
     * In this case, keys can be managed and used.
     * ii. MA scenario: A can be used, and M can be managed and used.
     * Problem: After the upgrade, A generates a key separately. If A uses the self-created key of domain 0
     * to perform encryption before M aligns with M to generate domain 0, then
     * After M aligns with M to generate a key and sends it to A, A cannot decrypt the data encrypted using its own key.
     * Considering scenario A, the following method is used for scenario B and scenario C: Domains 0
     * and 1 are shared domains by default. However, the agent does not generate keys by default,
     * and the master generates keys by default.
     */
    KmcCfgDomainInfo domainCfg[] = {
        { 0, KMC_MK_GEN_BY_INNER, "Generate MK by KMC", KMC_DOMAIN_TYPE_SHARE, {0} },
        { 1, KMC_MK_GEN_BY_IMPORT, "Register MK by APP", KMC_DOMAIN_TYPE_SHARE, {0} }
    };
    KmcCfgKeyType keyTypeCfg = { KMC_KEY_TYPE_ENCRPT_INTEGRITY, DEFAULT_KEY_LEN, DEFAULT_KEY_LIFE_DAYS, {0} };
    size_t i;
    unsigned long returnValue;
    /* 1. Default domain configuration */
    for (i = 0; i < WSEC_NUM_OF(domainCfg); i++) {
        returnValue = AddDomainToArray(kmcCfg, &domainCfg[i]);
        if (returnValue != WSEC_SUCCESS) {
            WSEC_LOG_E1("AddDomainToArray failed : %lu.", returnValue);
            WsecArrRemoveAll(kmcCfg->domainCfgArray);
            return WSEC_FALSE;
        }
    }

    /* 2. Default key type configuration in the domain */
    if (AddDomainKeyTypeToArray(kmcCfg, domainCfg[0].domainId, &keyTypeCfg) != WSEC_SUCCESS) {
        return WSEC_FALSE;
    }
    if (AddDomainKeyTypeToArray(kmcCfg, domainCfg[1].domainId, &keyTypeCfg) != WSEC_SUCCESS) {
        return WSEC_FALSE;
    }

    return WSEC_TRUE;
}

/* Constructing Default Data Protection Configurations */
static WsecVoid MakeDataProtectCfg(WsecUint32 type, KmcCfgDataProtect *cfg)
{
    if (cfg == NULL) {
        return;
    }

    if (type == SDP_ALG_ENCRPT) {
        cfg->algId = WSEC_ALGID_AES128_CBC;
        cfg->keyType = KMC_KEY_TYPE_ENCRPT_INTEGRITY;
        cfg->appendMac = WSEC_TRUE;
        cfg->keyIterations = 0;
    } else if (type == SDP_ALG_INTEGRITY) {
        cfg->algId = WSEC_ALGID_HMAC_SHA256;
        cfg->keyType = KMC_KEY_TYPE_ENCRPT_INTEGRITY;
        cfg->appendMac = WSEC_FALSE;
        cfg->keyIterations = 0;
    } else if (type == SDP_ALG_PWD_PROTECT) {
        cfg->algId = WSEC_ALGID_PBKDF2_HMAC_SHA256;
        cfg->keyType = 0;
        cfg->appendMac = 0;
        cfg->keyIterations = KMC_PWD_DEFAULT_ITERATION_COUNT;
    }
}

/* Construct the default root key configuration. */
static WsecVoid MakeRootKeyDefaultCfg(KmcCfgRootKey *rkCfg)
{
    if (rkCfg == NULL) {
        return;
    }
    rkCfg->validity = g_kmcDefaultRkCfg.validity;
    rkCfg->rmkIter = g_kmcDefaultRkCfg.rmkIter;
}

/*
 * KMC configuration data initialization: Obtain the KMC configuration from the app through the callback function.
 * If a configuration item fails to be obtained, the default configuration is used.
 */
unsigned long CfgDataInitEx(void)
{
    KmcCfgRootKey *rkCfg = NULL;
    KmcCfgKeyManagement *kmCfg = NULL;
    KmcCfgDataProtect *dataProtectCfg = NULL;
    WsecUint32 algType[] = { SDP_ALG_ENCRPT, SDP_ALG_INTEGRITY, SDP_ALG_PWD_PROTECT };
    WsecUint32 i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcCfg *kmcCfg = NULL;
    kmcCfg = (KmcCfg *)WSEC_MALLOC(sizeof(KmcCfg));
    if (kmcCfg == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* 1. Configure RootKey. */
    rkCfg = &kmcCfg->rkCfg;
    MakeRootKeyDefaultCfg(rkCfg);

    /* 2. Key life cycle management parameters */
    kmCfg = &kmcCfg->keyManagementCfg;
    MakeKeyManageDefaultCfg(kmCfg);

    /* 3. DataProtection Configuration */
    dataProtectCfg = kmcCfg->dataProtectCfg;
    for (i = 0; i < WSEC_NUM_OF(algType); i++, dataProtectCfg++) {
        MakeDataProtectCfg(algType[i], dataProtectCfg);
    }

    /* 4. Domain configuration */
    /* Use the default domain and its key type. */
    if (MakeDomainDefaultCfg(kmcCfg) == WSEC_FALSE) {
        WSEC_LOG_E("Make DefaultCfg4Domain failed.");
        (void)FreeKmcCfg(kmcCfg);
        return WSEC_ERR_KMC_CALLBACK_KMCCFG_FAIL;
    }
    g_kmcCfg = kmcCfg;
    return WSEC_SUCCESS;
}

/* Obtaining the Grace Period of an Expired Key */
int CfgGetGraceDaysAfterKeyExpired(void)
{
    return g_kmcCfg->keyManagementCfg.graceDaysForUseExpiredKey;
}

/* Search for the domain configuration and key type configuration based on domainId and keyType. */
WsecBool CfgSearchDomainKeyTypeCfg(WsecUint32 domainId, WsecUint16 keyType, KmcDomainCfg **domain,
    KmcCfgKeyType **keyTypeResult)
{
    KmcDomainCfg findDomain;
    KmcCfgKeyType findKeyType;
    KmcDomainCfg *foundDomain = NULL;
    unsigned long ret;

    ret = CfgCheckCfg();
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("CfgCheckCfg failed, ret %lu", ret);
        return WSEC_FALSE;
    }
    /* Find the domain. */
    (void)memset_s(&findDomain, sizeof(KmcDomainCfg), 0, sizeof(KmcDomainCfg));
    findDomain.domainInfo.domainId = domainId;
    foundDomain = (KmcDomainCfg *)WsecArrBinarySearch(g_kmcCfg->domainCfgArray, &findDomain);
    if (foundDomain == NULL) {
        return WSEC_FALSE;
    }
    if (domain != NULL) {
        *domain = foundDomain;
    }
    if (keyTypeResult == NULL) {
        return WSEC_TRUE;
    }

    /* You also need to query the key type in the domain. */
    (void)memset_s(&findKeyType, sizeof(KmcCfgKeyType), 0, sizeof(KmcCfgKeyType));
    findKeyType.keyType = keyType;
    *keyTypeResult = (KmcCfgKeyType *)WsecArrBinarySearch(foundDomain->keyTypeCfgArray, &findKeyType);

    return (*keyTypeResult != NULL);
}

/* Releases g_kmcCfg. */
void CfgFreeKmcCfg(void)
{
    g_kmcCfg = FreeKmcCfg(g_kmcCfg);
}

/* Setting the Maximum Number of Master Keys */
void CfgSetMkMaxCount(int count)
{
    g_maxMkCount = count;
    WSEC_LOG_I1("CfgSetMkMaxCount count %d", count);
}

/* Setting the Default Root Key */
WsecVoid CfgSetDefaultRootKeyCfg(const KmcCfgRootKey *rkCfg)
{
    (void)memcpy_s(&g_kmcDefaultRkCfg, sizeof(KmcCfgRootKey), rkCfg, sizeof(KmcCfgRootKey));
    WSEC_LOG_I("New default rootkey config set successfully.");
}

/* Setting the Root Key */
WsecVoid CfgSetRootKeyCfg(const KmcCfgRootKey *rkCfg)
{
    (void)memcpy_s(&g_kmcCfg->rkCfg, sizeof(KmcCfgRootKey), rkCfg, sizeof(KmcCfgRootKey));
}

/* Obtaining the Root Key Configuration */
WsecVoid CfgGetRootKeyCfg(KmcCfgRootKey *rkCfg)
{
    (void)memcpy_s(rkCfg, sizeof(KmcCfgRootKey), &g_kmcCfg->rkCfg, sizeof(KmcCfgRootKey));
}

/* Adding a Domain to the Memory */
unsigned long CfgAddDomainEx(const KmcCfgDomainInfo *domainInfo, const KmcKsfMem *ksfMem)
{
    unsigned long errorCode = WSEC_SUCCESS;
    int i;
    int mkCount;
    KmcMemMk *memMk = NULL;

    do {
        /* Check whether there are residual MKs in the domain. */
        mkCount = WsecArrGetCount(ksfMem->mkArray);
        for (i = 0; i < mkCount; i++) {
            memMk = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, i);
            if (memMk == NULL) {
                WSEC_LOG_E("MK array memory access failed.");
                errorCode = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
            /* The MK arrays are sorted by DomainId in ascending order. */
            if (memMk->mkInfo.domainId > domainInfo->domainId) {
                break;
            }
            if (memMk->mkInfo.domainId != domainInfo->domainId) {
                continue;
            }
            if (memMk->mkInfo.generateType != domainInfo->domainKeyFrom) {
                WSEC_LOG_E2("domainKeyFrom error (DomainId=%u, KeyId=%u)", memMk->mkInfo.domainId, memMk->mkInfo.keyId);
                errorCode = WSEC_ERR_KMC_ADD_DOMAIN_DISCREPANCY_MK;
                break;
            }
        }
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        errorCode = AddDomainToArray(g_kmcCfg, domainInfo);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I3("New Domain (ID=%u keysfrom=%u domainType=%hhu) added.",
            domainInfo->domainId, domainInfo->domainKeyFrom, domainInfo->domainType);
    } while (0);

    return errorCode;
}

/* Deleting a Domain from the Memory */
unsigned long CfgRmvDomainEx(WsecUint32 domainId)
{
    int idx;
    KmcDomainCfg domainInfo;
    unsigned long ret = WSEC_SUCCESS;

    (void)memset_s(&domainInfo, sizeof(KmcDomainCfg), 0, sizeof(KmcDomainCfg));

    do {
        domainInfo.domainInfo.domainId = domainId;
        idx = WsecArrBinarySearchAt(g_kmcCfg->domainCfgArray, &domainInfo);
        if (idx < 0) {
            WSEC_LOG_W1("The domain(Id=%u) not existed", domainId);
            ret = WSEC_ERR_KMC_DOMAIN_MISS;
            break;
        }

        WsecArrRemoveAt(g_kmcCfg->domainCfgArray, idx);
        WSEC_LOG_I1("Domain (ID=%u) deleted.", domainId);
    } while (0);
    return ret;
}

/* Adding a Master Key Type to a Specified Domain */
unsigned long CfgAddDomainKeyTypeEx(WsecUint32 domainId, const KmcCfgKeyType *keyTypeCfg)
{
    KmcDomainCfg *domain = NULL;
    KmcCfgKeyType *tempKeyType = NULL;
    unsigned long ret;

    do {
        if (CfgSearchDomainKeyTypeCfg(domainId, keyTypeCfg->keyType, &domain, &tempKeyType) == WSEC_TRUE) {
            WSEC_LOG_E2("The KeyType (DomainId=%u, KeyType=%u) already exist.", domainId, keyTypeCfg->keyType);
            ret = WSEC_ERR_KMC_ADD_REPEAT_KEY_TYPE;
            /*
             * The break code is missing in SPC002. Although AddDomainKeyTypeToArray is used to
             * determine whether the Add type is repeated,
             * There is no problem even if no result is returned,
             * However, the error code is returned.
             */
            break;
        }

        ret = AddDomainKeyTypeToArray(g_kmcCfg, domainId, keyTypeCfg);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I2("Domain keytype (domainID=%u, keytype=%u) added.", domainId, keyTypeCfg->keyType);
    } while (0);
    (void)domain;
    (void)tempKeyType;
    return ret;
}

/* Deleting a Master Key from a Specified Domain */
unsigned long CfgRmvDomainKeyTypeEx(WsecUint32 domainId, WsecUint16 keyType)
{
    KmcDomainCfg findDomain;
    KmcCfgKeyType findKeyType = { 0, 0, 0, {0} };
    KmcDomainCfg *domainCfg = NULL;
    int idx;
    unsigned long ret = WSEC_SUCCESS;

    (void)memset_s(&findDomain, sizeof(KmcDomainCfg), 0, sizeof(KmcDomainCfg));

    do {
        /* Querying a Domain */
        findDomain.domainInfo.domainId = domainId;
        idx = WsecArrBinarySearchAt(g_kmcCfg->domainCfgArray, &findDomain);
        if (idx < 0) {
            WSEC_LOG_E1("The Domain(Id=%u) not exist.", domainId);
            ret = WSEC_ERR_KMC_DOMAIN_MISS;
            break;
        }

        domainCfg = (KmcDomainCfg *)WsecArrGetAt(g_kmcCfg->domainCfgArray, idx);
        if (domainCfg == NULL) {
            WSEC_LOG_E("memory access fail.");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }

        /* Querying the KeyType Position in a Domain */
        findKeyType.keyType = keyType;
        idx = WsecArrBinarySearchAt(domainCfg->keyTypeCfgArray, &findKeyType);
        if (idx < 0) {
            WSEC_LOG_E2("The KeyType(DomainId=%u, KeyType=%u) not exist.", domainId, keyType);
            ret = WSEC_ERR_KMC_DOMAIN_KEYTYPE_MISS;
            break;
        }

        /* Delete */
        WsecArrRemoveAt(domainCfg->keyTypeCfgArray, idx);
        WSEC_LOG_I2("Domain keytype (domainID=%u, keytype=%u) deleted.", domainId, keyType);
    } while (0);
    return ret;
}

/* Obtaining the Number of Master Keys */
int CfgGetDomainCount(void)
{
    return WsecArrGetCount(g_kmcCfg->domainCfgArray);
}

/* Obtains the execution index domain info. */
unsigned long CfgGetDomain(int idx, KmcCfgDomainInfo *domainInfo)
{
    KmcDomainCfg *domainCfg = NULL;
    unsigned long ret = WSEC_SUCCESS;

    do {
        if (idx >= WsecArrGetCount(g_kmcCfg->domainCfgArray)) {
            ret = WSEC_ERR_INVALID_ARG;
            break;
        }

        domainCfg = (KmcDomainCfg *)WsecArrGetAt(g_kmcCfg->domainCfgArray, idx);
        if (domainCfg == NULL) {
            WSEC_LOG_E("memory access fail.");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }

        (void)memcpy_s(domainInfo, sizeof(KmcCfgDomainInfo), &domainCfg->domainInfo, sizeof(KmcCfgDomainInfo));
    } while (0);

    return ret;
}
