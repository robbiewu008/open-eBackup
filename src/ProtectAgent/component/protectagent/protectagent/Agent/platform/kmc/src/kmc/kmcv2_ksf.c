/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component
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
 */

#include "kmcv2_ksf.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv3_maskinfo.h"
#include "kmcv3_rk.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_hash.h"
#include "wsecv2_hmac.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_file.h"
#include "wsecv2_order.h"
#include "wsecv2_lock.h"
#include "wsecv2_mem.h"
#include "wsecv2_util.h"
#include "kmc_utils.h"

static const unsigned char g_ksfFlag[KMC_FLAG_LENGTH] = {
    0x5F, 0x64, 0x97, 0x8D, 0x19, 0x4F, 0x89, 0xCF, 0xA8, 0x3F, 0x8E, 0xE1, 0xDB, 0x01, 0x3C, 0x0C,
    0x88, 0x42, 0x4A, 0x1C, 0xB7, 0xFC, 0xAD, 0x70, 0x4E, 0x45, 0x13, 0xA5, 0x14, 0x46, 0x71, 0x6C
};

#define KMC_MAX_HARDWAREKEY_INFO_LEN        WSEC_MAX_HARD_CIPHERTEXT_LEN
#define KMC_MAX_SOFTLEVELRK_CIPHERTEXT_LEN  WSEC_MAX_HARD_CIPHERTEXT_LEN
#define KMC_MAX_MK_RECORD_LEN               WSEC_MAX_HARD_CIPHERTEXT_LEN
#define KMC_RMK_DOUBLE_LEN  ((KMC_RMK_LEN) * 2) /* EK+IK */
/* current KSF max size * 2 */
#define KMC_MAX_DELETE_KSF_LENGTH \
    ((KMC_FLAG_LENGTH + sizeof(KmcKsfRk) + ((WSEC_MK_NUM_MAX) * sizeof(KmcKsfMk)) + sizeof(KmcKsfHmac)) * 2)

static unsigned long RewriteOnCheckFail(KmcKsfMem * const *readBuff,
    WsecUint32 blockSize,
    const KmcCfg *kmcCfg,
    KmcKsfMem *ksfMem,
    unsigned long *returnValue);

static unsigned long WriteKsf(const KmcKsfMem *ksfMem,
    const char *file,
    const KmcKsfRk *rkNew,
    KmcKsfRk *rkWrite,
    const WsecBuff *ksfHash, const char *callBy);

/* Adding a Process Lock to the keystore File */
static WsecVoid ProcLockKeystore(void)
{
    WsecProcLockById(PROCLOCK4KEYSTORE);
}

/* Unlock the keystore file. */
static WsecVoid ProcUnlockKeystore(void)
{
    WsecProcUnlockById(PROCLOCK4KEYSTORE);
}

/* Hand over the CPU to execute upper-layer transactions. */
static WsecVoid DoEvent(WsecUint32 counter)
{
    if (counter != 0 && (counter % WSEC_EVENT_PERIOD) == 0) {
        WSEC_DO_EVENTS;
    }
}

/* Converts the byte order of the MK information in the KSF. */
static WsecVoid CvtByteOrderForKsfMk(KmcKsfMk *memMk, WsecUint32 direction, WsecUint16 ksfVersion)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (memMk == NULL) {
        return;
    }

    memMk->v2orV3.mkV2.mkRear.plaintextLen = WSEC_BYTE_ORDER_CVT_L(direction, memMk->v2orV3.mkV2.mkRear.plaintextLen);
    /* This field is added in KSF V2.If this field is also converted in KSF V1, the host sequence is verified in KSF V1
     * However, this field is not converted back to the host sequence in KSF V1. As a result, the verification fails
     */
    if (IsKsfV2(ksfVersion) == WSEC_TRUE || IsKsfV3(ksfVersion) == WSEC_TRUE) {
        memMk->cipherAlgId = WSEC_BYTE_ORDER_CVT_L(direction, memMk->cipherAlgId);
    }
    /* This field is not converted in KMC V1. */
    if (ksfVersion != KMC_KSF_VER) {
        memMk->ciphertextLen = WSEC_BYTE_ORDER_CVT_L(direction, memMk->ciphertextLen);
    }
    CvtByteOrderForMkInfo(&memMk->mkInfo, direction);
}

/* Checking the KSF */
unsigned long KsfCheckKeyStore(WsecBool rewriteOnCheckFail,
    const KmcCfg *kmcCfg,
    unsigned long *rewriteErrorCode,
    KmcKsfMem *ksfMem)
{
    KmcKsfMem *readBuff[KMC_KSF_WITH_THIRD_NUM] = {NULL};
    unsigned long returnValue = WSEC_SUCCESS;
    unsigned long temp;
    WsecUint32 i;
    KmcKsfOneksfCorruptNotify corruptNotify = {0};
    WsecUint32 fileCount = KMC_KSF_NUM;

    ProcLockKeystore();
    do {
        /* Apply for resources. */
        /* DTS2017042406832: The requested memory is not released after the application fails. */
        /* Apply for resources. */
        if (AllocMultiBlock(readBuff, sizeof(KmcKsfMem), KMC_KSF_WITH_THIRD_NUM) == WSEC_FALSE) {
            returnValue = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        /* 2. Read files A and B. */
        if (PriKmcSysGetState() != WSEC_RUNNING) {
            returnValue = WSEC_ERR_KMC_CBB_NOT_INIT;
            break;
        }

        if (rewriteOnCheckFail == WSEC_TRUE) {
            *rewriteErrorCode = RewriteOnCheckFail(readBuff, KMC_KSF_WITH_THIRD_NUM, kmcCfg, ksfMem, &returnValue);
            WSEC_LOG_I2("end rewriteOncheckFail, errorCode:%lu, returnValue:%lu,", *rewriteErrorCode, returnValue);
            break;
        }
        /* rewriteOnCheckFail is WSEC_FALSE */
        readBuff[0]->fromFile = PriKmcSysGetKsf(MASTER_KSF_INDEX);
        readBuff[1]->fromFile = PriKmcSysGetKsf(BACKUP_KSF_INDEX);
        if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
            readBuff[KMC_KSF_NUM]->fromFile = PriKmcSysGetKsfBackupKsf();
            fileCount++;
        }
        for (i = 0; i < fileCount; i++) {
            temp = ReadKsf(readBuff[i]->fromFile, __FUNCTION__, readBuff[i]);
            WSEC_LOG_I2("Read Ksf index %u result:%lu", i, temp);
            if (temp == WSEC_SUCCESS) {
                continue;
            }
            returnValue = temp;
            if (rewriteOnCheckFail == WSEC_FALSE) {
                WSEC_LOG_W1("Read KSF :%u failed, send notify", i);
                corruptNotify.keyStoreFile = readBuff[i]->fromFile;
                WSEC_NOTIFY(WSEC_KMC_NTF_ONE_KSF_CORRUPT, &corruptNotify, sizeof(corruptNotify));
            }
        }
    } while (0);
    ProcUnlockKeystore();

    FreeKsfMemArraySnapshot(readBuff, KMC_KSF_WITH_THIRD_NUM);
    return returnValue;
}

/* Reads data except V3 KSF HMAC. */
static unsigned long ReadKsfData(WsecHandle keystore, WsecHandle hashCtx, WsecVoid *buff, WsecUint32 len)
{
    unsigned long ret = WSEC_SUCCESS;
    if (len == 0) {
        return WSEC_SUCCESS;
    }
    if (!WSEC_FREAD_MUST(buff, len, keystore)) {
        return WSEC_ERR_READ_FILE_FAIL;
    }
    if (hashCtx != NULL) {
        ret = CacDigestUpdate(hashCtx, buff, len);
    }
    return ret;
}

/*
 * The maximum length of the KSF hardware persistency information and software-layer root key ciphertext
 * cannot exceed the specified maximum length.
 */
static unsigned long ReadLvInKsf(WsecHandle keystore,
    WsecHandle hashCtx,
    WsecUint32 maxLen,
    WsecVoid **buff,
    WsecUint32 *len)
{
    /* If the variable is not initialized, the CodeDEX alarm does not initialize the variable. */
    WsecUint32 dataLen = 0;
    unsigned long ret;
    WSEC_ASSERT(buff != NULL);
    WSEC_ASSERT(len != NULL);

    ret = ReadKsfData(keystore, hashCtx, &dataLen, (WsecUint32)sizeof(WsecUint32));
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    dataLen = WSEC_N2H_L(dataLen);
    *len = dataLen;
    /* The maximum size of the hardware persistent information is 1 MB. */
    if (dataLen > maxLen) {
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }
    if (dataLen == 0) {
        return WSEC_SUCCESS;
    }
    *buff = WSEC_MALLOC(dataLen);
    if (*buff == NULL) {
        return WSEC_ERR_MALLOC_FAIL;
    }
    ret = ReadKsfData(keystore, hashCtx, *buff, dataLen);
    return ret;
}

/*
 * Generate a new KSF version based on the following rules
 * WsecInitializeEx indicates that the V1/V2 mode is compatible. In this case, the KSF of the V2 version is generated.
 * WsecInitializeHw indicates the hardware mode. In this case, the V3 version is generated.
 */
static WsecUint16 GenNewKsfVersion(void)
{
    WSEC_LOG_I1("Begin to create ksf, hardware flag: %d.", PriKmcSysGetIsHardware());
    /*
     * The version number is determined when a KSF file is created.
     * The version number depends on whether the hardware returns
     * V3 or V2 (hardware V3; otherwise, the new environment V2).
     */
    return (WsecUint16)((PriKmcSysGetIsHardware() == WSEC_FALSE) ? KMC_KSF_VER_V2 : KMC_KSF_VER_V3);
}

/* Copy an available file to another or synchronization file. */
static unsigned long  RecoverKsfFromOkFile(const char* srcFile, char** destFiles, WsecUint32 destFilesNum)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 i;
    KmcKsfOneksfCorruptNotify corruptNotify = { NULL };
    for (i = 0; i < destFilesNum; i++) {
        if (WsecCopyFile(srcFile, destFiles[i]) != WSEC_TRUE) {
            WSEC_LOG_E2("Copy file failed. index:%lu, destFileNum:%lu", i, destFilesNum);
            corruptNotify.keyStoreFile = destFiles[i];
            ret = WSEC_ERR_FILE_COPY_FAIL;
            WSEC_NOTIFY(WSEC_KMC_NTF_ONE_KSF_CORRUPT, &corruptNotify, sizeof(corruptNotify));
        }
    }
    return ret;
}

static WsecVoid BackupMasterKsf(const KmcKsfName *filePathNames)
{
    unsigned long ret;
    char* destFiles[KMC_KSF_NUM] = {NULL};
    WsecUint32 destFilesNum = 1;

    destFiles[0] = filePathNames->keyStoreFile[BACKUP_KSF_INDEX];
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        destFiles[1] = filePathNames->keyStoreBackupFile;
        destFilesNum++;
    }
    ret = RecoverKsfFromOkFile(filePathNames->keyStoreFile[MASTER_KSF_INDEX], destFiles, destFilesNum);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_W1("Recover ksf failed, errno:%lu", ret);
    }
}

static WsecVoid CheckBackupKsfHash(const KmcKsfMem *ksfMem, KmcKsfMem* readBuff)
{
    /* The current hash value is the SHA256 result. Therefore, only 32 bytes are compared. */
    if (WSEC_MEMCMP(ksfMem->ksfHash, readBuff->ksfHash, KMC_HASH_SHA256_LEN) != 0) {
        WSEC_LOG_E2("KSF in memory(update number %u) is diffrent from KSF in file(update number %u)",
                    ksfMem->updateCounter,
                    readBuff->updateCounter);
    }
}

