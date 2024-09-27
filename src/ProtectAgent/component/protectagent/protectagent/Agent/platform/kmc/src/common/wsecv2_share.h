/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: internal interface of the KMC, which is not open to external systems
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_SHARE_H
#define KMC_SRC_COMMON_WSECV2_SHARE_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * hardwareParam struct :
 * hasSoftLevelRk   : indicate software level rk is uesed
 * hdCtx            : hdCtx.buff to hold hardwareParam for security hardware
 *                    hdctx.len to hold hardwareParam length for security hardware
 **/
typedef struct TagHardwareParam {
    WsecBool hasSoftLevelRk;
    WsecBuffConst hdCtx;
} KmcHardwareParam;

/**
 * roleType          : init roleType of kmc see KmcRoleType
 * KmcKsfName        : the KmcKsfName for ksf files. see KmcKsfName
 * enableThirdBackup : enable thrid backup file support
 * deleteKsfOnFailed : delete ksf when init failed(when enableThirdBackup set to true)
 * enableHw          : init with hardware if true is behavior equal to WsecInitializeHw
 * HardwareParam     : the KmcHardwareParam when init with hardware is used,
 *                     when init with software mode hdparm is ignored
 **/
typedef struct TagInitParams {
    WsecUint32 roleType;
    const KmcKsfName *filePathName;
    WsecBool enableThirdBackup;
    WsecBool deleteKsfOnInitFailed;
    WsecBool enableHw;
    KmcHardwareParam hdParm;
} WsecInternalInitParam;

/*
 * reserve: reserved parameter, which is not used currently.
 * Note: This function is not a direct external function and needs to be called using WsecInitializeEx.
 */
unsigned long KmcInitializeEx(const WsecInternalInitParam *initParam, KmcLockOrNot lockOrNot);

/* Note: This function is not an external function and needs to be triggered by WsecFinalizeEx. */
unsigned long KmcFinalizeEx(void);

/* KMC module reset. This function is not an external function and needs to be triggered by WsecResetEx. */
unsigned long KmcResetEx(WsecBool isHardware, WsecBuffConst hardwareParam);

/* KMC module role setting */
unsigned long KmcSetRoleType(WsecUint32 roleType);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_SHARE_H */
