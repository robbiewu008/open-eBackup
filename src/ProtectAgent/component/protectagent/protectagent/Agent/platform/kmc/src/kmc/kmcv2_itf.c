/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "kmcv2_itf.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv3_maskinfo.h"
#include "kmcv2_ksm.h"
#include "kmcv2_ksf.h"
#include "kmcv3_rk.h"
#include "kmcv2_cfg.h"
#include "kmcv2_pri.h"
#include "kmc_sync.h"
#include "kmc_mkf.h"
#include "kmc_utils.h"
#include "wsecv2_share.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_datetime.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_util.h"
#include "wsecv2_lock.h"
#include "wsecv2_mem.h"

#define KMC_LOG_DOMAIN_PRIVACY(domainId)                      \
WSEC_LOG_E3("DomainId(%u) is privacy(%d ~ %d)",               \
    (domainId), KMC_PRI_DOMAIN_ID_MIN, KMC_PRI_DOMAIN_ID_MAX)

#define KMC_IS_VALID_MK_FROM(domainKeyFrom) \
    WSEC_IS2(domainKeyFrom, KMC_MK_GEN_BY_INNER, KMC_MK_GEN_BY_IMPORT)

#define KMC_IS_PRI_DOMAIN(domainId) \
    WSEC_IN_SCOPE(domainId, KMC_PRI_DOMAIN_ID_MIN, KMC_PRI_DOMAIN_ID_MAX)

#define KMC_CFG_IS_ROOT_KEY_VALID(rkCfg) \
    (((rkCfg)->validity > 0) && ((rkCfg)->rmkIter > 0))

/*
 * advanceDay < MIN_RANGE_ADVANCE_DAY || advanceDay > MAX_RANGE_ADVANCE_DAY:
 *  number of days in advance for updating a key
 */
#define MIN_RANGE_ADVANCE_DAY 0
#define MAX_RANGE_ADVANCE_DAY 3650000
/* Locks the critical resources of the KMC. */
static WsecVoid ThreadLock(KmcLockOrNot lockOrNot, KmcLockType type)
{
    WSEC_ASSERT(lockOrNot == KMC_NOT_LOCK || lockOrNot == KMC_NEED_LOCK);
    WSEC_ASSERT(type == KMC_LOCK_NONE || type == KMC_LOCK_CFG || type == KMC_LOCK_KEYSTORE || type == KMC_LOCK_BOTH);
    if (lockOrNot == KMC_NOT_LOCK) {
        return;
    }
    if (((unsigned int)type & (unsigned int)KMC_LOCK_KEYSTORE) > 0) {
        WsecThreadLockById(LOCK4KEYSTORE);
    }
    if (((unsigned int)type & (unsigned int)KMC_LOCK_CFG) > 0) {
        WsecThreadLockById(LOCK4KMC_CFG);
    }
}

/* Unlock KMC critical resource */
static WsecVoid ThreadUnlock(KmcLockOrNot lockOrNot, KmcLockType type)
{
    WSEC_ASSERT(lockOrNot == KMC_NOT_LOCK || lockOrNot == KMC_NEED_LOCK);
    WSEC_ASSERT(type == KMC_LOCK_NONE || type == KMC_LOCK_CFG || type == KMC_LOCK_KEYSTORE || type == KMC_LOCK_BOTH);
    if (lockOrNot == KMC_NOT_LOCK) {
        return;
    }
    if (((unsigned int)type & (unsigned int)KMC_LOCK_KEYSTORE) > 0) {
        WsecThreadUnlockById(LOCK4KEYSTORE);
    }
    if (((unsigned int)type & (unsigned int)KMC_LOCK_CFG) > 0) {
        WsecThreadUnlockById(LOCK4KMC_CFG);
    }
}