static WsecBool IsAllBackupCorrupt(unsigned long firstRet, unsigned long secondRet)
{
    /* If the third backup is not enabled, the second backup is damaged, that is, all backups are damaged. */
    if (PriKmcSysGetIsEnableThirdBackup() != WSEC_TRUE && firstRet != WSEC_SUCCESS) {
        return WSEC_TRUE;
    }
    /* Two Backups Are Damaged When Three Backups Are Enabled */
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE && firstRet != WSEC_SUCCESS && secondRet != WSEC_SUCCESS) {
        return WSEC_TRUE;
    }
    /* In other scenarios, ensure that at least one file is available. */
    return WSEC_FALSE;
}

/*
 * The function logic is based on the IsAllKsfCorrupt function.
 * Ensure that at least one file is available (at least one of firstRet and secondRet is WSEC_SUCCESS).
 */
static WsecBool IsFirstKsfBetter(unsigned long firstRet, unsigned long secondRet, KmcKsfMem * const *readBuff)
{
    /* If third backup is not enabled, only one file is available. The first file must be the optimal copy. */
    if (PriKmcSysGetIsEnableThirdBackup() != WSEC_TRUE) {
        return WSEC_TRUE;
    }
    /* If the second file is damaged and unavailable, the first file must be the optimal recovery copy. */
    if (secondRet != WSEC_SUCCESS) {
        return WSEC_TRUE;
    }
    /* If the first file is damaged, select the second file as the optimal recovery copy. */
    if (firstRet != WSEC_SUCCESS) {
        return WSEC_FALSE;
    }
    /*
     * When the two files are both OK, the first file is later
     * than the second file (the value of updatecouter is greater).
     */
    if (readBuff[0]->updateCounter >= readBuff[1]->updateCounter) {
        return WSEC_TRUE;
    }
    return WSEC_FALSE;
}

/* readBuff save backupFile and third backupFile, okFileIndex is the ok file index in readBuff */
static unsigned long SelectBestBackupFile(const KmcKsfName *filePathNames, char** destFiles,
    WsecUint32* destFileNum, KmcKsfMem * const *readBuff, WsecUint32* okFileIndex)
{
    unsigned long validKsf[KMC_KSF_NUM] = {WSEC_SUCCESS};

    *destFileNum = KMC_KSF_NUM - 1;
    validKsf[0] = ReadKsf(filePathNames->keyStoreFile[BACKUP_KSF_INDEX], __FUNCTION__, readBuff[0]);
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        validKsf[1] = ReadKsf(filePathNames->keyStoreBackupFile, __FUNCTION__, readBuff[1]);
        (*destFileNum)++;
    }

    /*
     * Ensure that at least one backup is available.
     * If both backups are unavailable, a CORRUPT notification is sent and returned.
     */
    if (IsAllBackupCorrupt(validKsf[0], validKsf[1])) {
        WSEC_LOG_E2("Not Copy the file to rollback due to incorrect ksf1 %lu. backup:%lu", validKsf[0], validKsf[1]);
        WSEC_NOTIFY(WSEC_KMC_NTF_KEY_STORE_CORRUPT, NULL, 0);
        return validKsf[0];
    }

    destFiles[0] = filePathNames->keyStoreFile[MASTER_KSF_INDEX];
    if (IsFirstKsfBetter(validKsf[0], validKsf[1], readBuff)) {
        *okFileIndex = 0;
        if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
            destFiles[1] = filePathNames->keyStoreBackupFile;
        }
        return WSEC_SUCCESS;
    }

    /* In other scenarios, the third backup is used as the recovery copy. */
    destFiles[1] = filePathNames->keyStoreFile[BACKUP_KSF_INDEX];
    *okFileIndex = 1;
    return WSEC_SUCCESS;
}

static unsigned long ReadBackupFiles(const KmcKsfMem *ksfMem, const KmcKsfName *filePathNames, char** okFile,
                                     char** destFiles, WsecUint32* destFileNum)
{
    KmcKsfMem *readBuff[KMC_KSF_NUM] = {NULL};
    unsigned long ret;
    WsecUint32 okFileIndex = 0;

    if (AllocMultiBlock(readBuff, sizeof(KmcKsfMem), (WsecUint32)(KMC_KSF_NUM)) == WSEC_FALSE) {
        WSEC_LOG_E4MALLOC(sizeof(KmcKsfMem));
        WSEC_LOG_E("Malloc multi block failed, when read backup files");
        return WSEC_ERR_MALLOC_FAIL;
    }

    ret = SelectBestBackupFile(filePathNames, destFiles, destFileNum, readBuff, &okFileIndex);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("SelectBestBackupFile Failed, ret=%lu", ret);
        WSEC_NOTIFY(WSEC_KMC_NTF_KEY_STORE_CORRUPT, NULL, 0);
        FreeKsfMemArraySnapshot(readBuff, KMC_KSF_NUM);
        return  ret;
    }
    *okFile = readBuff[okFileIndex]->fromFile;

    CheckBackupKsfHash(ksfMem, readBuff[okFileIndex]);
    FreeKsfMemArraySnapshot(readBuff, KMC_KSF_NUM);
    return WSEC_SUCCESS;
}

/* Securely save data to a set of files. */
static unsigned long WriteFileSafe(const KmcKsfMem *ksfMem,
    const KmcKsfName *filePathNames,
    const KmcKsfRk *rkNew,
    KmcKsfRk *rkWrite,
    const WsecBuff *ksfHash, const char *callBy)
{
    unsigned long ret;

    char *okFile = NULL;
    char *destFiles[KMC_KSF_NUM] = {NULL};
    WsecUint32 destFilesNum = 0;

    ret = WriteKsf(ksfMem, filePathNames->keyStoreFile[0], rkNew, rkWrite, ksfHash, callBy);
    /* If data fails to be written to the primary file, the secondary file is copied. */
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Write master ksf failed ret %lu, try to recover from backup ksf.", ret);
        if (ReadBackupFiles(ksfMem, filePathNames, &okFile, destFiles, &destFilesNum) != WSEC_SUCCESS) {
            WSEC_LOG_E("Recover From Backup Ksf failed.");
            return ret;
        }

        (void)RecoverKsfFromOkFile(okFile, destFiles, destFilesNum);
        /* NOTICE: The return value should not equal with WSEC_SUCCESS here, since the master file wrote failed,
         * the changed MK should be rollback. It will result the ksf Memory inconsistent with KSF if return success;
        */
        return ret;
    }

    /* Copy the file that is successfully written to other files. */
    BackupMasterKsf(filePathNames);
    return WSEC_SUCCESS;
}

static WsecVoid NotifyUpdateAfterWriteFile(void)
{
    KmcKsfUpdateNotify notifyUpdate = {{0}};
    KmcThirdKsfUpdateNotify thirdNotifyUpdate = {NULL};
    notifyUpdate.keyStoreFile[MASTER_KSF_INDEX] = PriKmcSysGetKsf(MASTER_KSF_INDEX);
    notifyUpdate.keyStoreFile[BACKUP_KSF_INDEX] = PriKmcSysGetKsf(BACKUP_KSF_INDEX);
    if (notifyUpdate.keyStoreFile[MASTER_KSF_INDEX] != NULL && notifyUpdate.keyStoreFile[BACKUP_KSF_INDEX] != NULL) {
        WSEC_NOTIFY(WSEC_KMC_NTF_KEY_STORE_UPDATE, &notifyUpdate, sizeof(notifyUpdate));
    }
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        thirdNotifyUpdate.keyStoreBackupFile = PriKmcSysGetKsfBackupKsf();
        WSEC_NOTIFY(WSEC_KMC_NTF_THIRD_KSF_UPDATE, &thirdNotifyUpdate, sizeof(thirdNotifyUpdate));
    }
}

static unsigned long WriteKsfSafetyLockable(WsecBool shareDomainMkChanged,
    const KmcKsfRk *rkNew, KmcKsfMem *ksfMem, WsecBool lockOrNot, const char *callBy)
{
    unsigned long ret;
    KmcWriteKsfFailNotify notify = {0};
    KmcKsfName ksf = {{NULL}, NULL};
    KmcKsfRk *rkWrite = NULL;
    unsigned char hashValue[WSEC_HASH_LEN_MAX] = {0};
    WsecBuff ksfHash;
    WSEC_BUFF_ASSIGN(ksfHash, hashValue, WSEC_HASH_LEN_MAX);

    ksf.keyStoreFile[MASTER_KSF_INDEX] = PriKmcSysGetKsf(MASTER_KSF_INDEX);
    ksf.keyStoreFile[BACKUP_KSF_INDEX] = PriKmcSysGetKsf(BACKUP_KSF_INDEX);
    ksf.keyStoreBackupFile = PriKmcSysGetKsfBackupKsf();
    if (ksfMem == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    rkWrite = (KmcKsfRk *)WSEC_MALLOC(sizeof(KmcKsfRk));
    if (rkWrite == NULL) {
        WSEC_LOG_E("WSEC_MALLOC failed KmcKsfRk");
        return WSEC_ERR_MALLOC_FAIL;
    }
    (ksfMem->updateCounter)++;
    ksfMem->sharedMkUpdateCounter = ksfMem->sharedMkUpdateCounter + (WsecUint32)(shareDomainMkChanged ? 1 : 0);
    if (lockOrNot == WSEC_TRUE) {
        ProcLockKeystore();
    }
    ret = WriteFileSafe(ksfMem, &ksf, rkNew, rkWrite, &ksfHash, callBy);
    if (lockOrNot == WSEC_TRUE) {
        ProcUnlockKeystore();
    }
    if (ret != WSEC_SUCCESS) {
        notify.errorCode = ret;
        (ksfMem->updateCounter)--;
        ksfMem->sharedMkUpdateCounter = ksfMem->sharedMkUpdateCounter - (WsecUint32)(shareDomainMkChanged ? 1 : 0);
        WSEC_NOTIFY(WSEC_KMC_NTF_WRI_KEY_STORE_FAIL, &notify, sizeof(notify));
        WSEC_LOG_E1("WriteFileSafe failed %lu", ret);
    } else {
        (void)memcpy_s(ksfMem->ksfHash, (size_t)WSEC_HASH_LEN_MAX, hashValue, (size_t)WSEC_HASH_LEN_MAX);
        ret = KmcProtectRkMaterials(rkWrite);
        if (ret == WSEC_SUCCESS) {
            /*
             * NOTICE: The logic is not strict. If the active node is successfully updated but the standby
             * and third nodes fail to be updated, the standby node is notified of the three-copy update success.
             */
            NotifyUpdateAfterWriteFile();
            (void)memcpy_s(&ksfMem->rk, sizeof(KmcKsfRk), rkWrite, sizeof(KmcKsfRk));
        }
    }
    WSEC_CLEAR_FREE(rkWrite, sizeof(KmcKsfRk));
    return ret;
}

/* Secure KSF writing (dual-copy DR considered) */
unsigned long WriteKsfSafety(WsecBool shareDomainMkChanged, const KmcKsfRk *rkNew, KmcKsfMem *ksfMem,
    const char *callBy)
{
    WsecBool lockOrNot = WSEC_TRUE;
    return WriteKsfSafetyLockable(shareDomainMkChanged, rkNew, ksfMem, lockOrNot, callBy);
}

/* Updating the KSF Root Key */
unsigned long KsfUpdateRootKey(const unsigned char *entropy, WsecUint32 size,
    const KmcCfg *kmcCfg,
    KmcKsfMem *ksfMem,
    WsecUint16 ksfVersion)
{
    unsigned long errorCode;
    WsecBuffConst entropyBuff = { NULL, 0 };
    KmcKsfHardRk hardRk;
    KmcKsfRk rkNew;
    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
    do {
        entropyBuff.buff = entropy;
        entropyBuff.len = size;
        /* The version number is reserved. */
        KmcAssignKsfHardRk(&ksfMem->hardRk, &hardRk);
        errorCode = KmcCreateRootKey(&entropyBuff, ksfVersion, kmcCfg, &rkNew, &ksfMem->hardRk);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E("KmcCreateRootKey() failed."); /* It doesn't matter if you fail. */
            break;
        }
        errorCode = WriteKsfSafety(WSEC_FALSE, &rkNew, ksfMem, __FUNCTION__);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E("WriteKsfSafety() failed.");
            break;
        }
        WSEC_LOG_I("KMC root key updated successfully.");
    } while (0);

    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    if (errorCode == WSEC_SUCCESS) {
        KmcKsfHardRkRmvFree(&hardRk);
    } else {
        WSEC_LOG_E2("%s update root key failed %lu.", __FUNCTION__, errorCode);
        KmcKsfHardRkRmvFree(&ksfMem->hardRk);
        KmcAssignKsfHardRk(&hardRk, &ksfMem->hardRk);
    }
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));

    return errorCode;
}

