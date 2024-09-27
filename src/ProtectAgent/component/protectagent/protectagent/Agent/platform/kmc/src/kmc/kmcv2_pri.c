/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * 2019-04-09 Zhang Jie (employee ID: 00316590): Added some interfaces to adapt to KMC 3.0.
 */

#include "kmcv2_pri.h"
#include "securec.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"
#include "wsecv2_order.h"
#include "wsecv2_datetime.h"
#include "wsecv2_errorcode.h"

/* Managing Global Status */
static KmcSys g_kmcSys = { {NULL}, 0, 0, WSEC_FALSE, WSEC_FALSE, WSEC_FALSE, WSEC_FALSE, NULL};

/* Sets whether to enable hardware root key protection (corresponding to KSFV3). */
static WsecVoid PriKmcSysSetIsHardware(WsecBool isHardware)
{
    g_kmcSys.isHardware = isHardware;
}

/* Whether the root key of the software layer exists */
static WsecVoid PriKmcSysSetHasSoftLevelRk(WsecBool hasSoftLevelRk)
{
    g_kmcSys.hasSoftLevelRk = hasSoftLevelRk;
}

/* Sets the current KMC status. */
WsecVoid PriKmcSysSetState(WsecUint32 state)
{
    g_kmcSys.state = state;
}

/* set KMC current role */
WsecVoid PriKmcSysSetRole(WsecUint32 role)
{
    g_kmcSys.role = role;
    WSEC_LOG_I1("WSEC role is set to the %u", role);
}

/* enable the third ksf backup file */
WsecVoid PriKmcSysSetEnableThirdBackup(WsecBool isEnable)
{
    g_kmcSys.enableThirdBackup = isEnable;
}


/* delete ksf backup file when init failed */
WsecBool PriKmcSysGetIsDeleteKsfOnInitFailed(void)
{
    return g_kmcSys.deleteKsfOnInitFailed;
}

/* set delete ksf on startup feature when thrid backup is enabled */
WsecVoid PriKmcSysSetDeleteKsfOnInitFailed(WsecBool isEnable)
{
    g_kmcSys.deleteKsfOnInitFailed = isEnable;
}

/* Initializes the PRI module.The PRI module manages KmcSys structure and provides global information for the KMC. */
unsigned long PriKmcSysInit(WsecUint32 state, const WsecInternalInitParam *initParam)
{
    WSEC_ASSERT(initParam != NULL);
    unsigned long ret;

    PriKmcSysSetState(state);
    PriKmcSysSetIsHardware(initParam->enableHw);
    PriKmcSysSetHasSoftLevelRk(initParam->hdParm.hasSoftLevelRk);
    PriKmcSysSetRole(initParam->roleType);
    PriKmcSysSetEnableThirdBackup(initParam->enableThirdBackup);
    PriKmcSysSetDeleteKsfOnInitFailed(initParam->deleteKsfOnInitFailed);
    /* enableThirdBackup and deleteKsfOnInitFailed flag must set before ksf file init */
    ret = PriKmcSysInitKsfName(initParam->filePathName);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Init key store file failed");
        return ret;
    }
    return WSEC_SUCCESS;
}

/* Obtain the KSF name based on the KSF index. */
char *PriKmcSysGetKsf(WsecUint32 idx)
{
    if (idx < KMC_KSF_NUM) {
        return g_kmcSys.keystoreFile[idx];
    }
    return NULL;
}

/* get the third ksf backup file name */
char *PriKmcSysGetKsfBackupKsf(void)
{
    return g_kmcSys.keystoreBackupFile;
}

/* Whether enabled the third ksf backup file */
WsecBool PriKmcSysGetIsEnableThirdBackup(void)
{
    return g_kmcSys.enableThirdBackup;
}

/* Obtains whether hardware protection is enabled (corresponding to the V3 KSF). */
WsecBool PriKmcSysGetIsHardware(void)
{
    return g_kmcSys.isHardware;
}

/* Obtaining the Current KMC Status */
WsecUint32 PriKmcSysGetState(void)
{
    return g_kmcSys.state;
}

/* Obtaining the Current KMC Role */
WsecUint32 PriKmcSysGetRole(void)
{
    return g_kmcSys.role;
}

/* Whether the root key of the software layer exists, which is determined by the initialization parameter */
WsecBool PriKmcSysGetHasSoftLevelRk(void)
{
    return g_kmcSys.hasSoftLevelRk;
}

