/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: KMC management function implementation
 * Author: z00316590
 * Create: 2018-08-08
 */

#include "wsecv2_itf.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_share.h"
#include "wsecv2_util.h"
#include "wsecv2_order.h"
#include "wsecv2_lock.h"
#include "kmcv2_pri.h"

/* Functions that are periodically invoked */
static WsecFuncRegStatus g_cbbFuncRegState = { WSEC_FUNC_UNREG };

/* Registers the callback function registered by the app. */
unsigned long WsecRegFuncEx(const WsecCallbacks *allCallbacks)
{
    unsigned long ret;

    g_cbbFuncRegState.state = WSEC_FUNC_UNREG;
    if (allCallbacks == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    /* 1. Memory (optional) */
    ret = WsecSetMemCallbacks(&allCallbacks->memCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* 2. File operation (mandatory) */
    ret = WsecSetFileCallbacks(&allCallbacks->fileCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* 3. Strongly dependent on apps and mandatory functions */
    ret = WsecSetBasicRelyCallbacks(&allCallbacks->basicRelyCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* 4. Lock operation (forcible) */
    ret = WsecSetThreadLockCallbacks(&allCallbacks->lockCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* 5. Process lock operation (forcible) */
    ret = WsecSetProcLockCallbacks(&allCallbacks->procLockCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    /* 6. Random number (mandatory) */
    ret = WsecSetRngCallbacks(&allCallbacks->rngCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* 7. Time (mandatory) */
    ret = WsecSetTimeCallbacks(&allCallbacks->timeCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* 8. Hardware (conditional mandatory class, for example, hardware-based root key protection mandatory class) */
    ret = WsecSetHardwareCallbacks(&allCallbacks->hardwareCallbacks);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    g_cbbFuncRegState.state = WSEC_FUNC_REG;
    WSEC_LOG_I("WsecRegFuncEx success");
    return WSEC_SUCCESS;
}

/* Specified master or agent */
unsigned long WsecSetRole(WsecUint32 roleType)
{
    return KmcSetRoleType(roleType);
}

/*
 * Initialization function. The master or agent must be specified.
 * The paths of the active and standby files of the keystore must be specified. The keystone is reserved.
 */
static unsigned long InitializeKmc(const WsecInternalInitParam *initParam)
{
    WSEC_ASSERT(initParam != NULL);
    unsigned long ret;
    if (g_cbbFuncRegState.state == WSEC_FUNC_UNREG) {
        return WSEC_ERR_CALLBACKS_NOT_REG;
    }
    /* 2. Determine the CPU byte order. */
    WsecCheckCpuEndianMode();
    /* 3. Initialize the lock. */
    ret = WsecInitializeLock();
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WsecInitializeLock failed with %lu.", ret);
        return ret;
    }
    WSEC_LOG_I("WSEC locks initialized successful.");

    /* 4. Initializes the KMC if the SDP/KMC is enabled. */
    ret = KmcInitializeEx(initParam, KMC_NEED_LOCK);
    if (ret == WSEC_SUCCESS) {
        WSEC_LOG_I("WSEC initialized successful.");
    } else if (ret == WSEC_ERR_KMC_INI_MUL_CALL) {
        WSEC_LOG_W("WSEC initialized already.");
    } else {
        WSEC_LOG_E("WSEC initialize failed.");
        WsecFinalizeLock();
    }

    return ret;
}

/*
 * Initialization function. The master or agent must be specified.
 * The paths of the active and standby files of the keystore must be specified. The keystone is reserved.
 */
unsigned long WsecInitializeEx(WsecUint32 roleType,
    const KmcKsfName *filePathName,
    WsecBool useImportKey,
    WsecVoid *exParam)
{
    (void)useImportKey;
    WsecExtendInitParam *param = (WsecExtendInitParam *)exParam;
    WsecInternalInitParam initParam;
    initParam.roleType              = roleType;
    initParam.filePathName          = filePathName;
    initParam.enableThirdBackup     = (param == NULL) ? WSEC_FALSE : param->enableThirdBackup;
    initParam.deleteKsfOnInitFailed = (param == NULL) ? WSEC_FALSE : param->deleteKsfOnInitFailed;
    initParam.enableHw              = WSEC_FALSE;
    initParam.hdParm.hasSoftLevelRk = WSEC_FALSE;
    initParam.hdParm.hdCtx.buff     = NULL;
    initParam.hdParm.hdCtx.len      = 0;
    return InitializeKmc(&initParam);
}

/*
 * Initialization function. The root key is protected by hardware. This function is added in V3.
 * If the system boots from this function, the V3 data structure is used and rollback is not supported.
 */
unsigned long WsecInitializeHw(WsecUint32 roleType,
    const KmcKsfName *filePathName,
    WsecBool hasSoftLevelRk,
    const WsecVoid *hardwareParam, WsecUint32 hardwareParamLen,
    WsecVoid *exParam)
{
    WsecExtendInitParam *param = (WsecExtendInitParam *)exParam;
    WsecInternalInitParam initParam;
    initParam.roleType              = roleType;
    initParam.filePathName          = filePathName;
    initParam.enableThirdBackup     = (param == NULL) ? WSEC_FALSE : param->enableThirdBackup;
    initParam.deleteKsfOnInitFailed = (param == NULL) ? WSEC_FALSE : param->deleteKsfOnInitFailed;
    initParam.enableHw              = WSEC_TRUE;
    initParam.hdParm.hasSoftLevelRk = hasSoftLevelRk;
    initParam.hdParm.hdCtx.buff     = hardwareParam;
    initParam.hdParm.hdCtx.len      = hardwareParamLen;
    return InitializeKmc(&initParam);
}


/* Deinitializes a function. */
unsigned long WsecFinalizeEx(void)
{
    unsigned long returnValue;
    returnValue = KmcFinalizeEx();
    WsecFinalizeLock();
    WSEC_LOG_I("WSEC finalized.");

    return returnValue;
}

/*
 * Function for reloading keystore file.This function is used to reload files from the keystore file in the process.
 * Note: After this function is called, the configuration is restored to the default configuration.
 * If the default configuration is not used, the app needs to reconfigure the configuration.
 */
unsigned long WsecResetEx(void)
{
    WSEC_LOG_I("WSEC CBB Reset.");
    WsecBuffConst hardwareParam = { NULL, 0 };
    return KmcResetEx(WSEC_FALSE, hardwareParam);
}

/* V3 reset */
unsigned long WsecResetHw(const WsecVoid *hardwareParam, WsecUint32 hardwareParamLen)
{
    WsecBuffConst hardwareBuff = { NULL, 0 };
    hardwareBuff.buff = hardwareParam;
    hardwareBuff.len = hardwareParamLen;
    return KmcResetEx(WSEC_TRUE, hardwareBuff);
}

/* Obtains the current version number. */
const char *WsecGetVersion(void)
{
    const char *versionInfo = KMC_VERSION;
    return versionInfo;
}

#ifdef WSEC_DEBUG
/* Indicates the display structure length, which is used for commissioning in different environments. */
WsecVoid WsecShowStructSize(CallbackShowStructSize showStructures)
{
    showStructures("unsigned char", sizeof(unsigned char));
    showStructures("WsecUint16", sizeof(WsecUint16));
    showStructures("WsecUint32", sizeof(WsecUint32));
    showStructures("int", sizeof(int));
    showStructures("WsecBool", sizeof(WsecBool));
    showStructures("char", sizeof(char));
    showStructures("WsecHandle", sizeof(WsecHandle));
    showStructures("size_t", sizeof(size_t));
    showStructures("unsigned long", sizeof(unsigned long));
    showStructures("WsecVoid *", sizeof(WsecVoid *));
    showStructures("long", sizeof(long));

    showStructures("WsecSysTime", sizeof(WsecSysTime));

    showStructures("KmcMkInfo", sizeof(KmcMkInfo));
    showStructures("KmcCfgRootKey", sizeof(KmcCfgRootKey));
    showStructures("KmcCfgKeyManagement", sizeof(KmcCfgKeyManagement));
    showStructures("KmcCfgDataProtect", sizeof(KmcCfgDataProtect));
    showStructures("KmcCfgKeyType", sizeof(KmcCfgKeyType));
    showStructures("KmcCfgDomainInfo", sizeof(KmcCfgDomainInfo));
    showStructures("KmcRkParameters", sizeof(KmcRkParameters));
    showStructures("KmcKsfRk", sizeof(KmcKsfRk));
    showStructures("KmcMkRear", sizeof(KmcMkRear));
    showStructures("KmcKsfMk", sizeof(KmcKsfMk));
    showStructures("KmcMemMk", sizeof(KmcMemMk));
    showStructures("KmcMkfHeader", sizeof(KmcMkfHeader));
    showStructures("KmcMkfHeaderWithHmac", sizeof(KmcMkfHeaderWithHmac));
    showStructures("KmcMkFileOneMk", sizeof(KmcMkFileOneMk));
    showStructures("KmcSyncMk", sizeof(KmcSyncMk));
    showStructures("KmcSyncHeader", sizeof(KmcSyncMkHeader));
}
#endif