/*
 * Creates a master key set based on the current domain configuration and KeyType configuration, and creates a master
 * key for the internally generated domain (currently, only domain 0 is configured during initialization).
 */
static unsigned long CreateMkArrEx(const KmcCfg *kmcCfg, KmcKsfMem *ksfMem)
{
    unsigned long errorCode = WSEC_SUCCESS;
    KmcDomainCfg *domainCfg = NULL;
    KmcCfgKeyType *keyType = NULL;
    WsecUint32 keyId;
    int i;
    int j;
    int domainNum;
    int domainKeyTypeNum;

    WSEC_ASSERT(ksfMem != NULL);

    /* Create an MK based on the KMC configuration. */
    if (kmcCfg == NULL) {
        WSEC_LOG_E("The config memory does not exist");
        return WSEC_ERR_KMC_KEYCFGMEM_NOTEXIST;
    }

    domainNum = WsecArrGetCount(kmcCfg->domainCfgArray);
    for (i = 0; i < domainNum; i++) {
        domainCfg = (KmcDomainCfg *)WsecArrGetAt(kmcCfg->domainCfgArray, i);
        if (domainCfg == NULL) {
            WSEC_LOG_E("Domain cfg memory access failed.");
            errorCode = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        if (domainCfg->domainInfo.domainKeyFrom != KMC_MK_GEN_BY_INNER) {
            continue;
        }

        keyId = 0; /* The key ID is unique in each domain. */
        domainKeyTypeNum = WsecArrGetCount(domainCfg->keyTypeCfgArray);
        for (j = 0; j < domainKeyTypeNum; j++) {
            keyId++;
            keyType = (KmcCfgKeyType *)WsecArrGetAt(domainCfg->keyTypeCfgArray, j);
            if (keyType == NULL) {
                WSEC_LOG_E("Domain key type memory access fail.");
                errorCode = WSEC_ERR_OPER_ARRAY_FAIL;
                break;
            }
            errorCode = CreateMkItemEx(ksfMem, &domainCfg->domainInfo, keyType, NULL, keyId, WSEC_TRUE);
            if (errorCode != WSEC_SUCCESS) {
                WSEC_LOG_E1("CreateMkItemEx failed %lu", errorCode);
                break;
            }
        }
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("Master key creation Failed %lu.", errorCode);
            break;
        }
    }

    return errorCode;
}

/* Creating a Keystore File */
static unsigned long CreateKsf(const KmcCfg *kmcCfg, KmcKsfMem *ksfMem)
{
    unsigned long errorCode;
    KmcKsfRk rkNew;
    KmcKsfHardRk hardRk;
    /* Create KSF Version */
    WsecUint16 ksfVersion = GenNewKsfVersion();
    WsecUint32 role = PriKmcSysGetRole();
    WsecBool shareDomainMkChanged = WSEC_FALSE;
    if (ksfMem == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    if (IsKsfV1(ksfVersion) == WSEC_TRUE && role == KMC_ROLE_AGENT) {
        WSEC_LOG_E("V1 can only create ksf by master");
        return WSEC_ERR_KMC_INVALID_ROLETYPE;
    }
    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
    /* Creating a Root Key and an MK Array */
    do {
        KmcAssignKsfHardRk(&ksfMem->hardRk, &hardRk);
        errorCode = KmcCreateRootKey((WsecBuffConst *)NULL, ksfVersion, kmcCfg, &rkNew, &ksfMem->hardRk);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("KmcCreateRootKey failed %lu", errorCode);
            break;
        }
        /*
         * The V2/V3 Agent creates a default domain but does not create a shared key.
         * For details, see the MakeDomainDefaultCfg function.
         */
        if (role == KMC_ROLE_MASTER) {
            errorCode = CreateMkArrEx(kmcCfg, ksfMem);
            if (errorCode != WSEC_SUCCESS) {
                WSEC_LOG_E1("CreateMkArrEx failed %lu", errorCode);
                break;
            }
            shareDomainMkChanged = WSEC_TRUE;
        }
        /* Write data to a file. */
        errorCode = WriteKsfSafety(shareDomainMkChanged, &rkNew, ksfMem, __FUNCTION__);
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("WriteKsfSafety failed %lu", errorCode);
            break;
        }
    } while (0);

    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    if (errorCode == WSEC_SUCCESS) {
        KmcKsfHardRkRmvFree(&hardRk);
    } else {
        KmcKsfHardRkRmvFree(&ksfMem->hardRk);
        KmcAssignKsfHardRk(&hardRk, &ksfMem->hardRk);
    }
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
    return errorCode;
}

/* V3 KSF loading hardware RK */
static unsigned long LoadHwRootKey(WsecHandle keystore, WsecHandle hashCtx, KmcKsfHardRk *hardRk)
{
    unsigned long ret;
    KmcKsfHardRk tempHardRk = { WSEC_TRUE, { NULL, 0 }, { NULL, 0} };
    WSEC_ASSERT(keystore != NULL);
    do {
        /* The maximum size of the hardware persistent information is 1 MB. */
        ret = ReadLvInKsf(keystore, hashCtx, KMC_MAX_HARDWAREKEY_INFO_LEN,
            &tempHardRk.hrkInfo.buff, &tempHardRk.hrkInfo.len);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* Software-layer root key ciphertext */
        ret = ReadLvInKsf(keystore, hashCtx, KMC_MAX_SOFTLEVELRK_CIPHERTEXT_LEN,
            &tempHardRk.srkInfo.buff, &tempHardRk.srkInfo.len);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = KmcHardRkLoad(&tempHardRk);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("KmcHardRkLoad failed %lu", ret);
            break;
        }
        (void)memcpy_s(hardRk, sizeof(KmcKsfHardRk), &tempHardRk, sizeof(KmcKsfHardRk));
    } while (0);

    if (ret != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(tempHardRk.hrkInfo.buff, tempHardRk.hrkInfo.len);
        WSEC_CLEAR_FREE(tempHardRk.srkInfo.buff, tempHardRk.srkInfo.len);
    }
    return ret;
}

/* Reads the KSF RK by file handle. */
static unsigned long ReadRootKeyByHandle(WsecHandle keystore,
    WsecHandle hashCtx,
    KmcKsfRk *rk,
    KmcKsfMem *ksfMem)
{
    unsigned char ksfFormatFlag[sizeof(g_ksfFlag)] = {0};
    WsecUint16 ksfVersion;
    unsigned long ret;

    if (!(keystore != NULL && rk != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        /* 2) Read and check the format code. */
        ret = ReadKsfData(keystore, hashCtx, ksfFormatFlag, (WsecUint32)sizeof(ksfFormatFlag));
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("Read File fail.");
            break;
        }
        if (WSEC_MEMCMP(ksfFormatFlag, g_ksfFlag, sizeof(g_ksfFlag)) != 0) {
            WSEC_LOG_E("The file is not KSF format.");
            ret = WSEC_ERR_KMC_NOT_KSF_FORMAT;
            break;
        }

        /* 3) Read the root key information. */
        ret = ReadKsfData(keystore, hashCtx, rk, (WsecUint32)sizeof(KmcKsfRk));
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E("Read File fail.");
            break;
        }
        /* 4) Byte order conversion */
        ksfVersion = WSEC_N2H_S(rk->rkAttributes.version);
        if (IsValidKsfVersion(ksfVersion) == WSEC_FALSE) {
            WSEC_LOG_E1("KSF version %hu is not correct.", ksfVersion);
            return WSEC_ERR_KMC_KSF_DATA_INVALID;
        }
        CvtByteOrderForKsfRk(rk, WBCNETWORK2HOST, ksfVersion);

        /* 5) Check the RK, verify the Hash Value to obtain the version number */
        ret = KmcCheckRk(rk, ksfVersion);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
            ret = LoadHwRootKey(keystore, hashCtx, &ksfMem->hardRk);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("LoadHwRootKey failed %lu.", ret);
                break;
            }
        }
        /*
         * Each time the file is read, the host sequence KsfRk is read to the file.
         * If the host sequence is correct, synchronize the host sequence to the ksfMem memory.
         */
        (void)memcpy_s(&ksfMem->rk, sizeof(KmcKsfRk), rk, sizeof(KmcKsfRk));
        ksfMem->updateCounter = rk->updateCounter;
        ret = KmcProtectRkMaterials(&ksfMem->rk);
    } while (0);

    return ret;
}

static WsecBool CheckKsfExistsNoneEmpty(const char *file)
{
    long fileLen = 0;
    WSEC_ASSERT(file != NULL);
    if (WSEC_FSTATUS(file) == WSEC_FALSE) {
        return WSEC_FALSE;
    }
    /* if file exist but get file length error
     * may be syscall error so ignore */
    if (WsecGetFileLen(file, &fileLen) == WSEC_FALSE) {
        return WSEC_FALSE;
    }
    /* if WsecGetFileLen successfully file length >= 0 */
    return (fileLen > 0) ? WSEC_TRUE : WSEC_FALSE;
}

/*
 * Before secure reading, the KSF file is checked. If the file is damaged or does not exist, a notification is sent.
 * If the root key can be successfully read from a file, WSEC_TRUE is returned. Otherwise, WSEC_FALSE is returned.
 */
static WsecBool HasKsf(void)
{
    WsecUint32 i;
    /* init valid count is 2 or 3 */
    WsecUint32 notEmptyCount =
        (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) ? KMC_KSF_WITH_THIRD_NUM : KMC_KSF_NUM;
    ProcLockKeystore();
    /* check master and backup */
    for (i = 0; i < KMC_KSF_NUM; i++) {
        if (CheckKsfExistsNoneEmpty(PriKmcSysGetKsf(i)) == WSEC_FALSE) {
            /* The counter decreases by 1 if the KSF does not exist. */
            notEmptyCount--;
        }
    }
    /* if third backup enable check the third backup */
    if ((PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) &&
        (CheckKsfExistsNoneEmpty(PriKmcSysGetKsfBackupKsf()) == WSEC_FALSE)) {
        notEmptyCount--;
    }
    ProcUnlockKeystore();
    /* If the file does not exist, an alarm is reported indicating that the file does not exist. */
    if (notEmptyCount == 0) {
        WSEC_LOG_W("KSF not exist!");
        WSEC_NOTIFY(WSEC_KMC_NTF_KSF_INITOPENFAIL, NULL, 0);
        return WSEC_FALSE;
    }
    return WSEC_TRUE;
}