/* Deinitializes the PRI module. */
WsecVoid PriKmcSysUninit(void)
{
    WSEC_FREE(g_kmcSys.keystoreFile[MASTER_KSF_INDEX]);
    WSEC_FREE(g_kmcSys.keystoreFile[BACKUP_KSF_INDEX]);
    WSEC_FREE(g_kmcSys.keystoreBackupFile);
    g_kmcSys.state = WSEC_WAIT_INIT;
    g_kmcSys.role = KMC_ROLE_AGENT;
    g_kmcSys.isHardware = WSEC_FALSE;
    g_kmcSys.hasSoftLevelRk = WSEC_FALSE;
    g_kmcSys.enableThirdBackup = WSEC_FALSE;
    g_kmcSys.deleteKsfOnInitFailed = WSEC_FALSE;
}

/* init keystore files name */
WsecUint32 PriKmcSysInitKsfName(const KmcKsfName *ksf)
{
    g_kmcSys.keystoreFile[MASTER_KSF_INDEX] = WSEC_CLONE_STR(ksf->keyStoreFile[MASTER_KSF_INDEX]);
    g_kmcSys.keystoreFile[BACKUP_KSF_INDEX] = WSEC_CLONE_STR(ksf->keyStoreFile[BACKUP_KSF_INDEX]);
    if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
        g_kmcSys.keystoreBackupFile = WSEC_CLONE_STR(ksf->keyStoreBackupFile);
    }

    if (g_kmcSys.keystoreFile[MASTER_KSF_INDEX] == NULL ||
        g_kmcSys.keystoreFile[BACKUP_KSF_INDEX] == NULL ||
        (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE && g_kmcSys.keystoreBackupFile == NULL)) {
        WSEC_LOG_E("Allocate ksf memory failed.");
        WSEC_FREE(g_kmcSys.keystoreFile[MASTER_KSF_INDEX]);
        WSEC_FREE(g_kmcSys.keystoreFile[BACKUP_KSF_INDEX]);
        /* Avoid mistaken free memory when disabled recovery but the backupFile is not null */
        if (PriKmcSysGetIsEnableThirdBackup() == WSEC_TRUE) {
            WSEC_FREE(g_kmcSys.keystoreBackupFile);
        }
        return WSEC_ERR_MALLOC_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Check whether the KSF version number is valid. */
WsecBool IsValidKsfVersion(WsecUint16 ksfVersion)
{
    return (WSEC_IS3(ksfVersion, KMC_KSF_VER, KMC_KSF_VER_V2, KMC_KSF_VER_V3) ? WSEC_TRUE : WSEC_FALSE);
}

/* KSF V3 or Not */
WsecBool IsKsfV3(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (ksfVersion == KMC_KSF_VER_V3) ? WSEC_TRUE : WSEC_FALSE;
}

/* KSF V1 or Not */
WsecBool IsKsfV1(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (ksfVersion != KMC_KSF_VER) ? WSEC_FALSE : WSEC_TRUE;
}

/* KSF V2 or Not */
WsecBool IsKsfV2(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (ksfVersion != KMC_KSF_VER_V2) ? WSEC_FALSE : WSEC_TRUE;
}

/* KSF V1 can only be selected from V1 and V2. */
WsecBool IsKsfV1AndNotV2(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(WSEC_IS2(ksfVersion, KMC_KSF_VER, KMC_KSF_VER_V2));
    return (ksfVersion == KMC_KSF_VER) ? WSEC_TRUE : WSEC_FALSE;
}

/* KSF V1 or V2 */
WsecBool IsKsfV1OrV2(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (WSEC_IS2(ksfVersion, KMC_KSF_VER, KMC_KSF_VER_V2) ? WSEC_TRUE : WSEC_FALSE);
}

/* KSF V2 or V3 */
WsecBool IsKsfV2OrV3(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (WSEC_IS2(ksfVersion, KMC_KSF_VER_V2, KMC_KSF_VER_V3) ? WSEC_TRUE : WSEC_FALSE);
}

/* whether support sync mk by interface, currently only the KSF V1 not support */
WsecBool IsSupportSyncMk(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (ksfVersion == KMC_KSF_VER) ? WSEC_FALSE : WSEC_TRUE;
}

/*
 * Calculated in network sequence (RK hash and MK HMAC V1 are calculated in host sequence,
 * and V2/V3 are calculated in network sequence.)
 */
WsecBool CalcByNetWorkOrder(WsecUint16 ksfVersion)
{
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    return (ksfVersion == KMC_KSF_VER) ? WSEC_FALSE : WSEC_TRUE;
}

/* The byte order of the MK information is converted. */
WsecVoid CvtByteOrderForMkInfo(KmcMkInfo *mkInfo, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (mkInfo == NULL) {
        return;
    }

    mkInfo->domainId = WSEC_BYTE_ORDER_CVT_L(direction, mkInfo->domainId);
    mkInfo->keyId = WSEC_BYTE_ORDER_CVT_L(direction, mkInfo->keyId);
    mkInfo->keyType = WSEC_BYTE_ORDER_CVT_S(direction, mkInfo->keyType);

    WsecCvtByteOrderForDateTime(&(mkInfo->mkCreateTimeUtc), direction);
    WsecCvtByteOrderForDateTime(&(mkInfo->mkExpiredTimeUtc), direction);
}

/* MK Synchronization Byte Sequence Conversion */
WsecVoid CvtByteOrderForSyncMk(KmcSyncMk *mk, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (mk == NULL) {
        return;
    }

    CvtByteOrderForMkInfo(&mk->mkInfo, direction);
    mk->mkRear.plaintextLen = WSEC_BYTE_ORDER_CVT_L(direction, mk->mkRear.plaintextLen);
}

/* MK Synchronization Header Byte Sequence Conversion */
WsecVoid WsecCvtByteOrderForSyncMkHeader(KmcSyncMkHeader *header, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (header == NULL) {
        return;
    }
    header->msgType = WSEC_BYTE_ORDER_CVT_S(direction, header->msgType);
    header->version = WSEC_BYTE_ORDER_CVT_S(direction, header->version);
    header->msgLen = WSEC_BYTE_ORDER_CVT_L(direction, header->msgLen);
    header->sharedMkUpdateCounter = WSEC_BYTE_ORDER_CVT_L(direction, header->sharedMkUpdateCounter);
}

/* Converts the byte order of the header information in the MKF. */
WsecVoid CvtByteOrderForMkfHdr(KmcMkfHeader *mkfHeader, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (mkfHeader == NULL) {
        return;
    }

    mkfHeader->version = WSEC_BYTE_ORDER_CVT_S(direction, mkfHeader->version);
    mkfHeader->ksfVersion = WSEC_BYTE_ORDER_CVT_S(direction, mkfHeader->ksfVersion);
    mkfHeader->cipherAlgId = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->cipherAlgId);
    mkfHeader->iterForEncKey = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->iterForEncKey);
    mkfHeader->hmacAlgId = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->hmacAlgId);
    mkfHeader->iterForHmacKey = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->iterForHmacKey);
    mkfHeader->cipherLenPerMk = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->cipherLenPerMk);
    mkfHeader->mkNum = WSEC_BYTE_ORDER_CVT_L(direction, mkfHeader->mkNum);
}

