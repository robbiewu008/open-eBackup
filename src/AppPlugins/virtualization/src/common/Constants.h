/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <cstdint>
#include <set>
#include <openssl/sha.h>
#include "Macros.h"
#include "common/PluginTypes.h"
#include "common/EnvVarManager.h"

namespace VirtPlugin {
const int SUCCESS = 0;
const int FAILED = -1;
const int DATA_SAME_IGNORE_WRITE = 1;
const int DATA_ALL_ZERO_IGNORE_WRITE = 2;
const int CALCULATE_SHA_FAILED = 3;
const int CALCULATE_INITIAL_STATE = 4;

constexpr uint64_t BACKUP_INC_TO_FULL = 1577209901;
constexpr uint64_t CHCEK_USER_FAILED = 1077949061;
constexpr uint64_t CHECK_NETWORK_TIME_OUT = 1677931275;
constexpr uint32_t NET_STATUS_CODE_OK = 200;
constexpr uint32_t DIRTY_RANGE_BLOCK_SIZE =  4194304; // 4M
constexpr uint32_t DIRTY_RANGE_COMPRESS_SIZE = 1024 * 1024;
const int NOT_MATCH = -2;
const int DIFFERENT_FLOW = 9;  // hook点返回码，则表示忽略框架流程，自定义处理流程, 暂定该值为9
const int32_t DAMAGED = -2;
const int32_t DEFAULT_SIZE = 500;
const std::string AVAILABLE_CAPACITY_THRESHOLD_KEY = "available_capacity_threshold";
const double DEFAULT_STORAGE_THRESHOLD_LIMIT = 20; // 默认容量阈值
const double MIN_STORAGE_THRESHOLD_LIMIT = 0; // 最小容量阈值
const double MAX_STORAGE_THRESHOLD_LIMIT = 100; // 最大容量阈值

// 默认块大小为4M
constexpr uint64_t DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024ULL;
// 默认卷分段阈值为60GB
constexpr uint64_t DEFAULT_SEGMENT_THRESHOLD = 60 * 1024 * 1024 * 1024ULL;
// 分段加载块sha256 buffer的大小
constexpr uint64_t BLOCKS_SHA256_BUF_SIZE = DEFAULT_SEGMENT_THRESHOLD / DEFAULT_BLOCK_SIZE * SHA256_DIGEST_LENGTH;
// 业务子任务名需指定为 VirtualizationBusinessSubJob_数字，以获取对应的挂载点，提升性能
const std::string BUSINESS_SUB_JOB_NAME_PREFIX = "VirtualizationBusinessSubJob_";
const std::string REPORT_COPY_SUB_JOB = "ReportCopySubJob";
// snapshot description reminder
const std::string SNAP_DESCRIPTION_REMINDER = "**PLEASE DO NOT CHANGE THE SNAPSHOT NAME AND THIS DESCRIPTION**";
#ifdef WIN32
static const std::string VIRTUAL_LOG_PATH =
    "C:\\DataBackup\\ProtectClient\\ProtectClient-E\\log\\Plugins\\VirtualizationPlugin"; // 需要适配
static const std::string VIRT_PLUGIN_PATH = "C:\\DataBackup/ProtectClient\\Plugins\\VirtualizationPlugin\\";
static const std::string VIRTUAL_CONF_PATH = VIRT_PLUGIN_PATH + "\\conf\\";
#else
static const std::string VIRTUAL_LOG_PATH = "/DataBackup/ProtectClient/ProtectClient-E/slog/VirtualPlugin/log";
static const std::string VIRT_PLUGIN_PATH = "/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/";
static const std::string VIRTUAL_CONF_PATH = VIRT_PLUGIN_PATH + "/conf/";
#endif
const std::string GENERAL_CONF = "General";
const std::string DMI_DECODE_UUID  = "DmiDecodeUUID";
const std::string SUDO_DISK_TOOL_PATH = VirtPlugin::VIRT_PLUGIN_PATH + "bin/security_sudo_disk.sh";
static const std::string VIRTUAL_CONF_NAME = "hcpconf.ini";


// http错误码定义
enum class CertExternalErrorCode {
    // 一切正常
    CURLE_OK = 0,
    // 证书校验不过会报35
    CURL_SSL_CONNECT_ERROR = 35,
    // 被回调中止，回调将中止返回
    CURLE_ABORTED_BY_CALLBACK = 42,
    // 远程服务器的SSL证书不正常
    CURLE_PEER_FAILED_VERIFICATION = 60,
    // 读取SSL的CA证书时出现问题（路径？访问权限？）
    CURLE_SSL_CACERT_BADFILE = 77,
};

const std::set<int32_t> CertErrorCode = {static_cast<int32_t>(CertExternalErrorCode::CURLE_PEER_FAILED_VERIFICATION),
                                         static_cast<int32_t>(CertExternalErrorCode::CURLE_SSL_CACERT_BADFILE),
                                         static_cast<int32_t>(CertExternalErrorCode::CURLE_ABORTED_BY_CALLBACK),
                                         static_cast<int32_t>(CertExternalErrorCode::CURL_SSL_CONNECT_ERROR)};
}

#endif // CONSTANTS_H