/* Allocate a master key to derive RMK in KSF space. */
static unsigned long AllocMkRecordAndDeriveRmk(const KmcKsfRk *rkBuff, WsecBuff *mkRecord, WsecBuff *rmk)
{
    unsigned long returnValue = WSEC_SUCCESS;
    WsecUint32 mkRecordLen;
    WsecUint16 ksfVersion;
    WSEC_ASSERT(rkBuff != NULL);
    WSEC_ASSERT(mkRecord != NULL);
    WSEC_ASSERT(mkRecord->buff == NULL); /* To be allocated */
    WSEC_ASSERT(rmk != NULL);
    WSEC_ASSERT(rmk->buff == NULL); /* To be allocated */
    ksfVersion = rkBuff->rkAttributes.version;

    /* 1. Allocate resources and read rootkey information. */
    do {
        /* Calculate the size of the MK in the KSF file based on the version number. */
        if (IsKsfV1OrV2(ksfVersion) == WSEC_TRUE) {
            mkRecordLen = sizeof(KmcKsfMk);
        } else if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
            mkRecordLen = rkBuff->mkRecordLen;
        } else {
            returnValue = WSEC_ERR_KMC_READ_DIFF_VER_KSF_FAIL;
            break;
        }
        if ((mkRecordLen < sizeof(KmcKsfMk) || mkRecordLen > KMC_MAX_MK_RECORD_LEN)) {
            returnValue = WSEC_ERR_LARGER_THAN_MAX_MK_RECORD_LEN;
            break;
        }
        mkRecord->buff = WSEC_MALLOC(mkRecordLen);
        if (mkRecord->buff == NULL) {
            returnValue = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        mkRecord->len = mkRecordLen;
        rmk->buff = WSEC_MALLOC(KMC_RMK_DOUBLE_LEN); /* Part 2: EK IK */
        if (rmk->buff == NULL) {
            returnValue = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        rmk->len = KMC_RMK_DOUBLE_LEN; /* Part 2: EK IK */
        /* Calculating RMK */
        if (KmcMakeRmk(rkBuff, rmk) == WSEC_FALSE) {
            WSEC_LOG_E("KmcMakeRmk() fail.");
            returnValue = WSEC_ERR_PBKDF2_FAIL;
            break;
        }
    } while (0);

    if (returnValue != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(mkRecord->buff, mkRecord->len);
        WSEC_CLEAR_FREE(rmk->buff, rmk->len);
    }
    return returnValue;
}

/* Allocate root key space, read root keys, allocate master key space, and derive RMKs. */
static unsigned long PrepareForReadKsf(WsecHandle keystore,
    WsecHandle hashCtx,
    KmcKsfRk **rkBuff,
    WsecBuff *mkRead,
    WsecBuff *rmkBuff,
    KmcKsfMem *ksfMem)
{
    unsigned long returnValue;
    WSEC_ASSERT(keystore != NULL);
    WSEC_ASSERT(rkBuff != NULL);

    /* 1. Allocate resources and read rootkey information. */
    do {
        *rkBuff = (KmcKsfRk *)WSEC_MALLOC(sizeof(KmcKsfRk));
        if (*rkBuff == NULL) {
            WSEC_LOG_E4MALLOC(sizeof(KmcKsfRk));
            returnValue = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        returnValue = ReadRootKeyByHandle(keystore, hashCtx, *rkBuff, ksfMem);
        if (returnValue != WSEC_SUCCESS) {
            WSEC_LOG_E1("ReadRootKeyByHandle() failed %lu", returnValue);
            break;
        }
        returnValue = AllocMkRecordAndDeriveRmk(*rkBuff, mkRead, rmkBuff);
        if (returnValue != WSEC_SUCCESS) {
            WSEC_LOG_E1("AllocMkRecordAndDeriveRmk() failed %lu", returnValue);
            break;
        }
    } while (0);

    /* Sensitive information needs to be released in V1, V2, and V3. */
    if (returnValue != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(*rkBuff, sizeof(KmcKsfRk));
    }
    return returnValue;
}

/* In V1 ciphertextLen is not converted during byte order conversion,but its value is limited within WSEC_MK_LEN_MAX. */
static WsecBool ChangeMkCiphertextLenByteOrder(KmcKsfMk *mk)
{
    WSEC_ASSERT(mk != NULL);
    /* The value of KMC_MAX_MK_RECORD_LEN cannot exceed 65535. */
    if (mk->ciphertextLen > KMC_MAX_MK_RECORD_LEN) {
        mk->ciphertextLen = WSEC_SWAP_LONG(mk->ciphertextLen);
        if (mk->ciphertextLen > KMC_MAX_MK_RECORD_LEN) {
            WSEC_LOG_E1("Get the cipherlen of Mk fail. %u", mk->ciphertextLen);
            return WSEC_FALSE;
        }
    }
    return WSEC_TRUE;
}

/* Converts the byte order to the host byte order after the read operation is complete. */
static unsigned long ReadOneKsfMk(WsecHandle keystore,
    WsecHandle hmacCtx,
    WsecUint16 ksfVersion,
    unsigned char *mkKsf,
    WsecUint32 *mkKsfLen)
{
    unsigned long ret;
    KmcKsfMk *mk = (KmcKsfMk *)(WsecVoid *)mkKsf;
    WSEC_ASSERT(keystore != NULL);
    WSEC_ASSERT(mkKsf != NULL);
    WSEC_ASSERT(mkKsfLen != NULL);
    WSEC_ASSERT(*mkKsfLen >= sizeof(KmcKsfMk));
    ret = ReadKsfData(keystore, hmacCtx, mkKsf, (WsecUint32)sizeof(KmcKsfMk));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("ReadMkDataFromKsf fail.");
        return ret;
    }
    CvtByteOrderForKsfMk(mk, WBCNETWORK2HOST, ksfVersion);

    /* The byte order of the V1 ciphertext length is not converted, which has a bug. */
    if (ChangeMkCiphertextLenByteOrder(mk) != WSEC_TRUE) {
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }

    if (mk->ciphertextLen <= sizeof(mk->v2orV3.mkV3.key)) {
        *mkKsfLen = sizeof(KmcKsfMk);
        return WSEC_SUCCESS;
    }
    if (*mkKsfLen < (mk->ciphertextLen - sizeof(mk->v2orV3.mkV3.key)) + sizeof(KmcKsfMk)) {
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }
    ret = ReadKsfData(keystore, hmacCtx, mkKsf + sizeof(KmcKsfMk), mk->ciphertextLen - (WsecUint32)sizeof(mk->v2orV3.mkV3.key));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("ReadMkDataFromKsf fail.");
        return ret;
    }
    *mkKsfLen = (mk->ciphertextLen - (WsecUint32)sizeof(mk->v2orV3.mkV3.key)) + (WsecUint32)sizeof(KmcKsfMk);   /* soter 554 */
    return WSEC_SUCCESS;
}

/* Verifying KSF MK HMAC */
static unsigned long DoHmacVerifyForMemMk(KmcKsfMk *mkRead, WsecUint16 ksfVersion, WsecBuff rmkBuff,
    const KmcMemMk *mkMem)
{
    WsecBuffConst buff[4] = { { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 } }; /* 4 buffers are used for hashing */
    unsigned long ret;
    WsecBuff hmacKey = { NULL, 0 };
    WsecBuff hmacBuff = { NULL, 0 };
    WsecUint32 buffCount = 0;
    WsecUint32 domainId;
    WsecUint32 keyId;
    WSEC_ASSERT(mkRead != NULL);
    WSEC_ASSERT(mkMem != NULL);
    WSEC_ASSERT(rmkBuff.buff != NULL);
    domainId = mkRead->mkInfo.domainId;
    keyId = mkRead->mkInfo.keyId;
    if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
        CvtByteOrderForKsfMk(mkRead, WBCHOST2NETWORK, ksfVersion); /* Network sequence check */
    }
    /* 2.4.2 MK Integrity Check */
    WSEC_BUFF_ASSIGN(buff[buffCount], &mkRead->mkInfo, sizeof(mkRead->mkInfo));
    buffCount++;
    if (IsKsfV1AndNotV2(ksfVersion) == WSEC_TRUE) {
        WSEC_BUFF_ASSIGN(buff[buffCount], &(mkRead->cipherAlgId),
            (sizeof(mkRead->cipherAlgId) + sizeof(mkRead->reserve)));
        buffCount++;
    }
    WSEC_BUFF_ASSIGN(buff[buffCount], &(mkRead->v2orV3.mkV2.mkRear.plaintextLen), sizeof(mkRead->v2orV3.mkV2.mkRear.plaintextLen));
    buffCount++;
    WSEC_BUFF_ASSIGN(buff[buffCount], mkMem->mkRear.key, mkMem->mkRear.plaintextLen);
    buffCount++;
    WSEC_BUFF_ASSIGN(hmacKey, ((unsigned char *)rmkBuff.buff + KMC_RMK_LEN), KMC_RMK_LEN); /* IK */
    WSEC_BUFF_ASSIGN(hmacBuff, mkRead->v2orV3.mkV2.mkHash, sizeof(mkRead->v2orV3.mkV2.mkHash)); /* HMAC result */
    ret = WsecCheckHmacCode(KMC_HMAC_MK_ALGID, buff, buffCount, &hmacKey, &hmacBuff);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E3("Hmac check of Mk (domain %u keyId %u) failed %lu", domainId, keyId, ret);
        return ret;
    }

    if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
        CvtByteOrderForKsfMk(mkRead, WBCNETWORK2HOST, ksfVersion); /* Change back to the host order. */
    }
    return WSEC_SUCCESS;
}

/* V1\2 KSF MK obtains memory MK. */
static unsigned long MakeV1V2MemMk(KmcKsfMk *mkRead, WsecUint16 ksfVersion, WsecBuff rmkBuff, KmcMemMk *mkMem)
{
    unsigned long ret;
    unsigned char tempKey[WSEC_MK_LEN_MAX];
    unsigned char hashData[KMC_HASH_SHA256_LEN] = {0};
    WsecUint32 hashLen = KMC_HASH_SHA256_LEN;
    WSEC_ASSERT(rmkBuff.buff != NULL);
    WSEC_ASSERT(rmkBuff.len == KMC_RMK_DOUBLE_LEN); /* Two keys, EK IK */
    /* 2.4.1 Decrypting the MK */
    (void)memcpy_s(tempKey, (size_t)WSEC_MK_LEN_MAX, mkRead->v2orV3.mkV3.key, (size_t)WSEC_MK_LEN_MAX);
    mkMem->mkRear.plaintextLen = sizeof(mkMem->mkRear.key);
    ret = CacDecrypt((IsKsfV1AndNotV2(ksfVersion) == WSEC_TRUE) ? WSEC_ALGID_AES256_CBC : mkRead->cipherAlgId,
        rmkBuff.buff, KMC_RMK_LEN, mkRead->iv, (WsecUint32)sizeof(mkRead->iv),
        tempKey, mkRead->ciphertextLen, mkMem->mkRear.key, &mkMem->mkRear.plaintextLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("CacDecrypt() fail %lu", ret);
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }

    ret = DoHmacVerifyForMemMk(mkRead, ksfVersion, rmkBuff, mkMem);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    (void)memcpy_s(&(mkMem->mkInfo), sizeof(KmcMkInfo), &(mkRead->mkInfo), sizeof(KmcMkInfo));

    /* Calculate the hash value of the MK. */
    /* Generates the hash of the key. */
    if (CacDigest(WSEC_ALGID_SHA256, mkMem->mkRear.key, mkMem->mkRear.plaintextLen,
        hashData, &hashLen) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacDigest() failed.");
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }

    if (memcpy_s(mkMem->hashData, (size_t)WSEC_MK_HASH_REC_LEN, hashData, (size_t)WSEC_MK_HASH_REC_LEN) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }
    return WSEC_SUCCESS;
}

