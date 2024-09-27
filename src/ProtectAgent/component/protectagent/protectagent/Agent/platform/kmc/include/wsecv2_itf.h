/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: external interface
 * Author: Luan Shipeng l00171031
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_INCLUDE_WSECV2_ITF_H
#define KMC_INCLUDE_WSECV2_ITF_H

#include "wsecv2_type.h"
#include "wsecv2_errorcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WSEC_FILEPATH_MAX_LEN 260 /* Maximum length of a file path. */
#define KMC_VERSION "KMC 20.1.0"

/*
 * API Function Prototype Description
 * When the system is started or shut down, the application needs to call the following functions.
 */
/* Callback function registration */
unsigned long WsecRegFuncEx(const WsecCallbacks *allCallbacks);

/*
 * When no security hardware is available,
 * the initialization function is used to specify the master or agent.
 * The paths of the active and standby files of the keystore must be specified.
 * The variant parameter is reserved.
  */
unsigned long WsecInitializeEx(WsecUint32 roleType,
    const KmcKsfName *filePathName,
    WsecBool useImportKey,
    WsecVoid *exParam);

/*
 * When security hardware is used, the initialization function is used to specify the master or agent.
 * The paths of the active and standby files of the keystore must be specified. The variant parameter is reserved.
 * hasSoftLevelRk: indicates whether to use the software-layer root key for acceleration.
 * The software-layer root key is encrypted by the hardware root key and stored in the keystore file in ciphertext.
 * Once this parameter is specified, it cannot be changed.
 * This parameter has low security risks due to performance considerations.
 */
unsigned long WsecInitializeHw(WsecUint32 roleType,
    const KmcKsfName *filePathName,
    WsecBool hasSoftLevelRk,
    const WsecVoid *hardwareParam, WsecUint32 hardwareParamLen,
    WsecVoid *exParam);

/*
 * Function for reloading the keystore file.
 * This function is used to reload the keystore file from the keystore file in the process.
 * Note: After this function is called, the configuration is restored
 * to the default configuration. If the default configuration is not used,
 * the app needs to reconfigure the configuration.
 */
unsigned long WsecResetEx(void);

unsigned long WsecResetHw(const WsecVoid *hardwareParam, WsecUint32 hardwareParamLen);

/* Specified master or agent */
unsigned long WsecSetRole(WsecUint32 roleType);

/* Deinitializes a function. */
unsigned long WsecFinalizeEx(void);

/* Obtains the current version number. */
const char *WsecGetVersion(void);

#ifdef WSEC_DEBUG
/* Size of the callback Wsec data structure */
WsecVoid WsecShowStructSize(CallbackShowStructSize showStructures);
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_INCLUDE_WSECV2_ITF_H */
