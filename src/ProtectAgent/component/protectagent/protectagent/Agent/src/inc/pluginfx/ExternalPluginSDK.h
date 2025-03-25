/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * @file ExternalPluginSDK.h
 * @brief  header file of external plugin sdk external interfaces
 * @version 1.0.0.0
 * @date 2022-02-07
 * @author wangguitao 00510599
 */

#ifndef _EXTERNAL_PLUGIN_SDK_H
#define _EXTERNAL_PLUGIN_SDK_H

#include <string>
#include "common/Defines.h"

namespace {
// OPA : DataBackup Agent
static const int32_t OPA_SUCCESS = 0;
static const int32_t OPA_FAILED = 1;

// log level definition
static const int32_t OPA_LOG_DEBUG = 0;
static const int32_t OPA_LOG_INFO = 1;
static const int32_t OPA_LOG_WARN = 2;
static const int32_t OPA_LOG_ERROR = 3;
static const int32_t OPA_LOG_CRI = 4;

using CallbackWriteLog = void (*)(int32_t level, const std::string& filePathName, int32_t lineNum,
    const std::string& funcName, const std::string& logString);
}  // namespace

/* All callback functions */
AGENT_EXPORT struct OpaCallbacks {
    CallbackWriteLog writeLog;
};

/*
 * The initialization function is used to allocate global configuration.
 * The parameter rootPath is protectagent running root directory
 * The parameter isInitKmc is whether to initialize the KMC.
 */
AGENT_EXPORT uint32_t OpaInitialize(const std::string& rootPath, bool isInitKmc = true);

/*
 * The uninitialization function is used to free global configuration.
 */
AGENT_EXPORT uint32_t OpaUninitialize();

/*
 * Registers the callback function registered by the app
 */
AGENT_EXPORT uint32_t OpaRegFunc(const OpaCallbacks& allFuncs);

/*
* Get config value from agent configure file
* item : config item
*  0 : CRETIFICATE_ROOT_PATH,
*  1 : KEY_FILE_NAME,
*  2 : TRUSTE_CRETIFICATE_FILE_NAME,
*  3 : USE_CRETIFICATE_FILE_NAME,
*  4 : PASSWORD,
*  5 : ALGORITEHM_SUITE,
*  6 : HOST_NAME
*/
AGENT_EXPORT int32_t GetSecurityItemValue(int32_t item, std::string& value);

/*
* Decrypt string using KMC
*/
AGENT_EXPORT int32_t Decrypt(std::string input, std::string& output);

#endif