/* KSF MK: Obtains the memory MK of the V3 version. */
static unsigned long MakeV3MemMk(const KmcKsfHardRk *hardRk, const WsecVoid *mkRead, WsecUint32 mkReadLen,
    KmcMemMk *mkMem)
{
    const KmcKsfMk *mk = (const KmcKsfMk *)mkRead;
    unsigned long ret;
    unsigned char hashData[KMC_HASH_SHA256_LEN] = {0};
    WsecUint32 hashLen = KMC_HASH_SHA256_LEN;
    WsecBuffConst cipherBuff;

    if (mk->ciphertextLen > sizeof(mk->v2orV3.mkV3.key) &&
        (mkReadLen - sizeof(KmcKsfMk)) != (mk->ciphertextLen - sizeof(mk->v2orV3.mkV3.key))) {  /* soter 554 */
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }
    mkMem->mkRear.plaintextLen = sizeof(mkMem->mkRear.key);
    if (PriKmcSysGetHasSoftLevelRk() == WSEC_TRUE) {
        cipherBuff.buff = mk->v2orV3.mkV3.key;
        cipherBuff.len = mk->ciphertextLen;
        ret = KmcDecBySrk(hardRk, mk->cipherAlgId, mk->iv, (unsigned int)sizeof(mk->iv),
            &cipherBuff, mkMem->mkRear.key, &mkMem->mkRear.plaintextLen);
    } else {
        ret = KmcDecByHrk(hardRk, mk->v2orV3.mkV3.key, mk->ciphertextLen, mkMem->mkRear.key, &mkMem->mkRear.plaintextLen);
    }
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("MakeV3MemMk decrypt failed , %lu", ret);
        return ret;
    }
    (void)memcpy_s(&mkMem->mkInfo, sizeof(KmcMkInfo), &mk->mkInfo, sizeof(KmcMkInfo));

    /* Calculate the hash value of the MK. */
    /* Generates the hash of the key. */
    if (CacDigest(WSEC_ALGID_SHA256, mkMem->mkRear.key, mkMem->mkRear.plaintextLen,
        hashData, &hashLen) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacDigest() fail.");
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }
    if (memcpy_s(mkMem->hashData, (size_t)WSEC_MK_HASH_REC_LEN, hashData, (size_t)WSEC_MK_HASH_REC_LEN) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_KMC_NOT_KSF_FORMAT;
    }
    return ret;
}

/* Obtain the memory MK from the KSF MK record. */
static unsigned long MakeOneMemMk(const KmcKsfHardRk *hardRk,
    WsecVoid *mkRead,
    WsecUint32 mkReadLen,
    WsecUint16 ksfVersion,
    WsecBuff rmkBuff,
    KmcMemMk *mkMem)
{
    unsigned long ret;
    WSEC_ASSERT(mkRead != NULL);
    WSEC_ASSERT(mkReadLen >= sizeof(KmcKsfMk));
    WSEC_ASSERT(mkMem != NULL);
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    if (IsKsfV1OrV2(ksfVersion) == WSEC_TRUE) {
        WSEC_ASSERT(mkReadLen == sizeof(KmcKsfMk));
        ret = MakeV1V2MemMk((KmcKsfMk *)mkRead, ksfVersion, rmkBuff, mkMem);
    } else {
        WSEC_ASSERT(mkReadLen >= sizeof(KmcKsfMk));
        ret = MakeV3MemMk(hardRk, mkRead, mkReadLen, mkMem);
    }
    return ret;
}

/* Read MK from KSF */
static unsigned long ReadMkFromKsf(WsecHandle ksf,
    WsecHandle hashCtx,
    const KmcKsfRk *rk,
    WsecBuff mkRead,
    WsecBuff rmkBuff,
    KmcKsfMem *ksfMem)
{
    WsecUint16 ksfVersion = rk->rkAttributes.version;
    WsecUint32 i;
    KmcMemMk *mkRec = NULL;
    WsecUint32 readLen;
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 readMkNum = 0;
    /*
     * 2: read MK
     * Note: During MK reading, if a single MK fails to be read, subsequent MKs are still read.
     */
    for (i = 0; i < rk->mkNum; i++) {
        /* 2.1 Applying for Nodes to Be Added to the MK Linked List */
        mkRec = (KmcMemMk *)WSEC_MALLOC(sizeof(KmcMemMk));
        if (mkRec == NULL) {
            WSEC_LOG_E4MALLOC(sizeof(KmcMemMk));
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        readLen = mkRead.len;
        /* 2.2 Read a complete MK record. */
        ret = ReadOneKsfMk(ksf, hashCtx, ksfVersion, (unsigned char *)mkRead.buff, &readLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("ReadOneKsfMk failed %lu", ret);
            break;
        }

        ret = MakeOneMemMk(&ksfMem->hardRk, mkRead.buff, readLen, ksfVersion, rmkBuff, mkRec);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("MakeOneMemMk failed %lu", ret);
            break;
        }
        /* 2.5 Add MK to the array. */
        ret = AddMkToArray(ksfMem, mkRec, WSEC_FALSE);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("AddMkToArray() failed %lu", ret);
            break;
        }
        mkRec = NULL;
        readMkNum++;

        DoEvent(i);
    }
    WSEC_CLEAR_FREE(mkRec, sizeof(KmcMemMk));
    if (rk->mkNum != readMkNum) {
        WSEC_LOG_E3("ReadMkFromKsf failed %lu, excepted count %u actually read %u.", ret, rk->mkNum, readMkNum);
        ret = WSEC_ERR_KMC_READ_MK_FAIL;
    }
    return ret;
}

/* Read and verify the KSF HMAC of the V3 version. */
static unsigned long ReadAndCheckV3KsfHmac(WsecHandle ksf,
    WsecUint16 ksfVersion,
    const KmcKsfMem *ksfMem,
    KmcKsfHmac *calcHmac, WsecUint32 hashLen)
{
    WsecUint32 hmacLen = WSEC_HMAC_LEN_MAX;
    KmcKsfHmac ksfHmac;
    unsigned long ret;
    if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
        if (!WSEC_FREAD_MUST(&ksfHmac, sizeof(ksfHmac), ksf)) {
            WSEC_LOG_E("read ksf hmac failed in v3");
            return WSEC_ERR_READ_FILE_FAIL;
        }
        ret = KmcHardRkGetKsfHmac(&ksfMem->hardRk, WSEC_ALGID_HMAC_SHA256, calcHmac, hashLen, &hmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("KmcHardRkGetKsfHmac fail %lu", ret);
            return ret;
        }
        WSEC_ASSERT(hashLen == KMC_HASH_SHA256_LEN);
        if (WSEC_MEMCMP(calcHmac->HashData.hashData, ksfHmac.HashData.hashData, hashLen) != 0) {
            WSEC_LOG_E2("hash not match %llx %llx", ksfHmac.HashData.partHash, calcHmac->HashData.partHash);
            return WSEC_ERR_HASH_NOT_MATCH;
        }
        WSEC_ASSERT(hmacLen == KMC_HMAC_SHA256_LEN);
        if (WSEC_MEMCMP(calcHmac->HashHmac.hashHmac, ksfHmac.HashHmac.hashHmac, hmacLen) != 0) {
            WSEC_LOG_E2("hmac not match %llx %llx", ksfHmac.HashHmac.partHmac, calcHmac->HashHmac.partHmac);
            return WSEC_ERR_HMAC_AUTH_FAIL;
        }
    }
    return WSEC_SUCCESS;
}

/*
 * Reads data from a specified Keystore file (KSF for short).
 * Note: WSEC_ERR_KMC_READ_MK_FAIL indicates that an error occurs in the process of reading the media key,
 * However, the read MK is correctly output to keystoreData->mkArray.
 */