/* The MK information word in the MKF is converted into the stanza. */
WsecVoid CvtByteOrderForMkfMk(KmcMkFileOneMk *mkfMk, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    if (mkfMk == NULL) {
        return;
    }

    CvtByteOrderForMkInfo(&(mkfMk->mkInfo), direction);
    mkfMk->plaintextLen = WSEC_BYTE_ORDER_CVT_L(direction, mkfMk->plaintextLen);
}

/* compare the mk domainid with the input domainid */
KmcCompareResult KmcCompareDomain(WsecUint32 compareDomain, WsecUint32 toCompareDomain)
{
    if (toCompareDomain == KMC_ALL_DOMAIN) {
        return KMC_COMPARE_ALL;
    }
    if (compareDomain == toCompareDomain) {
        return KMC_COMPARE_EQ;
    }
    return KMC_COMPARE_NOT_EQ;
}

/* Set the key time. */
unsigned long SetLifeTime(WsecUint32 lifeDays, WsecSysTime *createUtc, WsecSysTime *expireUtc)
{
    if (((int)lifeDays) < 1) {
        WSEC_LOG_E1("lifeDays(%u) too big.", lifeDays);
        return WSEC_ERR_INVALID_ARG;
    }
    if (createUtc == NULL || expireUtc == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (!WsecGetUtcDateTime(createUtc)) {
        return WSEC_ERR_GET_CURRENT_TIME_FAIL;
    }
    return WsecDateTimeAddDay(createUtc, lifeDays, expireUtc) ? WSEC_SUCCESS : WSEC_FAILURE;
}
