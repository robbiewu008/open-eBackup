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
#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include <string>
#include <client/ClientInvoke.h>
#include "log/Log.h"
#include "common/JsonHelper.h"
#include "common/Constants.h"
#include "common/Structs.h"
#include "repository_handlers/RepositoryHandler.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"
#include "volume_handlers/common/ControlDevice.h"
#include "curl_http/HttpClientInterface.h"
#include "protect_engines/apsara_stack/common/Structs.h"
#include "protect_engines/openstack/api/cinder/model/VolumeDetail.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "tracepoint/EbkTracePoint.h"
#ifndef WIN32
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdExecutor.h"
#endif
#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

namespace VirtPlugin {
namespace Utils {
const int METAFILE_MAX_SIZE = 1024 * 1024 * 100;
const int32_t MILSECOND = 1000;
using Defer = std::shared_ptr<void>;
#ifndef WIN32
const std::string AGENT_CONF_PATH = "/DataBackup/ProtectClient/ProtectClient-E/conf/agent_cfg.xml";
#else
const std::string AGENT_CONF_PATH = "\\DataBackup\\ProtectClient\\ProtectClient-E\\conf\\agent_cfg.xml";
#endif

inline void SleepSeconds(const int& sleepTime)
{
#ifdef WIN32
        Sleep(sleepTime * MILSECOND);
#else
        sleep(sleepTime);
#endif
}

template<typename T>
int LoadFileToStruct(std::shared_ptr<RepositoryHandler> repoHandler, const std::string &file, T &t)
{
    LOGGUARD("");
    if (repoHandler == nullptr) {
        ERRLOG("repoHandler is null.");
        return FAILED;
    }

    if (repoHandler->Open(file, "r") != SUCCESS) {
        ERRLOG("Open file failed, file[%s]", file.c_str());
        return FAILED;
    }

    Defer _(nullptr, [&, repoHandler](...) {
        if (repoHandler != nullptr) {
            repoHandler->Close();
        }
    });

    size_t fileSize = repoHandler->FileSize(file);
    if (fileSize <= 0) {
        ERRLOG("Get file size failed, file[%s]", file.c_str());
        return FAILED;
    }

    DBGLOG("File: %s , size: %d", file.c_str(), fileSize);
    if (fileSize > METAFILE_MAX_SIZE) {
        ERRLOG("File size[%zu] exceeded the max size[%d] that virtual plugin can handle.[%s]",
            fileSize, METAFILE_MAX_SIZE, file.c_str());
        return FAILED;
    }

    if (repoHandler->Seek(0) != 0) {
        ERRLOG("Seek file to offset 0 failed. File: %s", file.c_str());
        return FAILED;
    }

    std::string content;
    size_t readRet = repoHandler->Read(content, fileSize);
    if (readRet != fileSize) {
        ERRLOG("Read file content failed, file[%s]", file.c_str());
        return FAILED;
    }

    if (!Module::JsonHelper::JsonStringToStruct(content, t)) {
        ERRLOG("Transfer metadata json string to struct failed.");
        return FAILED;
    }

    DBGLOG("Load file to struct success.");
    return SUCCESS;
}

// 防止网络瞬断引起的失败
template<typename T>
int LoadFileToStructWithRetry(std::shared_ptr<RepositoryHandler> repoHandler, const std::string &file, T &t)
{
    int retryTimes = 3;
    int sleepSeconds = 3;
    while (retryTimes > 0) {
        if (LoadFileToStruct(repoHandler, file, t) == SUCCESS) {
            return SUCCESS;
        }
        SleepSeconds(sleepSeconds);
        retryTimes--;
    }
    return FAILED;
}

int SaveToFile(const std::shared_ptr<RepositoryHandler> &repoHandler,
    const std::string &filename, const std::string &content);
int SaveToFileWithRetry(const std::shared_ptr<RepositoryHandler> &repoHandler, const std::string &filename,
    const std::string &content);
int ReadFile(const std::shared_ptr<RepositoryHandler> &repoHandler,
    const std::string &filename, std::string &content);
int GetRandom(uint64_t &num);

template<typename T>
int RetryOpWithT(std::function<T()> func, T expectValue, const std::string &opName,
                 int retryTimes = 3, const int sleepSeconds = 3)
{
    if (func == nullptr) {
        ERRLOG("func:[%s] is null", opName.c_str());
        return FAILED;
    }
    while (retryTimes > 0) {
        T ret = func();
        if (ret == expectValue) {
            break;
        }
        SleepSeconds(sleepSeconds);
        retryTimes--;
        if (retryTimes == 0) {
            ERRLOG("Operation[%s] failed.", opName.c_str());
            return FAILED;
        }
        ERRLOG("Operation[%s] failed, will retry.", opName.c_str());
    }
    return SUCCESS;
}

// 从AgentConf中获得是内置还是外置代理
int32_t GetAgentDeployScence(std::string &deployScence);
int32_t GetDeployType(std::string& result);
int32_t GetAgentConfig(std::string& result, const std::string& section, const std::string& child);
bool IsInnerAgentscence();
std::string GetInnerAgentLocalIp();
void InnerAgentAppointNetDevice(Module::HttpRequest& request, bool isOpService);
bool GetStoragesInfoFromExtendInfo(const std::string &authExtendInfo, std::vector<StorageInfo> &storageVector);
#ifndef WIN32
std::string GetProxyHostId(bool isOpenstack);
std::string DoGetProxyHostId(bool isOpenstack);
void SaveStringToVirtConfig(const std::string &str, const std::string &item);
void TransCmdParamsToCmdStr(const std::vector<Module::CmdParam> &cmdParam, std::string &cmdStr);
int32_t CallAgentExecCmd(const std::vector<Module::CmdParam> &cmdParams, std::vector<std::string> &cmdOut);
#endif
bool CheckVolIpInStorage(const VolInfo &volInfo, const StorageInfo &storageInfo);
void GetIpVectorFromString(std::vector<std::string> &ipList, const std::string &ip, const std::string &ipListString);
void ReportJobDetailWithLabel(const ReportJobDetailsParam &jobParam, const std::vector<std::string> &args,
    std::string jobId, std::string subJobId);
void ReportLog2Agent(ReportLog2AgentParam &param, std::string jobId, std::string subJobId);

void RemoveSpecialSymbols(std::string &str);
void ConvertDiskToVolume(const Disk &disk, OpenStackPlugin::Volume &volume);
int GetMetaRepoPath(std::string &metaRepoPath, std::shared_ptr<VirtPlugin::RepositoryHandler>& metaRepoHandler,
                    const AppProtect::BackupJob &job);
bool CheckIpv6Valid(const std::string &ip);
int32_t GetStringValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, std::string& result);
int32_t GetJsonValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, Json::Value& result);
int32_t GetBoolValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, bool& result);
int32_t SetJsonValueByKeyToJsonString(std::string& bodyStr, const std::string& key, const Json::Value& value);
void InitAndRegTracePoint();
void InitAndRegTracePointForHcs();
double SafeStod(const std::string& str, double defaultValue = FAILED);
int32_t GetUtf8CharNumber(const std::string &str);
std::vector<int> ParseVersion(const std::string& version);
bool CompareVersion(const std::string &version1, const std::string &version2);
std::string UrlEncode(const std::string& str);
template <class T>
int32_t TransStrToApsaraApiResponse(const std::string &resStr, T &apiResponse, const int32_t &expectedCode = 200)
{
    APSResponse apsResponse;
    if (!Module::JsonHelper::JsonStringToStruct(resStr, apsResponse)) {
        ERRLOG("Failed to parse API result, response str: %s.", WIPE_SENSITIVE(resStr).c_str());
        return FAILED;
    }
    if (apsResponse.m_statusCode != expectedCode) {
        ERRLOG("Call api failed, return code: %d , errinfo: %s.", apsResponse.m_statusCode, apsResponse.m_erro.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(apsResponse.m_body, apiResponse)) {
        ERRLOG("Failed to trans APSResponse to api response, body: %s.", WIPE_SENSITIVE(apsResponse.m_body).c_str());
        return FAILED;
    }
    DBGLOG("Trans APSResponse to api response success.");
    return SUCCESS;
}
} // end namespace Utils
} // end namespace VirtPlugin

#endif // UTILS_H