unsigned long ReadKsf(const char *keystoreFile, const char *callBy, KmcKsfMem *ksfMem)
{
    WsecHandle ksf = NULL;
    KmcKsfRk *rk = NULL;
    WsecBuff mkRead = { NULL, 0 };
    WsecBuff rmkBuff = { NULL, 0 };
    unsigned long ret;
    WsecHandle hashCtx = NULL;
    WsecUint32 hashLen = WSEC_HASH_LEN_MAX;
    KmcKsfHmac calcHmac;
    if (!(keystoreFile != NULL && ksfMem != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        ret = CacDigestInit(&hashCtx, WSEC_ALGID_SHA256);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ksf = WSEC_FOPEN(keystoreFile, KMC_FILE_READ_BINARY);
        if (ksf == NULL) {
            WSEC_LOG_E("Open KSF failed.");
            ret = WSEC_ERR_OPEN_FILE_FAIL;
            break;
        }
        ret = PrepareForReadKsf(ksf, hashCtx, &rk, &mkRead, &rmkBuff, ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ret = ReadMkFromKsf(ksf, hashCtx, rk, mkRead, rmkBuff, ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CacDigestFinal(&hashCtx, calcHmac.HashData.hashData, &hashLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        (void)memcpy_s(ksfMem->ksfHash, (size_t)WSEC_HASH_LEN_MAX, calcHmac.HashData.hashData, (size_t)WSEC_HASH_LEN_MAX);
        ret = ReadAndCheckV3KsfHmac(ksf, rk->rkAttributes.version, ksfMem, &calcHmac, hashLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I3("%s read ksf at %llx with update counter %u", callBy, calcHmac.HashData.partHash, ksfMem->updateCounter);
    } while (0);
    CacDigestReleaseCtx(&hashCtx);
    WSEC_FCLOSE(ksf);
    WSEC_CLEAR_FREE(mkRead.buff, mkRead.len);
    WSEC_CLEAR_FREE(rmkBuff.buff, rmkBuff.len);
    WSEC_CLEAR_FREE(rk, sizeof(KmcKsfRk));
    return ret;
}

/* returnValue only return lastest error code */
static unsigned long IsAllKsfCorrupt(KmcKsfMem * const *readBuff, WsecUint32 count,
    WsecUint32 *aimAt, unsigned long *returnValue)
{
    WsecUint32 i;
    unsigned long temp;
    WsecUint32 updateCounter = 0;
    unsigned long ret = WSEC_ERR_KMC_KSF_CORRUPT;
    WsecUint32 fileCount;
    WSEC_ASSERT(aimAt != NULL);
    WSEC_ASSERT(*aimAt == (KMC_KSF_WITH_THIRD_NUM));
    WSEC_ASSERT(count == (KMC_KSF_WITH_THIRD_NUM));

    readBuff[0]->fromFile = PriKmcSysGetKsf(MASTER_KSF_INDEX);
    readBuff[1]->fromFile = PriKmcSysGetKsf(BACKUP_KSF_INDEX);
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        readBuff[KMC_KSF_NUM]->fromFile = PriKmcSysGetKsfBackupKsf();
    }
    fileCount = (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) ? count : count - 1;   /* soter 554 */
    WSEC_LOG_I1("Total fileCount is :%lu.", fileCount);
    for (i = 0; i < fileCount; i++) {
        temp = ReadKsf(readBuff[i]->fromFile, __FUNCTION__, readBuff[i]);
        WSEC_LOG_I2("Read ksf index=%lu result: %lu", i, temp);
        if (temp == WSEC_SUCCESS && readBuff[i]->updateCounter >= updateCounter) {
            ret = WSEC_SUCCESS;
            updateCounter = readBuff[i]->updateCounter;
            *aimAt = i;
        }
        if (temp != WSEC_SUCCESS && returnValue != NULL) {
            *returnValue = temp;
        }
    }
    return ret;
}

/* Secondary reconstruction with the existing recoverFromOkFile */
static unsigned long RecoverFromAimFile(KmcKsfMem * const *readBuff, const WsecUint32 *aimAt)
{
    WsecUint32 i;
    char *okFile = NULL;
    char *destFile[KMC_KSF_NUM] = {NULL};
    WsecUint32 destFilesNum = 0;
    WsecUint32 fileCount;

    WSEC_ASSERT(readBuff != NULL);
    WSEC_ASSERT(aimAt != NULL);
    WSEC_ASSERT(*aimAt != (KMC_KSF_WITH_THIRD_NUM));

    fileCount = (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) ? KMC_KSF_WITH_THIRD_NUM : KMC_KSF_NUM;
    okFile = readBuff[*aimAt]->fromFile;
    WSEC_LOG_I1("total dest files: %lu.", fileCount);
    for (i = 0; i < fileCount; i++) {
        /*
         * If the target file is the same as the source file or the content of the target file
         * is the same as that of the source file, skip this step.
         */
        if (*aimAt == i || WSEC_MEMCMP(readBuff[i]->ksfHash,
            readBuff[*aimAt]->ksfHash, KMC_HASH_SHA256_LEN) == 0) {
            continue;
        }
        WSEC_LOG_I3("okFile index:%lu, destFile:%lu, fileCount:%lu.", *aimAt, i, fileCount);
        destFile[destFilesNum++] = readBuff[i]->fromFile;
    }
    return RecoverKsfFromOkFile(okFile, destFile, destFilesNum);
}

/* Read KSF */
static unsigned long ReadAllKsf(WsecUint32 role, KmcKsfMem * const *readBuff, WsecUint32 count,
    WsecUint32 *aimAt,
    KmcKsfMem **keystore)
{
    unsigned long ret;
    WSEC_ASSERT(count == KMC_KSF_WITH_THIRD_NUM);
    /* 2. Read files A and B. */
    ProcLockKeystore();
    ret = IsAllKsfCorrupt(readBuff, count, aimAt, NULL);
    /* If any file is damaged and one file is complete, overwrite the damaged file.
     * If all files are damaged, an alarm is reported.
     */
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("All KSF is corrupt");
        WSEC_NOTIFY(WSEC_KMC_NTF_KEY_STORE_CORRUPT, NULL, 0);
    } else {
        (void)RecoverFromAimFile(readBuff, aimAt);
        /* Release the old space and set a new keystore. */
        *keystore = FreeKsfSnapshot(*keystore);
        *keystore = readBuff[*aimAt];
    }
    ProcUnlockKeystore();
    /*
     * V1 does not allow AGENT to be started. (V1 must be started as MASTER.
     * Otherwise, some management actions cannot be completed.)
     */
    if (ret == WSEC_SUCCESS && readBuff[*aimAt]->rk.rkAttributes.version == KMC_KSF_VER && role == KMC_ROLE_AGENT) {
        /*
         * In the V1 upgrade scenario, the upgrade is normal.
         * Otherwise, the agent cannot be started during V1 compatibility.
         */
        if (PriKmcSysGetIsHardware() == WSEC_FALSE) {
            WSEC_LOG_E("V1 not support agent role");
            ret = WSEC_ERR_KMC_INVALID_ROLETYPE;
        }
    }
    return ret;
}

/*
 * Securely reads Keystore data.
 * The system supports two Keystore files. If an error occurs during data reading, the system reads another file.
 * 1. Preferentially select the data that can be correctly read from the file.
 * 2. If all the values are correctly read, the number of KSF updates is large.
 * 3. If the master node detects a file exception, it uses the normal file to overwrite the abnormal file.
 * 4. If the file is an agent file, the file is directly read. If one file is normal, the file can be loaded.
 * Note: This function writes the successfully read Keystore data into the global variable keystore.
 */
unsigned long ReadKsfSafety(WsecUint32 role, const KmcCfg *kmcCfg, KmcKsfMem **keystore)
{
    KmcKsfMem *readBuff[KMC_KSF_WITH_THIRD_NUM] = {NULL};
    WsecUint32 i;
    /* aimAt indicates that the readBuff element is used only when *keystore is set. */
    WsecUint32 aimAt = KMC_KSF_WITH_THIRD_NUM;
    unsigned long ret;
    WsecBool hasOkFile = WSEC_FALSE;
    WsecUint32 blockSize = KMC_KSF_WITH_THIRD_NUM;

    if (keystore == NULL) {
        WSEC_LOG_E("Invalid input keystore pointer");
        return WSEC_ERR_INVALID_ARG;
    }
    /* Apply for resources. */
    if (AllocMultiBlock(readBuff, sizeof(KmcKsfMem), blockSize) == WSEC_FALSE) {
        return WSEC_ERR_MALLOC_FAIL;
    }
    do {
        /* 1. Check whether the KSF exists. */
        hasOkFile = HasKsf();
        /* Keystores need to be created. Only V1 and V2 servers with the master role can create keystores. */
        /* V3 servers can create keystores, but the Agent server does not create a master key. */
        if (hasOkFile == WSEC_FALSE) {
            ret = CreateKsf(kmcCfg, readBuff[0]);
            if (ret == WSEC_SUCCESS) {
                aimAt = 0;
                readBuff[0]->fromFile = PriKmcSysGetKsf(MASTER_KSF_INDEX);
                *keystore = FreeKsfSnapshot(*keystore);
                *keystore = readBuff[0];
            }
            break; /* No matter whether the write operation is successful or fails, the process ends. */
        }
        ret = ReadAllKsf(role, readBuff, blockSize, &aimAt, keystore);
        if ((PriKmcSysGetIsDeleteKsfOnInitFailed() == WSEC_TRUE) &&
            (ret == WSEC_ERR_KMC_KSF_CORRUPT)) {
            WSEC_LOG_I("Delete all corrupt KSF on init kmc.");
            (void)SecureEraseKeystore();
        }
    } while (0);

    /* 5. Release resources. */
    for (i = 0; i < blockSize; i++) {
        if (i != aimAt) {
            readBuff[i] = FreeKsfSnapshot(readBuff[i]);
        }
    }

    return ret;
}

/* Preparing to write to the KSF */
static unsigned long PrepareForWriteKsf(const KmcKsfRk *rkMem,
    const KmcKsfMem *ksfMem,
    KmcKsfRk *rkWrite,
    WsecBuff *mkKsf,
    WsecBuff *rmkBuff)
{
    unsigned long errorCode;
    WsecBuffConst buffArray[1] = {{ NULL, 0 }};
    WsecBuff aboveHash = { NULL, 0 };
    /* If no initial value is assigned,CodeDEX generates an alarm but does not assign initial value to the variable. */
    WsecUint32 v3CiphertextLen = 0;
    WsecUint16 ksfVersion;

    WSEC_ASSERT(ksfMem != NULL);
    WSEC_ASSERT(rkWrite != NULL);

    do {
        /* 1. Construct a KSF to store the RK. */
        if (rkMem != NULL) {
            (void)memcpy_s(rkWrite, sizeof(KmcKsfRk), rkMem, sizeof(KmcKsfRk));
        } else {
            /* Host sequence. You can directly cancel the protection of the protected memory. */
            (void)memcpy_s(rkWrite, sizeof(KmcKsfRk), &ksfMem->rk, sizeof(KmcKsfRk));
            errorCode = KmcUnprotectRkMaterials(rkWrite);
            if (errorCode != WSEC_SUCCESS) {
                break;
            }
        }

        rkWrite->mkNum = (WsecUint32)WsecArrGetCount(ksfMem->mkArray);
        rkWrite->updateCounter = ksfMem->updateCounter;
        rkWrite->sharedMkUpdateCounter = ksfMem->sharedMkUpdateCounter;
        ksfVersion = rkWrite->rkAttributes.version;
        if (rkWrite->rkAttributes.version == KMC_KSF_VER_V3) {
            errorCode = WsecHwGetCipherLen(WSEC_MK_PLAIN_LEN_MAX, &v3CiphertextLen);
            if (errorCode != WSEC_SUCCESS) {
                break;
            }
            /*
             * No extra space is required if the hardware encryption result can be stored in the V2 structure or
             * encrypted using the software-layer root
             * key (AES-GCM, 112-byte encryption result must be less than 128 bytes).
             * If the hardware is used to encrypt the MK and the hardware encryption result cannot be stored in
             * the V2 structure, extra space needs to be allocated to store the hardware ciphertext.
             */
            if (v3CiphertextLen < WSEC_MK_LEN_MAX || PriKmcSysGetHasSoftLevelRk() == WSEC_TRUE) {
                rkWrite->mkRecordLen = sizeof(KmcKsfMk);
            } else {
                rkWrite->mkRecordLen = (WsecUint32)(sizeof(KmcKsfMk) + v3CiphertextLen - WSEC_MK_LEN_MAX);
            }
        }

        if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
            CvtByteOrderForKsfRk(rkWrite, WBCHOST2NETWORK, ksfVersion);
        }
        /* Hash of the root key */
        WSEC_BUFF_ASSIGN(buffArray[0], rkWrite, (sizeof(KmcKsfRk) - sizeof(rkWrite->aboveHash)));
        WSEC_BUFF_ASSIGN(aboveHash, rkWrite->aboveHash, sizeof(rkWrite->aboveHash));
        if (WsecCreateHashCode(WSEC_ALGID_SHA256, buffArray, 1, &aboveHash) == WSEC_FALSE) {
            WSEC_LOG_E("WsecCreateHashCode() fail");
            errorCode = WSEC_ERR_GEN_HASH_CODE_FAIL;
            break;
        }
        if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
            /* Restore the byte order, which is required later. */
            CvtByteOrderForKsfRk(rkWrite, WBCNETWORK2HOST, ksfVersion);
        }

        /*
         * 2. Allocate KSF MK and RMK and derive RMK. Do not execute the last invoking statement.
         * The return value is determined outside while.
         */
        errorCode = AllocMkRecordAndDeriveRmk(rkWrite, mkKsf, rmkBuff);
    } while (0);

    return errorCode;
}

/* Writes non-HMAC data to the KSF. */
static unsigned long WriteKsfData(WsecHandle keystore, WsecHandle hashCtx, const WsecVoid *buff, WsecUint32 len)
{
    WSEC_ASSERT(keystore != NULL);
    WSEC_ASSERT(hashCtx != NULL);
    if (len == 0) {
        return WSEC_SUCCESS;
    }
    if (!WSEC_FWRITE_MUST(buff, len, keystore)) {
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    return CacDigestUpdate(hashCtx, buff, len);
}

/* Writing Hardware Root Key(hrk) info */
static unsigned long WriteHardwareRootKeyInfo(const WsecHandle keystore,
    WsecUint16 ksfVersion,
    const KmcKsfHardRk *hardRk,
    WsecHandle hashCtx)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 len;

    if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
        len = WSEC_H2N_L(hardRk->hrkInfo.len);
        if (WriteKsfData(keystore, hashCtx, &len, (WsecUint32)sizeof(WsecUint32)) != WSEC_SUCCESS) {
            return WSEC_ERR_WRI_FILE_FAIL;
        }
        if (WriteKsfData(keystore, hashCtx, hardRk->hrkInfo.buff, hardRk->hrkInfo.len)) {
            return WSEC_ERR_WRI_FILE_FAIL;
        }
        len = WSEC_H2N_L(hardRk->srkInfo.len);
        if (WriteKsfData(keystore, hashCtx, &len, (WsecUint32)sizeof(WsecUint32)) != WSEC_SUCCESS) {
            return WSEC_ERR_WRI_FILE_FAIL;
        }
        if (WriteKsfData(keystore, hashCtx, hardRk->srkInfo.buff, hardRk->srkInfo.len)) {
            return WSEC_ERR_WRI_FILE_FAIL;
        }
    }

    return ret;
}