/* Only the master node can manage KMCs. */
static WsecBool CanManageKmc(void)
{
    if (GetLockStatus() == WSEC_LOCK_UNGEN) {
        WSEC_LOG_E("KMC not running.");
        return WSEC_FALSE;
    }
    if (PriKmcSysGetRole() != KMC_ROLE_MASTER) {
        WSEC_LOG_E("Operation can only be done by Master.");
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

/* Both the master and agent can use KMC. */
static WsecBool CanUseKmc(void)
{
    if (GetLockStatus() == WSEC_LOCK_UNGEN) {
        WSEC_LOG_E("KMC not running.");
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

/* Obtains the MK based on the domain ID and key ID. */
static unsigned long GetMkDetail(WsecUint32 domainId, WsecUint32 keyId,
    KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    int expiredDays = 0;
    KmcUseExpiredMkNotify mkNotify;
    WsecSysTime nowUtc = { 0, 0, 0, 0, 0, 0, 0 };
    unsigned long errorCode;

    if (!(keyPlaintextBuff != NULL && keyBuffLen != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    (void)memset_s(&mkNotify, sizeof(mkNotify), 0, sizeof(mkNotify));
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        errorCode = MemCheckKsfMemAndCfg();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        errorCode = MemGetMkDetail(domainId, keyId, mkInfo, keyPlaintextBuff, keyBuffLen);

        /*
         * If the key has expired and the grace period has expired, a notification is sent.
         * (Best effort is used. An error does not affect the return of the function.)
         */
        if (!WsecGetUtcDateTime(&nowUtc)) {
            WSEC_LOG_E("Get UTC fail.");
            break;
        }
        if (!WsecDateTimeDiffDay(&mkInfo->mkExpiredTimeUtc, &nowUtc, &expiredDays)) {
            WSEC_LOG_E("Date Dif fail.");
            break;
        }
        mkNotify.expiredDays = (int)expiredDays;
        (void)memcpy_s(&mkNotify.expiredMkInfo, sizeof(mkNotify.expiredMkInfo), mkInfo, sizeof(KmcMkInfo));

        /*
         * This API is used to access memory configuration information.
         * It can be called only after MemCheckKsfMemAndCfg is called successfully.
         */
        if (expiredDays >= CfgGetGraceDaysAfterKeyExpired()) {
            WSEC_NOTIFY(WSEC_KMC_NTF_USING_EXPIRED_MK, &mkNotify, sizeof(KmcUseExpiredMkNotify));
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return errorCode;
}

/* Notify the app that the MK has changed. */
static WsecVoid NotifyMkChanged(const KmcMkInfo *mk, WsecUint32 type)
{
    KmcMkChangeNotify notifyData;
    if (mk == NULL) {
        return;
    }
    if (memcpy_s(&notifyData.mkInfo, sizeof(notifyData.mkInfo), mk, sizeof(KmcMkInfo)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return;
    }

    notifyData.type = type;
    WSEC_NOTIFY(WSEC_KMC_NTF_MK_CHANGED, &notifyData, sizeof(notifyData));
}

/* Notify the APP that the RK is about to expire. */
static WsecVoid NotifyRkExpire(const KmcRkAttributes *rkAttributes, int remainLifeDays)
{
    KmcRkExpireNotify notifyData;
    if (rkAttributes == NULL) {
        return;
    }
    (void)memcpy_s(&notifyData.rkAttr, sizeof(notifyData.rkAttr), rkAttributes, sizeof(KmcRkAttributes));

    notifyData.remainDays = remainLifeDays;
    WSEC_NOTIFY(WSEC_KMC_NTF_RK_EXPIRE, &notifyData, sizeof(notifyData));
}

/* Deinitializes the KMC. */
static unsigned long FinalizeKmc(KmcLockOrNot lockOrNot, WsecBool forceFinalize, WsecBool removeHardRk)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecBool isHardware = WSEC_FALSE;
    WsecUint32 state;
    ThreadLock(lockOrNot, KMC_LOCK_BOTH);
    isHardware = PriKmcSysGetIsHardware();
    state = PriKmcSysGetState();
    if (state == WSEC_RUNNING || forceFinalize == WSEC_TRUE) {
        if (removeHardRk == WSEC_TRUE) {
            MemFreeGlobalAndRemoveHardRk();
        } else {
            MemFreeGlobal();
        }
        if (isHardware == WSEC_TRUE) {
            ret = KmcHardRkUninit();
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("KmcHardRkUninit fail %lu", ret);
            }
        }
        UninitMaskCode();
        CacUnInitRng();
    }
    ThreadUnlock(lockOrNot, KMC_LOCK_BOTH);

    return ret;
}

/*
 * Re-generate the root key, but the MK remains unchanged.
 * If entropy is specified, the entropy value is also involved in the root key generation.
 */
static unsigned long UpdateRootKey(const unsigned char *entropy, WsecUint32 size, KmcLockOrNot lockOrNot)
{
    unsigned long errorCode;
    WsecUint16 ksfVersion;
    KmcRkAttributes rkAttr;

    ThreadLock(lockOrNot, KMC_LOCK_KEYSTORE);
    do {
        errorCode = MemCheckKsfMemAndCfgEx();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        MemGetRkAttr(&rkAttr);
        if (!WsecGetUtcDateTime(&rkAttr.rkExpiredTimeUtc)) {
            WSEC_LOG_E("Get current UTC failed.");
            errorCode = WSEC_ERR_GET_CURRENT_TIME_FAIL;
            break;
        }

        NotifyRkExpire(&rkAttr, 0); /* An alarm is triggered before the root key material is updated. */
        ksfVersion = rkAttr.version;
        errorCode = MemUpdateRootKey(entropy, size, ksfVersion);
    } while (0);

    ThreadUnlock(lockOrNot, KMC_LOCK_KEYSTORE);

    return errorCode;
}

/*
 * Obtain the current maximum/minimum MK ID in a specified domain. endKeyId is the obtained value.
 * endKeyId is written only when the operation is successful.
 */
static unsigned long GetEndMkId(KmcLockOrNot lockOrNot, WsecUint32 domainId, WsecUint32 *endKeyId, WsecUint32 endType)
{
    WsecUint32 maxId = 0;
    WsecUint32 minId = 0;
    unsigned long ret;

    WSEC_ASSERT(endKeyId != NULL);
    WSEC_ASSERT(WSEC_IS2(endType, KMC_MIN_KEYID, KMC_MAX_KEYID));

    ThreadLock(lockOrNot, KMC_LOCK_KEYSTORE);
    ret = MemCheckKsfMemAndCfg();
    if (ret != WSEC_SUCCESS) {
        ThreadUnlock(lockOrNot, KMC_LOCK_KEYSTORE);
        return ret;
    }

    ret = MemGetEndMkId(domainId, &minId, &maxId);
    ThreadUnlock(lockOrNot, KMC_LOCK_KEYSTORE);

    if (ret == WSEC_SUCCESS) {
        switch (endType) {
            case KMC_MIN_KEYID:
                *endKeyId = minId;
                break;
            case KMC_MAX_KEYID:
                *endKeyId = maxId;
                break;
            default:
                break;
        }
    }

    return ret;
}

/* Obtains the notification status. */
static WsecUint32 GetNotifyStatus(WsecUint32 status)
{
    /* The status parameter has been verified externally. You do not need to check it again. */
    if (status == KMC_KEY_STATUS_ACTIVE) {
        return (WsecUint32)KMC_KEY_ACTIVATED;
    } else if (status == KMC_KEY_STATUS_TOBEACTIVE) {
        return (WsecUint32)KMC_KEY_TOBEACTIVATED;
    } else {
        return (WsecUint32)KMC_KEY_INACTIVATED;
    }
}

/*
 * Obtain the MK based on the domain ID and key ID. If the MK cannot be obtained, obtain it based on the hash.
 * Ensure that the hash values are consistent.
 */
unsigned long KmcGetMkByIDHash(WsecUint32 domainId,
    WsecUint32 keyId,
    const unsigned char *hashData, WsecUint32 hashLen,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (keyPlaintextBuff == NULL || keyBuffLen == NULL || hashData == NULL || hashLen < WSEC_MK_HASH_REC_LEN) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = MemCheckKsfMemAndCfg();
    if (ret != WSEC_SUCCESS) {
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return ret;
    }
    ret = MemGetMkByIDHash(domainId, keyId, hashData, hashLen, keyPlaintextBuff, keyBuffLen);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/* 2. External Interface Functions */
/* KMC module role setting */
unsigned long KmcSetRoleType(WsecUint32 roleType)
{
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (roleType > KMC_ROLE_MASTER) {
        WSEC_LOG_E("RoleType is wrong Pls reset the role type");
        return WSEC_ERR_KMC_INVALID_ROLETYPE;
    }
    PriKmcSysSetRole(roleType); /* Specify the current identity. */
    return WSEC_SUCCESS;
}

/* Check the initial input parameters and check whether they are initialized. */
static unsigned long KmcInitilizeExPrepare(const WsecInternalInitParam *initParam)
{
    /* initParam assert outside */
    /* 1. Check input parameters. */
    if (initParam->filePathName == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS2(initParam->enableHw, WSEC_TRUE, WSEC_FALSE)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS2(initParam->hdParm.hasSoftLevelRk, WSEC_TRUE, WSEC_FALSE)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (initParam->roleType > KMC_ROLE_MASTER) {
        WSEC_LOG_E("Role type is wrong, pls reset the role type");
        return WSEC_ERR_KMC_INVALID_ROLETYPE;
    }
    if (initParam->filePathName->keyStoreFile[MASTER_KSF_INDEX] == NULL ||
        initParam->filePathName->keyStoreFile[BACKUP_KSF_INDEX] == NULL) {
        WSEC_LOG_E1("The %d KSF names must be all-provided.",
            (int)WSEC_NUM_OF(initParam->filePathName->keyStoreFile));
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS2(initParam->enableThirdBackup, WSEC_TRUE, WSEC_FALSE) ||
        (initParam->enableThirdBackup == WSEC_TRUE &&
        initParam->filePathName->keyStoreBackupFile == NULL)) {
        WSEC_LOG_E("The thrid KSF enable flag not set properly or "
                   "The third KSF names must be provided when enable Third backup.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS2(initParam->deleteKsfOnInitFailed, WSEC_TRUE, WSEC_FALSE)) {
        WSEC_LOG_E("Delete Ksf when startup cfg not set properly.");
        return WSEC_ERR_INVALID_ARG;
    }
    /* 2 Initialize or Not */
    if (PriKmcSysGetState() == WSEC_RUNNING || PriKmcSysGetState() == WSEC_ON_INIT) {
        WSEC_LOG_E("KMC is running, not initialize repeatedly before finalized.");
        return WSEC_ERR_KMC_INI_MUL_CALL;
    }
    return WSEC_SUCCESS;
}

/*
 * reserve: reserved parameter, which is not used currently.
 * Note: This function is not a direct external function and needs to be called using WsecInitializeEx.
 */
unsigned long KmcInitializeEx(const WsecInternalInitParam *initParam, KmcLockOrNot lockOrNot)
{
    WSEC_ASSERT(initParam != NULL);
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    unsigned long temp;
    ThreadLock(lockOrNot, KMC_LOCK_BOTH);
    ret = KmcInitilizeExPrepare(initParam);
    if (ret != WSEC_SUCCESS) {
        ThreadUnlock(lockOrNot, KMC_LOCK_BOTH);
        return ret;
    }
    do {
        ret = CacInitRng();
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Rng initialize failed %lu.", ret);
            ret = WSEC_ERR_SDP_RAND_INIT_FAILED;
            break;
        }
        ret = InitMaskCode();
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Mask initialize failed %lu.", ret);
            ret = WSEC_ERR_MASK_INIT_FAIL;
            break;
        }
        /* The initialization process is not complete and is returned. The initialization is in progress. */
        ret = PriKmcSysInit(WSEC_ON_INIT, initParam);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("PriKmcSysInit failed.");
            break;
        }
        CfgSetMkMaxCount(WSEC_MK_NUM_MAX);
        if (initParam->enableHw == WSEC_TRUE) {
            ret = KmcHardRkInit(initParam->hdParm.hdCtx.buff, initParam->hdParm.hdCtx.len);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("KmcHardRkInit failed %lu", ret);
                break;
            }
        }
        ret = MemLoadDataEx(PriKmcSysGetRole());
    } while (0);
    /* If the preceding operations fail, release the resources. */
    if (ret != WSEC_SUCCESS) {
        temp = FinalizeKmc(KMC_NOT_LOCK, WSEC_TRUE, WSEC_FALSE);
        WSEC_LOG_E2("KmcInitializeEx failed %lu, Finalize ret %lu.", ret, temp);
        PriKmcSysSetState(WSEC_INIT_FAIL);
    } else {
        PriKmcSysSetState(WSEC_RUNNING);
    }
    ThreadUnlock(lockOrNot, KMC_LOCK_BOTH);

    return ret;
}

/* Note: This function is not an external function and needs to be triggered by WsecFinalizeEx. */
unsigned long KmcFinalizeEx(void)
{
    unsigned long errorCode;
    errorCode = FinalizeKmc(KMC_NEED_LOCK, WSEC_FALSE, WSEC_FALSE);
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E1("FinalizeKmc fail %lu", errorCode);
    }

    return errorCode;
}

static unsigned long CloneKsfName(KmcKsfName* fileName)
{
    unsigned long ret = WSEC_SUCCESS;
    fileName->keyStoreFile[MASTER_KSF_INDEX] = WSEC_CLONE_STR(PriKmcSysGetKsf(MASTER_KSF_INDEX));
    fileName->keyStoreFile[BACKUP_KSF_INDEX] = WSEC_CLONE_STR(PriKmcSysGetKsf(BACKUP_KSF_INDEX));
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        fileName->keyStoreBackupFile = WSEC_CLONE_STR(PriKmcSysGetKsfBackupKsf());
    }
    if (fileName->keyStoreFile[MASTER_KSF_INDEX] == NULL ||
        fileName->keyStoreFile[BACKUP_KSF_INDEX] == NULL ||
        (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE && fileName->keyStoreBackupFile == NULL)) {
        WSEC_FREE(fileName->keyStoreFile[MASTER_KSF_INDEX]);
        WSEC_FREE(fileName->keyStoreFile[BACKUP_KSF_INDEX]);
        if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
            WSEC_FREE(fileName->keyStoreBackupFile);
        }
        WSEC_LOG_E("Allocate ksf memory failed.");
        ret = WSEC_ERR_MALLOC_FAIL;
    }
    return ret;
}

/* KMC module reset. This function is not an external function and needs to be triggered by WsecResetEx. */
unsigned long KmcResetEx(WsecBool isHardware, WsecBuffConst hardwareParam)
{
    KmcKsfName fileName = {{NULL}, NULL};
    unsigned long ret;
    WsecInternalInitParam initParam;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        if (PriKmcSysGetState() != WSEC_RUNNING) {
            WSEC_LOG_E("KMC not running.");
            ret = WSEC_ERR_KMC_CBB_NOT_INIT;
            break;
        }
        /* 1. Construct initialization parameters. */
        ret = CloneKsfName(&fileName);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /*
         * The KmcInitializeEx global variables role and hasSoftLevelRk must be obtained before FinalizeKmc,
         * Otherwise, the value may be changed during FinalizeKmc.
         * As a result, the parameter transferred to KmcInitializeEx is incorrect.
         * As a result, the FinalizeKmc initialization returns an error or the result is unexpected.
         */
        initParam.roleType = PriKmcSysGetRole();
        initParam.filePathName = &fileName;
        initParam.enableThirdBackup = PriKmcSysGetIsEnableThirdBackup();
        initParam.deleteKsfOnInitFailed = PriKmcSysGetIsDeleteKsfOnInitFailed();
        initParam.enableHw = isHardware;
        initParam.hdParm.hasSoftLevelRk = PriKmcSysGetHasSoftLevelRk();
        WSEC_BUFF_ASSIGN(initParam.hdParm.hdCtx, hardwareParam.buff, hardwareParam.len);

        ret = FinalizeKmc(KMC_NOT_LOCK, WSEC_FALSE, WSEC_FALSE);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("KMC Finalize ret %lu", ret);
            break;
        }
        /* 3. Initialization */
        ret = KmcInitializeEx(&initParam, KMC_NOT_LOCK);
        if (ret == WSEC_SUCCESS) {
            WSEC_LOG_I1("KmcResetEx success %d", isHardware);
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    WSEC_FREE(fileName.keyStoreFile[0]);
    WSEC_FREE(fileName.keyStoreFile[1]);
    WSEC_FREE(fileName.keyStoreBackupFile);
    return ret;
}

/* This API is used to delete a key. A key can be deleted only when it is not in the Active state. */
unsigned long KmcRmvMk(WsecUint32 domainId, WsecUint32 keyId)
{
    KmcMkInfo removeMk = { 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } };
    unsigned long ret;
    WsecBool shareDomainMkChanged = WSEC_FALSE;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemRmvMk(domainId, keyId, shareDomainMkChanged, &removeMk);
        if (ret == WSEC_SUCCESS) {
            WSEC_LOG_I2("KmcRmvMk success domain %u id %u", domainId, keyId);
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    /* Notify the app and write logs. */
    if (ret == WSEC_SUCCESS) {
        NotifyMkChanged(&removeMk, KMC_KEY_REMOVED);
        WSEC_LOG_I3("Del MK(DomainId=%u, KeyId=%u, KeyType=%u).", removeMk.domainId, removeMk.keyId, removeMk.keyType);
    }

    return ret;
}

/*
 * This API is used to register a key in a specified external import domain.
 * The caller needs to input the content of the registered key. The status of the created key is
 * KMC_KEY_STATUS_TOBEACTIVE (key that is about to take effect). To activate this key, call KmcActivateMk,
 * This function writes the keystore file after a key is added.
 * Note: When registering a key, ensure the randomness of the key.
 * Do not register a key with poor randomness. Otherwise, the key may be
 * Crack: Use functions such as SdpEncryptEx and SdpEncryptWithHmacEx
 * to carry the hash of the registration key in the ciphertext header.
 * Value used to uniquely identify a key. If the registered key is too simple,
 * the original MK may be reversely obtained from the rainbow table. recommender
 * Using secure random number as the registered key
 */
unsigned long KmcRegisterMkEx(WsecUint32 domainId,
    WsecUint32 keyId,
    const unsigned char *plaintextKey, WsecUint32 keyLen)
{
    unsigned long ret;
    WsecBool shareDomainMkChanged = WSEC_FALSE;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (KMC_IS_PRI_DOMAIN(domainId)) {
        KMC_LOG_DOMAIN_PRIVACY(domainId);
        return WSEC_ERR_KMC_CANNOT_ACCESS_PRI_DOMAIN;
    }
    if (!((plaintextKey != NULL) && (keyLen > 0) && (keyLen <= WSEC_MK_PLAIN_LEN_MAX))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemRegisterMkEx(domainId, keyId, shareDomainMkChanged, plaintextKey, keyLen);
        if (ret == WSEC_SUCCESS) {
            WSEC_LOG_I2("KmcRegisterMkEx success domain %u id %u", domainId, keyId);
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return ret;
}

static unsigned long DoKmcCreateMkEx(WsecUint32 domainId, WsecUint32 *keyId)
{
    unsigned long ret;
    WsecUint32 currentMaxKeyId;
    WsecBool shareDomainMkChanged = WSEC_FALSE;
    do {
        /*
         * This check is mandatory even if GetEndMkId is checked internally.
         * If GetEndMkId is deleted, the failure code returned by GetEndMkId may not be WSEC_ERR_KMC_MK_MISS.
         */
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = GetEndMkId(KMC_NOT_LOCK, domainId, &currentMaxKeyId, KMC_MAX_KEYID);
        /*
         * If the field is empty, WSEC_ERR_KMC_MK_MISS is returned, and keyId is set to 1.
         * Otherwise, WSEC_SUCCESS is returned.
         */
        if (ret != WSEC_SUCCESS) {
            WSEC_ASSERT(ret == WSEC_ERR_KMC_MK_MISS);
            currentMaxKeyId = 0;
        }

        ret = MemCreateMkEx(domainId, currentMaxKeyId, shareDomainMkChanged, keyId);
        if (ret == WSEC_SUCCESS) {
            WSEC_LOG_I2("KmcCreateMkEx success domain %u id %u", domainId, *keyId);
        }
    } while (0);
    return ret;
}
/*
 * This API is used to create a key in a specified internal automatic generation domain and return
 * the corresponding keyId information. The caller does not need to specify the key content.
 * the created key is in the KMC_KEY_STATUS_TOBEACTIVE state (the key is about to take effect).
 * If you want to activate it,
 * To activate the key, call KmcActivateMk. After the key is added, the keystore file is written.
 */
unsigned long KmcCreateMkEx(WsecUint32 domainId, WsecUint32 *keyId)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (keyId == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    ret = DoKmcCreateMkEx(domainId, keyId);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return ret;
}

/*
 * Update the root key. If this parameter is left blank, the root key will be written into the keystore file.
 * If this parameter is specified, the root key provided by the upper layer will be used.
 * The entropy value is used as a part of the input to update the root key.
 */
unsigned long KmcUpdateRootKey(const unsigned char *keyEntropy, WsecUint32 size)
{
    /* In V3 and later versions, the root key can be updated on both the primary and secondary nodes. */
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    return UpdateRootKey(keyEntropy, size, KMC_NEED_LOCK);
}

static unsigned long DoGetRootKeyInfo(KmcRkAttributes *rkAttr)
{
    unsigned long errorCode;
    errorCode = MemCheckKsfMemAndCfg();
    if (errorCode != WSEC_SUCCESS) {
        return errorCode;
    }
    MemGetRkAttr(rkAttr);
    return WSEC_SUCCESS;
}

/* Obtaining the Attributes of the Current Root Key */
unsigned long KmcGetRootKeyInfo(KmcRkAttributes *rkAttr)
{
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (rkAttr == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    errorCode = DoGetRootKeyInfo(rkAttr);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return errorCode;
}

/*
 * Obtains the number of keys in the current keystore.
 * This API can be used together with KmcGetMk to traverse key status.
 */
int KmcGetMkCount(void)
{
    int count;
    if (CanUseKmc() == WSEC_FALSE) {
        return -1;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return -1;
    }
    count = MemGetMkCount();
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return count;
}

/*
 * Obtain the key information in the current keystore.
 * The index starts from 0 and the upper limit is KmcGetDomainCount-1.
 * If the key is modified or deleted during the obtaining process, the key may fail to be obtained.
 * If this function is used to traverse key information, ensure
 * that other threads do not update or reload keys during the traversal,
 * In addition, do not change the status of any key or delete or add any key.
 * This function is used to obtain the mk based on the list of all keys
 * and cannot be used together with KmcGetMkCountByDomain.
 */
unsigned long KmcGetMk(int idx, KmcMkInfo *memMk)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (!((idx >= 0) && memMk != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemGetMkInfo(idx, memMk);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Obtains the number of keys in the specified domain in the current keystore.
 * This API cannot be used together with KmcGetMk.
 */
int KmcGetMkCountByDomain(WsecUint32 domainId)
{
    int mkCount;

    if (CanUseKmc() == WSEC_FALSE) {
        return -1;
    }
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return -1;
    }
    mkCount = MemGetMkCountByDomain(domainId);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return mkCount;
}

static unsigned long CheckRmvMkCount(WsecUint32 domainId, int rmvMkCount)
{
    int mkCount;
    mkCount = MemGetMkCountByDomain(domainId);
    if (mkCount <= 0) {
        WSEC_LOG_E1("Input param invalid, mk miss in domain (%u).", domainId);
        return WSEC_ERR_KMC_MK_MISS;
    }
    if (rmvMkCount <= 0) {
        WSEC_LOG_E1("Input param invalid, rmvMkCount (%d) is too small", rmvMkCount);
        return WSEC_ERR_INVALID_ARG;
    }
    /* Since mkCount >= 1, mkCount - 1 >= 0 and mkCount < 4096, rmvMkCount not greater than 4096 */
    if (rmvMkCount > mkCount - 1) {
        WSEC_LOG_E1("Input param invalid, rmvMkCount (%d) is too big", rmvMkCount);
        return WSEC_ERR_INVALID_ARG;
    }
    return WSEC_SUCCESS;
}

static WsecVoid DoRmvMkByCountNotify(KmcMkInfoArray rmvMkInfoArray)
{
    int i;
    for (i = 0; i < rmvMkInfoArray.mkCount; i++) {
        NotifyMkChanged(&rmvMkInfoArray.mkInfos[i], KMC_KEY_REMOVED);
        WSEC_LOG_I3("Del MK(DomainId=%u, KeyId=%u, KeyType=%u).",
            rmvMkInfoArray.mkInfos[i].domainId, rmvMkInfoArray.mkInfos[i].keyId, rmvMkInfoArray.mkInfos[i].keyType);
    }
}

/* Delete specific number of oldest inactive mk. */
unsigned long KmcRmvMkByCount(WsecUint32 domainId, int rmvCount, int *actualRmvedCount)
{
    unsigned long ret;
    KmcMkInfoArray rmvMkInfoArray = { NULL, 0 };
    WsecBool shareDomainMkChanged = WSEC_FALSE;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (actualRmvedCount == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        /* Only v1 master && v2v3 mastter/agent-local could delete MK */
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ret = CheckRmvMkCount(domainId, rmvCount);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* rmvMkCount not greater than 4096 */
        rmvMkInfoArray.mkInfos = (KmcMkInfo *)WSEC_MALLOC(((WsecUint32)rmvCount) * sizeof(KmcMkInfo));
        if (rmvMkInfoArray.mkInfos == NULL) {
            WSEC_LOG_E1("%s allocate rmvMkInfo mem failed.", __FUNCTION__);
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        rmvMkInfoArray.mkCount = rmvCount;
        ret = MemRmvMkByCount(domainId, shareDomainMkChanged, &rmvMkInfoArray);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    /* Notify the app and write logs. */
    if (ret == WSEC_SUCCESS) {
        *actualRmvedCount = rmvMkInfoArray.mkCount;
        DoRmvMkByCountNotify(rmvMkInfoArray);
    } else {
        *actualRmvedCount = 0;
    }
    WSEC_FREE(rmvMkInfoArray.mkInfos);
    return ret;
}

/* This interface is used to obtain the maximum key ID of a specified domain. */
unsigned long KmcGetMaxMkId(WsecUint32 domainId, WsecUint32 *maxKeyId)
{
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (maxKeyId == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    return GetEndMkId(KMC_NEED_LOCK, domainId, maxKeyId, KMC_MAX_KEYID);
}

/* Obtains the minimum key ID of the current specified domain. */
unsigned long KmcGetMinMkId(WsecUint32 domainId, WsecUint32 *minKeyId)
{
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (minKeyId == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    return GetEndMkId(KMC_NEED_LOCK, domainId, minKeyId, KMC_MIN_KEYID);
}

/*
 * This interface is used to obtain a key and its status information based on the specified hash value.
 * The creation time and expiration time in the information is based on the following rules
 * The local time is obtained by the function. The caller can determine whether the default expiration time is 180 days.
 * If multiple keys with different IDs but the same key content exist in the keystore,
 * this function obtains random keys from these keys.
 * A key
 */
unsigned long KmcGetMkDetailByHash(const unsigned char *hashData, WsecUint32 hashLen,
    KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(keyPlaintextBuff != NULL && keyBuffLen != NULL && hashData != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (hashLen != WSEC_MK_HASH_REC_LEN) {
        WSEC_LOG_E2("hashLen must be given %u, but %u, so input-buff insufficient.", WSEC_MK_HASH_REC_LEN, hashLen);
        return WSEC_ERR_KMC_INVALID_KEYHASH_LEN;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
        WSEC_LOG_E("KmcGetMkDetailByHash, KMC not running.");
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }
    ret = MemGetMkDetailByHash(hashData, hashLen, mkInfo, keyPlaintextBuff, keyBuffLen);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * This interface is used to obtain the status information about a specified key.
 * The creation time and expiration time in the information is obtained based on the local time function.
 * The caller can determine whether the validity period is 180 days by default based on the site requirements.
 * Keys with different IDs but the same key content. This function obtains a random key from these keys.
 */
unsigned long KmcGetMkInfoByContent(const unsigned char *keyPlaintextBuff, WsecUint32 keyLen, KmcMkInfo *mkInfo)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(keyPlaintextBuff != NULL && mkInfo != NULL && (keyLen <= WSEC_MK_PLAIN_LEN_MAX))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
        WSEC_LOG_E("KmcGetMkInfoByContent, KMC not running.");
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }
    ret = MemGetMkInfoByContent(keyPlaintextBuff, keyLen, mkInfo);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * This API is used to obtain the key information of a specified domain ID and key ID,
 * including the original key and basic mapping information.
 * and key status information. The creation and expiration time in the information is
 * obtained based on the local time function,
 * The caller can determine whether the validity period is 180 days by default.
 */
unsigned long KmcGetMkDetail(WsecUint32 domainId, WsecUint32 keyId, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    KmcMkInfo mk = { 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } };
    unsigned char keyPlain[WSEC_MK_LEN_MAX] = {0};
    WsecUint32 keyLen;
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(keyPlaintextBuff != NULL && keyBuffLen != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        keyLen = sizeof(keyPlain);
        errorCode = GetMkDetail(domainId, keyId, &mk, keyPlain, &keyLen);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        /* Output the key plaintext. */
        if (*keyBuffLen < keyLen) {
            WSEC_LOG_E2("Key buffer len must at least given %u, but %u", keyLen, *keyBuffLen);
            errorCode = WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
            break;
        }
        if (memcpy_s(keyPlaintextBuff, (size_t)*keyBuffLen, keyPlain, (size_t)keyLen) != EOK) {
            WSEC_LOG_E4MEMCPY;
            errorCode = WSEC_ERR_MEMCPY_FAIL;
            break;
        }
        *keyBuffLen = keyLen;

        /* Output basic MK information */
        if (mkInfo != NULL) {
            (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &mk, sizeof(mk));
        }
    } while (0);

    (void)memset_s(keyPlain, sizeof(keyPlain), 0, sizeof(keyPlain));
    return errorCode;
}

/*
 * This interface is used to obtain a valid key in a specified domain.
 * If multiple keys in a domain are valid, any of them can be obtained.
 * This function also returns the status information of the key.
 * The creation time and expiration time in the information is obtained based on the local time function,
 * The invoker can determine whether the certificate is trusted based on the site requirements.
 * The default expiration time is 180 days. If there are multiple certificates in the keystore,
 * This function is used to obtain a random key from the keys with the same domain but different IDs but in valid state.
 */
unsigned long KmcPriGetActiveMk(WsecUint32 domainId, KmcKeyTypesInfo keyTypes, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(keyPlaintextBuff != NULL && keyBuffLen != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    /* The KMC contains three types of keys. The minimum number of types is 1. */
    WSEC_ASSERT(keyTypes.typeCount > KMC_KEY_TYPE_MIN);
    /* The KMC contains three types of keys. The maximum number of types is 3. */
    WSEC_ASSERT(keyTypes.typeCount < KMC_KEY_TYPE_MAX);

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
        WSEC_LOG_E("KmcGetActiveMk, KMC not running.");
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }

    ret = MemGetActiveMk(domainId, keyTypes, mkInfo, keyPlaintextBuff, keyBuffLen);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * This interface is used to obtain a valid key in a specified domain.
 * If multiple keys in a domain are valid, any of them can be obtained.
 * This function also returns the status information of the key.
 * The creation time and expiration time in the information is obtained based on the local time function,
 * The invoker can determine whether the certificate is trusted based on the site requirements.
 * The default expiration time is 180 days. If there are multiple certificates in the keystore,
 * This function is used to obtain a random key from the keys with the same domain but different IDs but in valid state.
 */
unsigned long KmcGetActiveMk(WsecUint32 domainId,
    KmcMkInfo *mkInfo, unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen)
{
    KmcKeyTypesInfo keyTypes = { KMC_KEY_TYPE_MAX - 1,
        { KMC_KEY_TYPE_ENCRPT_INTEGRITY, KMC_KEY_TYPE_ENCRPT, KMC_KEY_TYPE_INTEGRITY} };
    return KmcPriGetActiveMk(domainId, keyTypes, mkInfo, keyPlaintextBuff, keyBuffLen);
}

/*
 * Obtains the hash value of the key based on the specified domain and key ID.
 * The hash value is the first eight bytes of the SHA256 calculation result of the plaintext key.
 */
unsigned long KmcGetMkHash(WsecUint32 domainId, WsecUint32 keyId, unsigned char *hashData, WsecUint32 *hashLen)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(hashData != NULL && hashLen != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemGetMkHash(domainId, keyId, hashData, hashLen);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Maximum number of mks. The value cannot be greater than 4096.
 * If the number of MKs reaches the upper limit,
 * KmcCreateMkEx and KmcRegisterMkEx cannot be used to create or register a master key
 */
unsigned long KmcSetMkMaxCount(int count)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(WSEC_IN_SCOPE(count, 1, WSEC_MK_NUM_MAX))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = MemCheckKsfMemAndCfgEx();
    if (ret != WSEC_SUCCESS) {
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return ret;
    }
    CfgSetMkMaxCount(count);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return WSEC_SUCCESS;
}

/*
 * This API is used to change the status of a specified key.
 * For a single key, you can change the status to any specified status,
 * Use this function with caution. If it is used improperly, multiple valid keys may exist in the same domain.
 * will be written into the keystore file.
 */
unsigned long KmcSetMkStatus(WsecUint32 domainId, WsecUint32 keyId, unsigned char status)
{
    unsigned long ret;
    KmcMkInfo notifyMkInfo;
    WsecBool needNotify = WSEC_FALSE;
    WsecBool shareDomainMkChanged = WSEC_FALSE;
    if (!(WSEC_IS3(status, KMC_KEY_STATUS_ACTIVE, KMC_KEY_STATUS_INACTIVE, KMC_KEY_STATUS_TOBEACTIVE))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    (void)memset_s(&notifyMkInfo, sizeof(KmcMkInfo), 0, sizeof(KmcMkInfo));
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemSetMkStatus(domainId, keyId, shareDomainMkChanged, status, &needNotify, &notifyMkInfo);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    if (needNotify == WSEC_TRUE) {
        NotifyMkChanged(&notifyMkInfo, GetNotifyStatus(status));
        WSEC_LOG_I3("The MK(DomainId=%u, KeyId=%u)'s status change to %hhu .", domainId, keyId, status);
    }

    return ret;
}

/* Obtains the status of the key with a specified domain ID and key ID. */
unsigned long KmcGetMkStatus(WsecUint32 domainId, WsecUint32 keyId, unsigned char *status)
{
    unsigned long ret;

    if (status == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemGetMkStatus(domainId, keyId, status);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

static unsigned long DoKmcActivateMk(WsecUint32 domainId, WsecUint32 keyId)
{
    unsigned long ret;
    WsecArray changedMkArray = NULL;
    int i;
    KmcMkInfo mkInfoNotify;
    int mkChangedMkCount;
    KmcMkInfo *mkInfo = NULL;
    WsecBool shareDomainMkChanged = WSEC_FALSE;

    (void)memset_s(&mkInfoNotify, sizeof(mkInfoNotify), 0, sizeof(mkInfoNotify));
    do {
        ret = MemCheckKsfMemAndCfgDomain(domainId, &shareDomainMkChanged);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* Create a linked list to save the current key in the same domain. */
        changedMkArray = WsecArrInitialize(0, (WsecUint32)sizeof(KmcMkInfo), 0, NULL, WsecArrStdRemoveElement);
        if (changedMkArray == NULL) {
            WSEC_LOG_E("Array Initialize fail.");
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        ret = MemActivateMk(domainId, keyId, shareDomainMkChanged, changedMkArray, &mkInfoNotify);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        NotifyMkChanged(&mkInfoNotify, KMC_KEY_ACTIVATED);
        WSEC_LOG_I2("The MK (DomainId=%u, KeyId=%u) is activated, and other keys of this domain is deactivated.",
            mkInfoNotify.domainId, mkInfoNotify.keyId);
        mkChangedMkCount = WsecArrGetCount(changedMkArray);
        for (i = 0; i < mkChangedMkCount; i++) {
            mkInfo = (KmcMkInfo *)WsecArrGetAt(changedMkArray, i);
            if (mkInfo == NULL || mkInfo->keyId == keyId) {
                continue;
            }
            mkInfo->status = KMC_KEY_STATUS_INACTIVE;
            NotifyMkChanged(mkInfo, KMC_KEY_INACTIVATED);
        }
    } while (0);

    (void)WsecArrFinalize(changedMkArray);
    return ret;
}

/* Activates a specified key. That is, the specified key is valid and all other keys in the same domain are invalid. */
unsigned long KmcActivateMk(WsecUint32 domainId, WsecUint32 keyId)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = DoKmcActivateMk(domainId, keyId);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/*
 * The default configuration of the root key in the KMC is as follows
 * The validity period is 3650 days, and the number of iterations is 10000.
 * This function can be used to modify the default rootkey configuration.
 * The modification takes effect after the function is called and takes effect when the current process exits.
 * The configuration of also becomes invalid.
 * This function can be called before WsecInitializeEx to set the default rootkey configuration.
 * After this function is set, it is valid for the root key generated during subsequent initialization.
 * If the application calls this function, it does not call this function any more.
 * If the configuration of KmcSetRootKeyCfg is modified, the invocation of this function is
 * also valid for updating the root key. If after initialization,
 * KmcSetRootKeyCfg needs to be called to modify the rootkey configuration in state.
 * When the WsecResetEx function is called, this function is used to set
 * The default configuration of still takes effect.
 * However, the configurations generated by other configuration functions are invalid and need to be reconfigured.
 * Note: The greater the number of iterations, the more difficult the brute force cracking is theoretically.
 * This function does not verify the upper limit of rkCfg iterations,
 * However, if the number of iterations is too large, the key derivation time is long.
 * Therefore, you must limit the maximum number of iterations based on the site requirements.
 * Otherwise, DoS attacks may occur.
 */
unsigned long KmcSetDefaultRootKeyCfg(const KmcCfgRootKey *rkCfg)
{
    /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    int temp;
    if (rkCfg == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* Check the validity of the parameter content. */
    if (!KMC_CFG_IS_ROOT_KEY_VALID(rkCfg)) {
        WSEC_LOG_E("'validity' or 'rmkIter' invalid");
        return WSEC_ERR_INVALID_ARG;
    }

    temp = (int)rkCfg->validity;
    if (temp < 1 || temp > KMC_KEYTYPE_MAX_LIFEDAYSEX) {
        WSEC_LOG_E1("'validity'(%d) is not correct.", temp);
        return WSEC_ERR_INVALID_ARG;
    }

    CfgSetDefaultRootKeyCfg(rkCfg);
    return WSEC_SUCCESS;
}

/* Sets the rootkey. The setting takes effect only after the rootkey is updated. */
unsigned long KmcSetRootKeyCfg(const KmcCfgRootKey *rkCfg)
{
    unsigned long errorCode;
    /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    int temp;

    if (rkCfg == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    /* Check the validity of the parameter content. */
    if (!KMC_CFG_IS_ROOT_KEY_VALID(rkCfg)) {
        WSEC_LOG_E("'validity' or 'rmkIter' invalid");
        return WSEC_ERR_INVALID_ARG;
    }

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    temp = (int)rkCfg->validity;
    if (temp < 1 || temp > KMC_KEYTYPE_MAX_LIFEDAYSEX) {
        WSEC_LOG_E1("'validity'(%d) is not correct.", temp);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        errorCode = MemCheckKsfMemAndCfgEx();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        CfgSetRootKeyCfg(rkCfg);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    if (errorCode == WSEC_SUCCESS) {
        WSEC_LOG_I("New Rootkey Config set.");
    }

    return errorCode;
}

/* Obtains the current settings. */
unsigned long KmcGetRootKeyCfg(KmcCfgRootKey *rkCfg)
{
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (rkCfg == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        errorCode = MemCheckKsfMemAndCfg();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        CfgGetRootKeyCfg(rkCfg);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);

    return errorCode;
}

/*
 * Refresh the MK mask in the memory. The MK is protected by the mask in the memory.
 * The mask needs to be refreshed periodically. You are advised to enable the cyclic function.
 * Timer. After initialization, this function is called periodically.
 * It is recommended that this function be called once an hour.
 */
unsigned long KmcRefreshMkMaskEx(void)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemRefreshMkMaskEx();
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/*
 * Erase all keys, including the key information in the keystore file and memory,
 * for board recycling. Exercise caution when performing this operation.
 */
unsigned long KmcSecureEraseKeystore(void)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = MemCheckKsfMemAndCfgEx();
    if (ret != WSEC_SUCCESS) {
        ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
        return ret;
    }
    ret = SecureEraseKeystore();
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    if (ret == WSEC_SUCCESS) {
        WSEC_LOG_I("Erase keystore.");
        ret = FinalizeKmc(KMC_NEED_LOCK, WSEC_FALSE, WSEC_TRUE);
    }

    return ret;
}

/* Obtains the currently effective key, key ID, and hash value in a specified domain. */
unsigned long KmcGetActiveMkWithHash(WsecUint32 domainId, unsigned char *keyBuff, WsecUint32 *keyBuffLen,
    WsecUint32 *keyId, unsigned char *keyHash, size_t hashLen)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (!(keyBuff != NULL && keyBuffLen != NULL && keyId != NULL && keyHash != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ret = MemGetActiveMkWithHash(domainId, keyBuff, keyBuffLen, keyId, keyHash, hashLen);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/* Add Domain Configuration */
unsigned long KmcAddDomainEx(const KmcCfgDomainInfo *domainInfo)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (domainInfo == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (KMC_IS_PRI_DOMAIN(domainInfo->domainId)) {
        KMC_LOG_DOMAIN_PRIVACY(domainInfo->domainId);
        return WSEC_ERR_KMC_CANNOT_ACCESS_PRI_DOMAIN;
    }
    if (!KMC_IS_VALID_MK_FROM(domainInfo->domainKeyFrom)) {
        WSEC_LOG_E("'domainKeyFrom' invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemAddDomainEx(domainInfo);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    return ret;
}

/* Delete the domain configuration. */
unsigned long KmcRmvDomainEx(WsecUint32 domainId)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (KMC_IS_PRI_DOMAIN(domainId)) {
        KMC_LOG_DOMAIN_PRIVACY(domainId);
        WSEC_LOG_E("KmcRmvDomainEx domainId may not privacy domain");
        return WSEC_ERR_KMC_CANNOT_ACCESS_PRI_DOMAIN;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CfgRmvDomainEx(domainId);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    return ret;
}

/*
 * This command is used to add the configuration of a specified domain key type.
 * Only the KMC_KEY_TYPE_ENCRPT_INTEGRITY key type can be added.
 */
unsigned long KmcAddDomainKeyTypeEx(WsecUint32 domainId, const KmcCfgKeyType *keyTypeCfg)
{
    int temp; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (KMC_IS_PRI_DOMAIN(domainId)) {
        KMC_LOG_DOMAIN_PRIVACY(domainId);
        return WSEC_ERR_KMC_CANNOT_ACCESS_PRI_DOMAIN;
    }
    if (keyTypeCfg == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (keyTypeCfg->keyType != KMC_KEY_TYPE_ENCRPT_INTEGRITY) {
        WSEC_LOG_E1("Input keyTypeCfg->keyType(%u) invalid.", keyTypeCfg->keyType);
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IN_SCOPE(keyTypeCfg->keyLen, 1, WSEC_MK_PLAIN_LEN_MAX)) {
        WSEC_LOG_E1("Input keyTypeCfg->keyLen invalid. it must not over %u", WSEC_MK_PLAIN_LEN_MAX);
        return WSEC_ERR_INVALID_ARG;
    }
    temp = (int)keyTypeCfg->keyLifeDays;
    if ((temp < 1) || (temp > KMC_KEYTYPE_MAX_LIFEDAYSEX)) {
        WSEC_LOG_E1("'KeyLifeDays'(%d) is not proper.", temp);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CfgAddDomainKeyTypeEx(domainId, keyTypeCfg);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    return ret;
}

/* Delete the key type configuration of a specified domain. */
unsigned long KmcRmvDomainKeyTypeEx(WsecUint32 domainId, WsecUint16 keyType)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (KMC_IS_PRI_DOMAIN(domainId)) {
        KMC_LOG_DOMAIN_PRIVACY(domainId);
        WSEC_LOG_E("KmcRmvDomainKeyTypeEx domainId may not privacy domain");
        return WSEC_ERR_KMC_CANNOT_ACCESS_PRI_DOMAIN;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CfgRmvDomainKeyTypeEx(domainId, keyType);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    return ret;
}

/* Obtains the number of current domains. If an error occurs, a negative number is returned. */
int KmcGetDomainCount(void)
{
    int count;
    if (CanUseKmc() == WSEC_FALSE) {
        return -1;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        if (MemCheckKsfMemAndCfg() != WSEC_SUCCESS) {
            count = -1;
            break;
        }
        count = CfgGetDomainCount();
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);

    return count;
}

/*
 * Obtains the domain configuration information based on idx. This parameter is used together with KmcGetDomainCount.
 * The value range of idx is [0, KmcGetDomainCount - 1].
 * Note: If this function is used to traverse the domain information,
 * ensure that other threads do not update or reload the domain during the traversal,
 * Do not change the status of any domain, or delete or add a domain.
 */
unsigned long KmcGetDomain(int idx, KmcCfgDomainInfo *domainInfo)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (!((idx >= 0) && domainInfo != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_CFG);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CfgGetDomain(idx, domainInfo);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_CFG);

    return ret;
}

/*
 * Generate a new keystore file that contains all the current master keys of all domains.
 * The file name is specified by the product.
 * Note: Do not use the keystore specified when the process initializes the KMC as the file name.
 */
unsigned long KmcGenerateKsfAll(const char *keystoreFile)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (WSEC_IS_EMPTY_STRING(keystoreFile)) {
        WSEC_LOG_E("KmcGenerateKsfAll input a null string");
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemGenerateKsfAll(keystoreFile);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Generate a keystore file based on the specified domain.
 * The keystore file contains the current master key in the domain. The file name is specified by the product.
 * Note: Do not use the keystore specified when the process initializes the KMC as the file name.
 */
unsigned long KmcGenerateKsfByDomain(WsecUint32 domainId, const char *keystoreFile)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (WSEC_IS_EMPTY_STRING(keystoreFile)) {
        WSEC_LOG_E("KmcGenerateKsfByDomain input a null string");
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemGenerateKsfByDomain(domainId, keystoreFile);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Regenerates a KSF. This function does not change any root key or master key,
 * but only rewrites the memory key to two KSF files.
 * For example, the TPM can invoke this function to write the KMC data protected by the new authorization value to
 * the KSF file after the data authorization value is changed.
 */
unsigned long KmcReGenerateKsf(void)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemReGenerateKsf();
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Stores the KSF to a specified version (V1 or V2).
 * Note
 * 1. This interface is used only when KSF V3 needs to be downgraded to KSF V2 or KSF V1 after the upgrade.
 * 2. Do not perform any key management operation after the interface is invoked and before the downgrade is complete.
 * Otherwise, the MK will be lost after the downgrade.
 * 3. It is recommended that KmcSecureEraseKeystore be invoked immediately after this interface is invoked to
 * clear the KSF V3 file to prevent information leakage.
 * 4. KMC 3.0 is upgraded and the hardware root key feature is used. The KSF version is V3, KSF V2 (KMC V2), or KSF V1.
 * (KMC V1) is incompatible. In this scenario, this function must be called to save the KSF file as the source version,
 * Otherwise, KMC 3.0 KSF V3 cannot be parsed after the downgrade.
 * 5. If the source version is KMC V2 and the deployment mode is multi-node or multi-process master/client,
 * after the upgrade
 * The local domain key cannot be planned because the local domain key is available only in KMC 3.0.
 * After the downgrade, the local domain keys of the master and client are different,
 * Synchronizing the KSF will overwrite the local domain key of the Agent. As a result,
 * the service data cannot be decrypted.
 */
unsigned long KmcGenerateKsfAs(WsecUint16 ksfVersion, const char *ksfName)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (WSEC_IS_EMPTY_STRING(ksfName)) {
        WSEC_LOG_E("KmcGenerateKsfAs input a null string");
        return WSEC_ERR_INVALID_ARG;
    }
    if (ksfVersion != KMC_KSF_VER && ksfVersion != KMC_KSF_VER_V2) {
        WSEC_LOG_E1("KmcGenerateKsfAs version %hu", ksfVersion);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ret = MemGenerateV1V2Ksf(ksfVersion, ksfName);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return ret;
}

/* Exports a memory key to a specified KSF file. */
unsigned long KmcExportKsf(const char *keystoreFile, KmcExportKsfCfg *exportKsfCfg)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (keystoreFile == NULL || exportKsfCfg == NULL) {
        WSEC_LOG_E("KmcExportKsf export param is NULL.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (KMC_DOMAINID_OUT_OF_RANGE(exportKsfCfg->domainId)) {
        WSEC_LOG_E("KmcExportKsf keystore domainId arg invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    ret = MemExportKsf(keystoreFile, exportKsfCfg);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return ret;
}

/* Exports special MKs filter by domainIds and keyIds to keystore file.
 * Note
 * 1. It will be return fail directly if one of MK exported failure.
 * 2. The MK with status TOBEACTIVE is not allowed to export, Since TOBEACTIVE MK is not used for encryption.
 * 3. If the elements of input param pair in exportKsfCfg is duplicate, duplicate elements will be deduplicated.
 * 4. DO NOT USE THIS INTERFACE IN V1 AND STREAM DATA ENCRYPTION, SINCE THE CIPHER HEADER DOES NOT CONTAIN MK HASH
 */
unsigned long KmcExportKsfByKeys(const char *keystoreFile, const KmcExportKsfByKeysCfg *exportKsfCfg)
{
    unsigned long ret;
    int i;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (keystoreFile == NULL || exportKsfCfg == NULL || exportKsfCfg->pair == NULL || exportKsfCfg->pairCount == 0) {
        WSEC_LOG_E("KmcExportKsfByKeyIds export param is invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    if (exportKsfCfg->pairCount > WSEC_MK_NUM_MAX) {
        WSEC_LOG_E1("KmcExportKsfByKeyIds export keyIds exceed max limit,paircout=%d", exportKsfCfg->pairCount);
        return WSEC_ERR_INVALID_ARG;
    }
    for (i = 0; i < exportKsfCfg->pairCount; i++) {
        if (KMC_DOMAINID_OUT_OF_RANGE((exportKsfCfg->pair + i)->domainId)) {
            WSEC_LOG_E("KmcExportKsfByKeyIds keystore domainId arg invalid.");
            return WSEC_ERR_INVALID_ARG;
        }
        if (((exportKsfCfg->pair + i)->hashData != NULL && (exportKsfCfg->pair + i)->hashLen != WSEC_MK_HASH_REC_LEN) ||
            ((exportKsfCfg->pair + i)->hashLen == 0 && (exportKsfCfg->pair + i)->hashData != NULL)) {
            WSEC_LOG_E1("KmcExportKsfByKeyIds hashLen=%u or hashdata not matching", (exportKsfCfg->pair + i)->hashLen);
            return WSEC_ERR_INVALID_ARG;
        }
    }
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    ret = MemExportByKeys(keystoreFile, exportKsfCfg);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);

    return ret;
}

/* Specify the keystore file, import it to the memory MK, and synchronize it to the active and standby KSFs.
 * Notes
 * 1. Currently, KMC support two import types, IMPORT_MK_ACTION_REPLACE and IMPORT_MK_ACTION_ADD,
 * The type IMPORT_MK_ACTION_REPLACE will fully replace memory MKs with Mks in keystoreFile, and IMPORT_MK_ACTION_ADD
 * will incrementally add MKs in keystoreFile to memory without overwrite the origin memory MKs.
 * 2. You can import special domain or all domain in keystoreFile. For all domain import, you can set domainid equals
 * KMC_ALL_DOMAIN.
 * 3. You can also import MKs filter by domainType,domainType can value in KMC_DOMAIN_TYPE_SHARE,KMC_DOMAIN_TYPE_LOCAL,
 * KMC_DOMAIN_TYPE_IGNORE. Both share and local domain will be import if the type set to KMC_DOMAIN_TYPE_IGNORE.
 * It will treat as shared domain MKs if the domain of the MKs is not create in KMC. So the MKs whose domain is not
 * create will not import to memory keystore if domain type set to LOCAL, but the Mks will import to memory if domain
 * type set to SHARE or IGNORE.
 */
unsigned long KmcImportKsf(const char *keystoreFile, KmcImportKsfCfg *importKsfCfg)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (keystoreFile == NULL || importKsfCfg == NULL) {
        WSEC_LOG_E("KmcImportKsf importKsfCfg param is NULL.");
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS3(importKsfCfg->domainType, KMC_DOMAIN_TYPE_SHARE, KMC_DOMAIN_TYPE_LOCAL, KMC_DOMAIN_TYPE_IGNORE)) {
        WSEC_LOG_E1("KmcImportKsf invalid domainType, domainType:%hhu", importKsfCfg->domainType);
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WSEC_IS2(importKsfCfg->importMkActionType, IMPORT_MK_ACTION_REPLACE, IMPORT_MK_ACTION_ADD)) {
        WSEC_LOG_E1("KmcImportKsf invalid import mk action type, type:%hhu.", importKsfCfg->importMkActionType);
        return WSEC_ERR_INVALID_ARG;
    }
    if (KMC_DOMAINID_OUT_OF_RANGE(importKsfCfg->domainId)) {
        WSEC_LOG_E("KmcImportKsf keystore domainId arg invalid.");
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    ret = MemImportKsf(keystoreFile, importKsfCfg);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_BOTH);
    return ret;
}

/*
 * Check the integrity of the keystore file. This operation can be performed only on the Master node.
 * Check whether the keystore file is complete. If the file is damaged,
 * You can use the rewriteErrorCode parameter to determine whether to rewrite the keystore.
 * Check whether the rewriting is successful.
 * Note: Rewriting will cause the original key data in the rewritten keystore to be lost,
 * historical data cannot be decrypted, and the keystore cannot be used.
 * The cause of file integrity damage cannot be located.
 *  If the application specifies rewriting, the rewriting logic is as follows
 * (1) If a keystore file is valid, overwrite it with a valid one.
 * (2) If the two files are invalid, rewrite the two files using the memory MK.
 */
unsigned long KmcCheckKeyStore(WsecBool rewriteOnCheckFail, unsigned long *rewriteErrorCode)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (rewriteOnCheckFail == WSEC_TRUE) {
        if (rewriteErrorCode == NULL) {
            return WSEC_ERR_INVALID_ARG;
        } else {
            *rewriteErrorCode = WSEC_SUCCESS;
        }
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemCheckKeyStore(rewriteOnCheckFail, rewriteErrorCode);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/* Obtains the number of iterations for updating the keystore in the current memory. */
unsigned long KmcGetUpdateNumFromMem(WsecUint32 *updateCounter)
{
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (updateCounter == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        errorCode = MemCheckKsfMemAndCfg();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        MemGetKsfUpdateNumFromMem(updateCounter);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return errorCode;
}

/* Obtains the number of update iterations of a specified ksf file. */
unsigned long KmcGetUpdateNumFromFile(const char *keystoreFile, WsecUint32 *updateCounter)
{
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (!(updateCounter != NULL && keystoreFile != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    errorCode = GetKsfUpdateNumberFromKeystore(keystoreFile, updateCounter);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return errorCode;
}

/* Obtains the number of iterations for updating the shared key in the current memory. */
unsigned long KmcGetSharedMkUpdateNumFromMem(WsecUint32 *updateCounter)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (updateCounter == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        MemGetSharedMkUpdateNumFromMem(updateCounter);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/* Obtains the number of iterations for updating the shared key of a specified ksf file. */
unsigned long KmcGetSharedMkUpdateNumFromFile(const char *keystoreFile, WsecUint32 *updateCounter)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (!(updateCounter != NULL && keystoreFile != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = GetSharedMkUpdateNumberFromKsf(keystoreFile, updateCounter);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Export all the current MKs to a file encrypted using the key derived from the password for remote backup.
 * Note: The greater the number of iterations, the more difficult the brute force cracking is theoretically.
 * This function does not verify the upper limit of the number of iterations, but
 * If the generation times are too large, the key derivation time will be long.
 * Therefore, you must limit the maximum generation times based on the site requirements,
 * Otherwise, DoS attacks may occur.
 */
unsigned long KmcExportMkFileEx(WsecUint16 mkfVersion,
    const char *destFile,
    const unsigned char *password, WsecUint32 passwordLen,
    WsecUint32 iter)
{
    unsigned long ret;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (destFile == NULL || password == NULL || passwordLen == 0) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (mkfVersion != KMC_MKF_VER_V2 && mkfVersion != KMC_MKF_VER) {
        WSEC_LOG_E1("Mkf version invalid, mkfVersion%hu", mkfVersion);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    /* 3. Construct data and write data to a file. */
    do {
        ret = MemCheckKsfMemAndCfgEx();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemExportMkFileEx(mkfVersion, destFile, password, passwordLen, iter);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/*
 * Import all MKs using the password. The imported MKs overwrite the original keystore files in the memory.
 * The keystore files are used for remote backup encryption.
 * Restore the local keystore file. Exercise caution when performing this operation.
 */
unsigned long KmcImportMkFileEx(const char *fromFile, const unsigned char *password, WsecUint32 passwordLen)
{
    unsigned long errorCode;

    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    if (!(fromFile != NULL && password != NULL && (passwordLen > 0))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        errorCode = MemCheckKsfMemAndCfgEx();
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        /* Opening a file */
        errorCode = MemImportMkFileEx(fromFile, password, passwordLen);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return errorCode;
}

/* Master send specific shared domain to Agent */
unsigned long KmcMasterSendMkByDomain(WsecUint32 domainId, WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    unsigned long ret;
    /* Only the master clock can send synchronization packets. */
    if (CanManageKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    if (sendSyncData == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ret = CheckInputDomainIdAndType(domainId);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E2("invalid domain:%u, errno:%lu", domainId, ret);
        return ret;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemMasterSendMkByDomain(domainId, param, sendSyncData);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/* MASTER: indicates that the AGENT sends all shared master keys. */
unsigned long KmcMasterSendAllMk(WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    return KmcMasterSendMkByDomain(KMC_ALL_DOMAIN, param, sendSyncData);
}

static unsigned long AgentRecvMkByDomain(WsecUint32 recvMode,
    WsecUint32 *outDomainId, WsecVoid *param, CallbackRecvSyncData recvSyncData)
{
    unsigned long ret;
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    /* Only the agent can receive the message. */
    if (PriKmcSysGetRole() != KMC_ROLE_AGENT) {
        return WSEC_ERR_KMC_INVALID_ROLETYPE;
    }

    if (recvSyncData == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = MemCheckKsfMemAndCfg();
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MemAgentRecvMkByDomain(recvMode, outDomainId, param, recvSyncData);
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/* Agent Recv special shared MK from master by domain */
unsigned long KmcAgentRecvMkByDomain(WsecUint32 *domainId, WsecVoid *param, CallbackRecvSyncData recvSyncData)
{
    return AgentRecvMkByDomain(KMC_PARTIAL_DOMAIN, domainId, param, recvSyncData);
}

/* The AGENT receives all shared master keys from the master. */
unsigned long KmcAgentRecvAllMk(WsecVoid *param, CallbackRecvSyncData recvSyncData)
{
    return AgentRecvMkByDomain(KMC_ALL_DOMAIN, NULL, param, recvSyncData);
}

/* Checking and Updating the Root Key */
unsigned long KmcAutoUpdateRk(int updateDaysBefore)
{
    unsigned long ret;
    int day;
    WsecSysTime nowUTC;
    WsecBool dateDiffRet;
    KmcRkAttributes rkAttributes;
    if (updateDaysBefore < 0) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }
    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        ret = DoGetRootKeyInfo(&rkAttributes);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        dateDiffRet = WsecGetUtcDateTime(&nowUTC);
        if (dateDiffRet == WSEC_FALSE) {
            ret = WSEC_ERR_GET_CURRENT_TIME_FAIL;
            WSEC_LOG_E("Get current UTC failed.");
            break;
        }
        dateDiffRet = WsecDateTimeDiffDay(&nowUTC, &rkAttributes.rkExpiredTimeUtc, &day);
        if (dateDiffRet == WSEC_FALSE) {
            WSEC_LOG_E("Date diff failed.");
            ret = WSEC_ERR_CALC_DIFF_DAY_FAIL;
            break;
        }
        if (day <= updateDaysBefore) {
            ret = UpdateRootKey(NULL, 0, KMC_NOT_LOCK);
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);

    return ret;
}

/* Check whether each MK in a domain expires. If the MK expires at the latest time, the MK expires. */
static unsigned long CheckWhetherHasMkToBeUpdated(WsecUint32 domainId, const WsecSysTime *nowUTC,
    KmcMkInfo *mkInfo, int *expireRemainday)
{
    int day = 0;
    int diffDay;
    WsecBool firstAssign = WSEC_TRUE;
    WsecBool hasMk = WSEC_FALSE;
    unsigned long ret = WSEC_SUCCESS;
    WsecBool boolRet;
    KmcMkInfo tempMkInfo;
    int keyCount;
    int i;
    keyCount = MemGetMkCount();
    for (i = 0; i < keyCount; i++) {
        ret = MemGetMkInfo(i, &tempMkInfo);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (tempMkInfo.domainId != domainId) {
            continue;
        }
        hasMk = WSEC_TRUE;

        boolRet = WsecDateTimeDiffDay(nowUTC, &tempMkInfo.mkExpiredTimeUtc, &diffDay);
        if (boolRet == WSEC_FALSE) {
            WSEC_LOG_E("cannot get the diff days");
            ret = WSEC_ERR_CALC_DIFF_DAY_FAIL;
            break;
        }
        if (firstAssign == WSEC_TRUE || (firstAssign == WSEC_FALSE && diffDay > day)) {
            day = diffDay;
            firstAssign = WSEC_FALSE;
            (void)memcpy_s(mkInfo, sizeof(KmcMkInfo), &tempMkInfo, sizeof(KmcMkInfo));
        }
    }

    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    if (hasMk == WSEC_FALSE) {
        return WSEC_ERR_KMC_MK_MISS;
    }
    /* Check whether the latest MK in the domain expires. */
    *expireRemainday = day;
    return ret;
}

/* Search for and check the domain type matching status. */
static unsigned long CheckWhetherDomainMkWillUpdate(WsecUint32 domainId, KmcMkInfo *mkInfo, int *expireRemainDay)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecSysTime nowUTC;
    WsecBool dateTimeRet;
    KmcCfgDomainInfo tempDomainInfo;
    int domainCount = CfgGetDomainCount();
    int i;
    dateTimeRet = WsecGetUtcDateTime(&nowUTC);
    if (dateTimeRet == WSEC_FALSE) {
        WSEC_LOG_E("Get current UTC failed.");
        return WSEC_ERR_GET_CURRENT_TIME_FAIL;
    }

    for (i = 0; i < domainCount; i++) {
        ret = CfgGetDomain(i, &tempDomainInfo);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (tempDomainInfo.domainId == domainId) {
            ret = CheckWhetherHasMkToBeUpdated(domainId, &nowUTC, mkInfo, expireRemainDay);
            break;
        }
    }
    return ret;
}

/* Automatically check whether each MK in the imported domain expires. */
unsigned long KmcAutoCheckDomainLatestMk(WsecUint32 domainId, int advanceDay, WsecBool *hasMkToBeUpdated,
    KmcMkInfo *mkInfo, int *expireRemainDay)
{
    unsigned long ret;
    int remainDay = advanceDay + 1;
    if (mkInfo == NULL || hasMkToBeUpdated == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    /* The number of days in advance to update cannot be greater than 0. */
    if (advanceDay < MIN_RANGE_ADVANCE_DAY || advanceDay > MAX_RANGE_ADVANCE_DAY) {
        /* The maximum value for preventing overflow is 3650000. */
        WSEC_LOG_E1("Advance update days (%d) can not smaller than zero or larger than 3650000.", advanceDay);
        return WSEC_ERR_INVALID_ARG;
    }
    if (CanUseKmc() == WSEC_FALSE) {
        return WSEC_ERR_INVALID_CALL_SEQ;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    do {
        *hasMkToBeUpdated = WSEC_FALSE;
        ret = MemCheckKsfMemAndCfgDomain(domainId, NULL);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CheckWhetherDomainMkWillUpdate(domainId, mkInfo, &remainDay);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (expireRemainDay != NULL) {
            *expireRemainDay = remainDay;
        }
        if (remainDay <= advanceDay) {
            *hasMkToBeUpdated = WSEC_TRUE;
        }
    } while (0);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/* Obtains the current system UTC. */
WsecBool KmcGetUtcDateTime(WsecSysTime *nowUtc)
{
    WsecBool ret = WSEC_FALSE;
    if (nowUtc == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return ret;
    }

    if (CanUseKmc() == WSEC_FALSE) {
        return ret;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = WsecGetUtcDateTime(nowUtc);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

/* Obtains the number of days between two time points. */
WsecBool KmcDateTimeDiffDay(const WsecSysTime *startTime, const WsecSysTime *endTime, int *day)
{
    WsecBool ret = WSEC_FALSE;
    if (startTime == NULL || endTime == NULL || day == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return ret;
    }

    if (CanUseKmc() == WSEC_FALSE) {
        return ret;
    }

    ThreadLock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    ret = WsecDateTimeDiffDay(startTime, endTime, day);
    ThreadUnlock(KMC_NEED_LOCK, KMC_LOCK_KEYSTORE);
    return ret;
}