/* Calculate KSF MK HMAC */
static unsigned long DoHmacForKsfV12Mk(KmcKsfMk *mkKsf, WsecBuff rmkBuff, WsecUint16 ksfVersion)
{
    WsecUint32 tempPlainLen;
    /* 4 buffers are used for hashing. */
    WsecBuffConst temp[4] = { { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 } };
    WsecUint32 buffCount = 0;
    WsecBuff hmacData = { NULL, 0 };
    WsecBuff hmacKey = { NULL, 0 };
    unsigned long ret;
    WSEC_ASSERT(IsKsfV1OrV2(ksfVersion));
    tempPlainLen = mkKsf->v2orV3.mkV2.mkRear.plaintextLen;
    /* Calculate the hash value of the MK. */
    if (ksfVersion == KMC_KSF_VER_V2) {
        /* Converts the first byte order of the HMAC into the network order. */
        CvtByteOrderForKsfMk(mkKsf, WBCHOST2NETWORK, ksfVersion);
    }
    WSEC_BUFF_ASSIGN(temp[buffCount], &mkKsf->mkInfo, sizeof(mkKsf->mkInfo));
    buffCount++;
    if (ksfVersion == KMC_KSF_VER) {
        WSEC_BUFF_ASSIGN(temp[buffCount], &mkKsf->cipherAlgId, sizeof(mkKsf->cipherAlgId) + sizeof(mkKsf->reserve));
        buffCount++;
    }
    WSEC_BUFF_ASSIGN(temp[buffCount], &mkKsf->v2orV3.mkV2.mkRear.plaintextLen, sizeof(mkKsf->v2orV3.mkV2.mkRear.plaintextLen));
    buffCount++;
    WSEC_BUFF_ASSIGN(temp[buffCount], mkKsf->v2orV3.mkV2.mkRear.key, tempPlainLen);
    buffCount++;
    WSEC_BUFF_ASSIGN(hmacData, mkKsf->v2orV3.mkV2.mkHash, sizeof(mkKsf->v2orV3.mkV2.mkHash));
    WSEC_BUFF_ASSIGN(hmacKey, ((unsigned char *)rmkBuff.buff + KMC_RMK_LEN), KMC_RMK_LEN);
    /* Calculate HMAC */
    ret = WsecCreateHmacCode(KMC_HMAC_MK_ALGID, temp, buffCount, &hmacKey, &hmacData);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Create HMAC fail.");
        return ret;
    }

    if (ksfVersion == KMC_KSF_VER_V2) {
        /* The network sequence is changed back to the host sequence. */
        CvtByteOrderForKsfMk(mkKsf, WBCNETWORK2HOST, ksfVersion);
    }
    return WSEC_SUCCESS;
}

/* The memory MK is used to construct KSF MK records of V1 and V2. */
static unsigned long MakeV1V2KsfMk(const KmcMemMk *mkMem, KmcKsfMk *mkKsf, WsecBuff rmkBuff, WsecUint16 ksfVersion)
{
    unsigned long ret;
    unsigned char key[WSEC_MK_LEN_MAX];
    WSEC_ASSERT(rmkBuff.buff != NULL);
    WSEC_ASSERT(rmkBuff.len == KMC_RMK_DOUBLE_LEN); /* 2: EK+IK */
    do {
        (void)memcpy_s(&mkKsf->mkInfo, sizeof(mkKsf->mkInfo), &mkMem->mkInfo, sizeof(mkMem->mkInfo));
        (void)memcpy_s(&mkKsf->v2orV3.mkV2.mkRear, sizeof(mkKsf->v2orV3.mkV2.mkRear), &mkMem->mkRear, sizeof(mkMem->mkRear));
        ret = CacRandom(mkKsf->iv, (WsecUint32)sizeof(mkKsf->iv)); /* Use random numbers for encryption and use IVs. */
        if (ret != WSEC_SUCCESS) {
            break;
        }
        mkKsf->cipherAlgId = KMC_ENCRYPT_MK_ALGID;
        /*
         * The key plaintext stored in the array is not the actual plaintext and is masked.
         * Therefore, the key plaintext needs to be unmasked.
         */
        ret = UnprotectData(mkMem->mkRear.key, mkMem->mkRear.plaintextLen,
            mkKsf->v2orV3.mkV2.mkRear.key, &mkKsf->v2orV3.mkV2.mkRear.plaintextLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = DoHmacForKsfV12Mk(mkKsf, rmkBuff, ksfVersion);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* Encrypt the MK plaintext using RMK. */
        mkKsf->ciphertextLen = WSEC_MK_LEN_MAX;
        (void)memcpy_s(key, (size_t)WSEC_MK_LEN_MAX, mkKsf->v2orV3.mkV2.mkRear.key, (size_t)WSEC_MK_LEN_MAX);
        ret = CacEncrypt(KMC_ENCRYPT_MK_ALGID, rmkBuff.buff, KMC_RMK_LEN,
            mkKsf->iv, (WsecUint32)sizeof(mkKsf->iv), key, mkKsf->v2orV3.mkV2.mkRear.plaintextLen,
            mkKsf->v2orV3.mkV2.mkRear.key, &mkKsf->ciphertextLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("CacEncrypt() fail. %lu", ret);
            ret = WSEC_ERR_ENCRPT_FAIL;
            break;
        }
        /* Converting the Byte Sequence to the Network Sequence Before Writing a File */
        CvtByteOrderForKsfMk(mkKsf, WBCHOST2NETWORK, ksfVersion);
    } while (0);
    (void)memset_s(key, (size_t)WSEC_MK_LEN_MAX, 0, (size_t)WSEC_MK_LEN_MAX);
    return ret;
}

/* The memory MK constructs the KSF MK record of the V3 version. */
static unsigned long MakeV3KsfMk(const KmcMemMk *mkMem, const KmcKsfHardRk *hardRk, WsecVoid *mkKsf,
    WsecUint32 *mkKsfLen)
{
    unsigned long errorCode;
    KmcKsfMk *mk = (KmcKsfMk *)mkKsf;
    unsigned char key[WSEC_MK_LEN_MAX];
    WsecUint32 ciphertextLen;
    WsecBuffConst plainBuff;

    WSEC_ASSERT(mkMem != NULL);
    WSEC_ASSERT(mkKsf != NULL);
    WSEC_ASSERT(mkKsfLen != NULL);
    WSEC_ASSERT(*mkKsfLen >= sizeof(KmcKsfMk));

    do {
        (void)memset_s(mk, sizeof(KmcKsfMk), 0, sizeof(KmcKsfMk));
        (void)memcpy_s(&mk->mkInfo, sizeof(mk->mkInfo), &mkMem->mkInfo, sizeof(mkMem->mkInfo));
        errorCode = CacRandom(mk->iv, (WsecUint32)sizeof(mk->iv)); /* Use random numbers for encryption and use IVs. */
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        /*
         * The key plaintext stored in the array is not the actual plaintext and is masked.
         * Therefore, the key plaintext needs to be unmasked.
         */
        mk->v2orV3.mkV3.plaintextLen = mkMem->mkRear.plaintextLen;
        errorCode = UnprotectData(mkMem->mkRear.key, mkMem->mkRear.plaintextLen, key, &mk->v2orV3.mkV3.plaintextLen);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        /*
         * Encrypt the MK plaintext using the hardware key or software layer root
         * key(hardware layer root key encryption protection).
         */
        if (PriKmcSysGetHasSoftLevelRk() == WSEC_TRUE) {
            mk->cipherAlgId = WSEC_ALGID_AES256_GCM;
            ciphertextLen = sizeof(mk->v2orV3.mkV3.key);
            plainBuff.buff = key;
            plainBuff.len = mk->v2orV3.mkV3.plaintextLen;
            errorCode = KmcEncBySrk(hardRk, mk->cipherAlgId,
                mk->iv, (WsecUint32)sizeof(mk->iv), &plainBuff,
                mk->v2orV3.mkV3.key, &ciphertextLen);
        } else {
            ciphertextLen = (WsecUint32)((*mkKsfLen - sizeof(KmcKsfMk)) + sizeof(mk->v2orV3.mkV3.key)); /* soter 554 */
            errorCode = KmcEncByHrk(hardRk, key, mk->v2orV3.mkV3.plaintextLen, mk->v2orV3.mkV3.key, &ciphertextLen);
        }
        if (errorCode != WSEC_SUCCESS) {
            WSEC_LOG_E1("MakeV3KsfMk enctypt fail, return value %lu", errorCode);
            break;
        }
        *mkKsfLen = (WsecUint32)((ciphertextLen > sizeof(mk->v2orV3.mkV3.key)) ?
            ((ciphertextLen - sizeof(mk->v2orV3.mkV3.key)) + sizeof(KmcKsfMk)) : sizeof(KmcKsfMk));
        mk->ciphertextLen = ciphertextLen;
        /* Converting the Byte Sequence to the Network Sequence Before Writing a File */
        CvtByteOrderForKsfMk(mk, WBCHOST2NETWORK, KMC_KSF_VER_V3);
    } while (0);
    (void)memset_s(key, sizeof(key), 0, sizeof(key));
    return errorCode;
}

/* The memory MK constructs a KSF MK record. */
static unsigned long MakeOneKsfMk(const KmcMemMk *mkMem,
    WsecBuff rmkBuff,
    const KmcKsfHardRk *hardRk,
    WsecUint16 ksfVersion,
    WsecVoid *mkKsf,
    WsecUint32 *mkKsfLen)
{
    unsigned long ret;
    WSEC_ASSERT(mkMem != NULL);
    WSEC_ASSERT(mkKsf != NULL);
    WSEC_ASSERT(mkKsfLen != NULL);
    WSEC_ASSERT(*mkKsfLen >= sizeof(KmcKsfMk));
    WSEC_ASSERT(WSEC_IS3(ksfVersion, KMC_KSF_VER, KMC_KSF_VER_V2, KMC_KSF_VER_V3));
    if (ksfVersion == KMC_KSF_VER || ksfVersion == KMC_KSF_VER_V2) {
        ret = MakeV1V2KsfMk(mkMem, (KmcKsfMk *)mkKsf, rmkBuff, ksfVersion);
    } else {
        ret = MakeV3KsfMk(mkMem, hardRk, mkKsf, mkKsfLen);
    }
    return ret;
}

/* Write to KSF */
static unsigned long WriteToKsf(WsecHandle ksf,
    WsecHandle hashCtx,
    const KmcKsfMem *ksfMem,
    KmcKsfRk *rkWrite,
    const WsecBuff *mkKsf,
    WsecBuff rmkBuff)
{
    unsigned long ret;
    WsecUint16 ksfVersion = rkWrite->rkAttributes.version;
    int mkNum;
    int i;
    KmcMemMk *memMk = NULL;
    unsigned int mkKsfLen;
    /* Writes the RK to a file. */
    ret = WriteKsfData(ksf, hashCtx, g_ksfFlag, (WsecUint32)sizeof(g_ksfFlag));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Write flag fail %lu", ret);
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    CvtByteOrderForKsfRk(rkWrite, WBCHOST2NETWORK, ksfVersion);
    ret = WriteKsfData(ksf, hashCtx, rkWrite, (WsecUint32)sizeof(KmcKsfRk));
    CvtByteOrderForKsfRk(rkWrite, WBCNETWORK2HOST, ksfVersion);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Write Rk fail %lu", ret);
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    ret = WriteHardwareRootKeyInfo(ksf, ksfVersion, &ksfMem->hardRk, hashCtx);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Write HardRk fail %lu", ret);
        return ret;
    }

    /* Write each MK into a file. */
    mkNum = WsecArrGetCount(ksfMem->mkArray);
    for (i = 0; i < mkNum; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(ksfMem->mkArray, i);
        if (memMk == NULL) {
            WSEC_LOG_W("memMk is NULL");
            continue;
        }

        mkKsfLen = mkKsf->len;
        ret = MakeOneKsfMk(memMk, rmkBuff, &ksfMem->hardRk, ksfVersion, mkKsf->buff, &mkKsfLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("make mk fail %lu", ret);
            break;
        }
        ret = WriteKsfData(ksf, hashCtx, mkKsf->buff, mkKsfLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Write mk fail %lu", ret);
            break;
        }
        if ((i != 0) && (i % (WSEC_EVENT_PERIOD) == 0)) {
            WSEC_DO_EVENTS;
        }
    }
    return ret;
}

/* For OceanStor V3, write FLUSH KSF after KSF HMAC. */
static unsigned long WriteV3KsfHmacAndFlushKsf(WsecHandle ksf,
    const KmcKsfMem *ksfMem,
    WsecUint16 ksfVersion,
    KmcKsfHmac *ksfHmac,
    WsecUint32 hashLen)
{
    unsigned long ret;
    WsecUint32 hmacLen = WSEC_HMAC_LEN_MAX;
    if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
        ret = KmcHardRkGetKsfHmac(&ksfMem->hardRk, WSEC_ALGID_HMAC_SHA256, ksfHmac, hashLen, &hmacLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("KmcHardRkGetKsfHmac fail %lu", ret);
            return ret;
        }
        if (!WSEC_FWRITE_MUST(ksfHmac, sizeof(KmcKsfHmac), ksf)) {
            WSEC_LOG_E1("WriteV3KsfHmacAndFlushKsf WSEC_FWRITE_MUST failed, errno %d", WSEC_FERRNO(ksf));
            return WSEC_ERR_WRI_FILE_FAIL;
        }
    }

    if (WSEC_FFLUSH(ksf) != 0) {
        WSEC_LOG_E1("WriteV3KsfHmacAndFlushKsf WSEC_FFLUSH failed, errno %d", WSEC_FERRNO(ksf));
        return WSEC_ERR_FILE_FLUSH_FAIL;
    }
    return WSEC_SUCCESS;
}

static void FinalWriteKsf(WsecHandle hashCtx, WsecHandle ksf, WsecBuff *rmkBuff, WsecBuff *mkKsf)
{
    WSEC_ASSERT(rmkBuff != NULL);
    WSEC_ASSERT(mkKsf != NULL);

    CacDigestReleaseCtx(&hashCtx);
    WSEC_FCLOSE(ksf);
    WSEC_CLEAR_FREE(rmkBuff->buff, rmkBuff->len);
    WSEC_CLEAR_FREE(mkKsf->buff, mkKsf->len);
}

/* Write the Keystore file,use the rk to encrypt the MK in keystoreData, save the encrypted MK to the Keystore file. */
static unsigned long WriteKsf(const KmcKsfMem *ksfMem, const char *file, const KmcKsfRk *rkNew, KmcKsfRk *rkWrite,
    const WsecBuff *ksfHash, const char *callBy)
{
    unsigned long ret;
    WsecHandle ksf = NULL;
    WsecBuff rmkBuff = { NULL, 0 };
    WsecBuff mkKsf = { NULL, 0 };
    WsecHandle hashCtx = NULL;
    WsecUint32 hashLen = WSEC_HASH_LEN_MAX;
    KmcKsfHmac ksfHmac;

    if (!(ksfMem != NULL && file != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }

    do {
        ret = PrepareForWriteKsf(rkNew, ksfMem, rkWrite, &mkKsf, &rmkBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ksf = WSEC_FOPEN(file, KMC_FILE_WRITE_BINARY);
        if (ksf == NULL) {
            WSEC_LOG_E("Open file fail");
            ret = WSEC_ERR_OPEN_FILE_FAIL;
            break;
        }
        ret = CacDigestInit(&hashCtx, WSEC_ALGID_SHA256);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* Write a file. */
        ret = WriteToKsf(ksf, hashCtx, ksfMem, rkWrite, &mkKsf, rmkBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CacDigestFinal(&hashCtx, ksfHmac.HashData.hashData, &hashLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = (memcpy_s(ksfHash->buff, (size_t)WSEC_HASH_LEN_MAX, ksfHmac.HashData.hashData, (size_t)WSEC_HASH_LEN_MAX) == EOK)
            ? WSEC_SUCCESS : WSEC_ERR_MEMCPY_FAIL;
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("copy data failed,error code: %lu", ret);
            break;
        }

        ret = WriteV3KsfHmacAndFlushKsf(ksf, ksfMem, rkWrite->rkAttributes.version, &ksfHmac, hashLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I3("%s write ksf at %llx with update counter %u", callBy, ksfHmac.HashData.partHash, ksfMem->updateCounter);
    } while (0);

    FinalWriteKsf(hashCtx, ksf, &rmkBuff, &mkKsf);
    return ret;
}

/* KSM generates KSF */
unsigned long GenKsf(KmcKsfMem *ksfMem, const char *keystoreFile, const char *callBy)
{
    WSEC_ASSERT(ksfMem != NULL);
    WSEC_ASSERT(keystoreFile != NULL);
    unsigned long ret;
    KmcKsfRk *rkWrite = (KmcKsfRk *)WSEC_MALLOC(sizeof(KmcKsfRk));
    unsigned char hashValue[WSEC_HASH_LEN_MAX] = {0};
    WsecBuff ksfHash;
    WSEC_BUFF_ASSIGN(ksfHash, hashValue, WSEC_HASH_LEN_MAX);
    if (rkWrite == NULL) {
        WSEC_LOG_E("WSEC_MALLOC failed KmcKsfRk");
        return WSEC_ERR_MALLOC_FAIL;
    }
    ret = WriteKsf(ksfMem, keystoreFile, NULL, rkWrite, &ksfHash, callBy);
    if (ret != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(rkWrite, sizeof(KmcKsfRk));
        return ret;
    }
    ret = KmcProtectRkMaterials(rkWrite);
    if (ret != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(rkWrite, sizeof(KmcKsfRk));
        return ret;
    }
    (void)memcpy_s(&ksfMem->rk, sizeof(KmcKsfRk), rkWrite, sizeof(KmcKsfRk));
    ret = (memcpy_s(ksfMem->ksfHash, (size_t)WSEC_HASH_LEN_MAX, ksfHash.buff, (size_t)WSEC_HASH_LEN_MAX) == EOK)
        ? WSEC_SUCCESS : WSEC_ERR_MEMCPY_FAIL;
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WSEC_MALLOC failed KmcKsfRk, error code:%lu", ret);
        WSEC_CLEAR_FREE(rkWrite, sizeof(KmcKsfRk));
        return ret;
    }
    WSEC_CLEAR_FREE(rkWrite, sizeof(KmcKsfRk));
    return ret;
}

/* Safely Erasing KSF */
unsigned long SecureEraseKeystore(void)
{
    char *file = NULL;
    WsecUint32 i;
    unsigned long errorCode = WSEC_SUCCESS;
    ProcLockKeystore();
    /* CBB in WSEC_ON_INIT or WSEC_RUNNING can delete KSF
     * WSEC_RUNNING : KMC already initialized
     * WSEC_ON_INIT : KMC is on initialize
     * */
    if (!(PriKmcSysGetState() == WSEC_RUNNING ||
        PriKmcSysGetState() == WSEC_ON_INIT)) {
        ProcUnlockKeystore();
        return WSEC_ERR_KMC_CBB_NOT_INIT;
    }
    for (i = 0; i < KMC_KSF_NUM; i++) {
        file = PriKmcSysGetKsf(i);
        if (file == NULL) {
            continue;
        }

        if (!WsecDeleteFileSafe(file, KMC_MAX_DELETE_KSF_LENGTH)) {
            errorCode = WSEC_FAILURE;
            WSEC_LOG_E("WsecDeleteFileSafe fail.");
        }
    }
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        file = PriKmcSysGetKsfBackupKsf();
        if (!WsecDeleteFileSafe(file, KMC_MAX_DELETE_KSF_LENGTH)) {
            errorCode = WSEC_FAILURE;
            WSEC_LOG_E("WsecDeleteFileSafe backup file fail.");
        }
    }
    ProcUnlockKeystore();
    return errorCode;
}

/* KSF Get Update Times */
static unsigned long GetUpdateNumberFromKeystore(WsecBool ksfNumber, const char *keystoreFile,
    WsecUint32 *updateCounter)
{
    unsigned long ret;
    KmcKsfMem *readBuff = NULL;
    readBuff = (KmcKsfMem *)WSEC_MALLOC(sizeof(KmcKsfMem));
    if (readBuff == NULL) {
        WSEC_LOG_E4MALLOC(sizeof(KmcKsfMem));
        return WSEC_ERR_MALLOC_FAIL;
    }
    (void)memset_s(readBuff, sizeof(KmcKsfMem), 0, sizeof(KmcKsfMem));

    ProcLockKeystore();
    do {
        if (PriKmcSysGetState() != WSEC_RUNNING) {
            WSEC_LOG_E("KMC not running.");
            ret = WSEC_ERR_KMC_CBB_NOT_INIT;
            break;
        }
        ret = ReadKsf(keystoreFile, __FUNCTION__, readBuff);
        /* Reads the updated version number in a specified keystore file. */
        if (ret != WSEC_SUCCESS) {
            break;
        }
        *updateCounter = (ksfNumber == WSEC_TRUE) ? readBuff->updateCounter : readBuff->sharedMkUpdateCounter;
    } while (0);
    ProcUnlockKeystore();

    (void)FreeKsfSnapshot(readBuff);

    return ret;
}

/* KSF Get KSF Update Times */
unsigned long GetKsfUpdateNumberFromKeystore(const char *keystoreFile, WsecUint32 *updateCounter)
{
    return GetUpdateNumberFromKeystore(WSEC_TRUE, keystoreFile, updateCounter);
}

/* Number of Times the KSF Obtains Shared MK Updates */
unsigned long GetSharedMkUpdateNumberFromKsf(const char *keystoreFile, WsecUint32 *updateCounter)
{
    return GetUpdateNumberFromKeystore(WSEC_FALSE, keystoreFile, updateCounter);
}


/* If the input parameter is empty,no file is valid and needs to be created again. Otherwise, the file name is valid. */
static unsigned long RewriteOnCheckFail(KmcKsfMem * const *readBuff,
    WsecUint32 blockSize,
    const KmcCfg *kmcCfg,
    KmcKsfMem *ksfMem,
    unsigned long *returnValue)
{
    unsigned long ret;
    KmcKsfRk rkNew;
    KmcKsfHardRk hardRk;
    WsecUint16 ksfVersion;
    WsecUint32 aimAt = KMC_KSF_WITH_THIRD_NUM;

    do {
        ret = IsAllKsfCorrupt(readBuff, blockSize, &aimAt, returnValue);
        if (ret != WSEC_SUCCESS) {
            WSEC_NOTIFY(WSEC_KMC_NTF_KEY_STORE_CORRUPT, NULL, 0);
            WSEC_LOG_E("All file is corrupt");
            break;
        }
        ret = RecoverFromAimFile(readBuff, &aimAt);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Recover from ok file failed %lu", ret);
        }
        return ret;
    } while (0);

    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
    do {
        ksfVersion = ksfMem->rk.rkAttributes.version;
        KmcAssignKsfHardRk(&ksfMem->hardRk, &hardRk);
        ret = KmcCreateRootKey((WsecBuffConst *)NULL, ksfVersion, kmcCfg, &rkNew, &ksfMem->hardRk);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /*
         * In this case, shareMkUpdateCounter remains unchanged because
         * the original global g_keystore is still used to write files.
         */
        ret = WriteKsfSafetyLockable(WSEC_FALSE, &rkNew, ksfMem, WSEC_FALSE, __FUNCTION__);
        if (ret != WSEC_SUCCESS) {
            break;
        }
    } while (0);
    (void)memset_s(&rkNew, sizeof(KmcKsfRk), 0, sizeof(KmcKsfRk));
    if (ret == WSEC_SUCCESS) {
        KmcKsfHardRkRmvFree(&hardRk);
    } else {
        KmcKsfHardRkRmvFree(&ksfMem->hardRk);
        KmcAssignKsfHardRk(&hardRk, &ksfMem->hardRk);
    }
    (void)memset_s(&hardRk, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));

    return ret;
}
