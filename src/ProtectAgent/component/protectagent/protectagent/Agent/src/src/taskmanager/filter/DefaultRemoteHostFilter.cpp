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
#include "taskmanager/filter/DefaultRemoteHostFilter.h"
#include "taskmanager/externaljob/Job.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"
#include "message/curlclient/DmeRestClient.h"
DefaultRemoteHostFilter::DefaultRemoteHostFilter()
{}

DefaultRemoteHostFilter::~DefaultRemoteHostFilter()
{}

mp_bool IsDataturboMount(PluginJobData &data)
{
    if (data.param.isMember("taskParams") && data.param["taskParams"].isMember("dataLayout")) {
        if (data.param["taskParams"]["dataLayout"].isMember("srcDeduption") &&
            data.param["taskParams"]["dataLayout"]["srcDeduption"].isBool()) {
            if (data.param["taskParams"]["dataLayout"]["srcDeduption"].asBool()) {
                data.param["taskParams"]["dataLayout"]["extendInfo"]["srcDeduption"] = true;
                return MP_TRUE;
            }
        }
        data.param["taskParams"]["dataLayout"]["extendInfo"]["srcDeduption"] = false;
    }
    return MP_FALSE;
}

mp_int32 DefaultRemoteHostFilter::GetLANType(Json::Value& jobParam, mp_int32& lanType)
{
    (void) jobParam;
    auto Ret = DmeRestClient::GetInstance()->CheckEcsMetaInfo();
    lanType = APPINFO_LAN_TYPE_VLAN;
    if (Ret == MP_SUCCESS) {
        lanType = APPINFO_LAN_TYPE_VXLAN;
        INFOLOG("Job mount type is Vxlan");
    }
    return MP_SUCCESS;
}

mp_void DefaultRemoteHostFilter::Transfer2ByPortType(const std::vector<mp_int32>& portTypes,
                                                     std::vector<Json::Value>& remoteHost,
                                                     std::vector<Json::Value>& filteredRemoteHost)
{
    for (auto iter = remoteHost.begin(); iter != remoteHost.end(); ++iter) {
        Json::Value& remoteHostItem = *iter;
        if (!remoteHostItem.isMember(JobParamKey::PORT_TYPE)) {
            continue;
        }
        mp_int32 pType;
        if (remoteHostItem[JobParamKey::PORT_TYPE].isString()) {
            pType = atoi(remoteHostItem[JobParamKey::PORT_TYPE].asCString());
        } else if (remoteHostItem[JobParamKey::PORT_TYPE].isInt()) {
            pType = remoteHostItem[JobParamKey::PORT_TYPE].asInt();
        } else {
            ERRLOG("Parse port type failed!");
            return;
        }
        bool find = false;
        for (mp_int32 portType : portTypes) {
            if (pType == portType) {
                DBGLOG("Matching, remoteHost=%s", Json::FastWriter().write(remoteHostItem).c_str());
                filteredRemoteHost.push_back(remoteHostItem);
                find = true;
                break;
            }
        }
        if (!find) {
            WARNLOG("Not matching, remoteHost=%s", Json::FastWriter().write(remoteHostItem).c_str());
        }
    }
}

mp_void DefaultRemoteHostFilter::UpdateReposParam(const std::vector<mp_int32>& portTypes,
                                                  Json::Value& jobParam,
                                                  std::map<Json::ArrayIndex, std::vector<Json::Value>> jsonReposMap)
{
    for (auto vecIter = jsonReposMap.begin(); vecIter != jsonReposMap.end(); ++vecIter) {
        for (auto& repoItemJson : vecIter->second) {
            std::vector<Json::Value> remoteHost;
            std::vector<Json::Value> filteredRemoteHost;
            CJsonUtils::GetJsonArrayJson(repoItemJson, JobParamKey::REMOTE_HOST, remoteHost);
            repoItemJson[JobParamKey::REMOTE_HOST] = Json::Value(); // clear remoteHost json array
            Transfer2ByPortType(portTypes, remoteHost, filteredRemoteHost);
            std::vector<Json::Value>& finalRemoteHost = filteredRemoteHost.empty() ? remoteHost : filteredRemoteHost;
            for (auto& hostItem : finalRemoteHost) {
                repoItemJson[JobParamKey::REMOTE_HOST].append(hostItem);
            }
            if (vecIter->first == 0) {
                jobParam[JobParamKey::REPOSITORIES].append(repoItemJson);
            } else {
                jobParam[JobParamKey::COPIES][vecIter->first - 1][JobParamKey::REPOSITORIES].append(repoItemJson);
            }
        }
    }
}

mp_int32 DefaultRemoteHostFilter::DoFilter(PluginJobData& jobData, mp_bool isInner)
{
    INFOLOG("Filter begin, jobId=%s, subJobId=%s", jobData.mainID.c_str(), jobData.subID.c_str());
    mp_int32 lanType;
    Json::Value& jobParam = jobData.param;
    if (isInner || IsDataturboMount(jobData) || GetLANType(jobParam, lanType) == MP_FAILED) {
        // If is inner Agent or get port type failed will skip filter.
        WARNLOG("Skip filter, jobId=%s, subJobId=%s.", jobData.mainID.c_str(), jobData.subID.c_str());
        return MP_FAILED;
    }
    if (lanType == APPINFO_LAN_TYPE_VLAN || lanType == APPINFO_LAN_TYPE_VXLAN) {
        std::map<Json::ArrayIndex, std::vector<Json::Value>> jsonRepoMap;
        std::vector<Json::Value> vecJsonBackupRepo;
        // get backup repositories
        CJsonUtils::GetJsonArrayJson(jobParam, JobParamKey::REPOSITORIES, vecJsonBackupRepo);
        jsonRepoMap.insert(std::make_pair(0, vecJsonBackupRepo));
        // clear backup repositories json array
        jobParam[JobParamKey::REPOSITORIES] = Json::Value();
        // get copies repositories
        for (Json::ArrayIndex idx = 0; idx < jobParam[JobParamKey::COPIES].size(); ++idx) {
            std::vector<Json::Value> vecJsonCopyRepo;
            CJsonUtils::GetJsonArrayJson(jobParam[JobParamKey::COPIES][idx],
                                         JobParamKey::REPOSITORIES,
                                         vecJsonCopyRepo);
            jsonRepoMap.insert(std::make_pair(idx + 1, vecJsonCopyRepo));
            // clear copies repositories json array
            jobParam[JobParamKey::COPIES][idx][JobParamKey::REPOSITORIES] = Json::Value();
        }
        std::vector<mp_int32> portTypes;
        if (lanType == APPINFO_LAN_TYPE_VLAN) {
            portTypes = { LOGIC_PORT_TYPE_ETH, LOGIC_PORT_TYPE_BOND, LOGIC_PORT_TYPE_VLAN };
            UpdateReposParam(portTypes, jobParam, jsonRepoMap);
        } else if (lanType == APPINFO_LAN_TYPE_VXLAN) {
            portTypes = { LOGIC_PORT_TYPE_SIP, LOGIC_PORT_TYPE_ETH, LOGIC_PORT_TYPE_BOND, LOGIC_PORT_TYPE_VLAN };
            UpdateReposParam(portTypes, jobParam, jsonRepoMap);
        }
        INFOLOG("Filter finished, jobId=%s, subJobId=%s", jobData.mainID.c_str(), jobData.subID.c_str());
    } else {
        WARNLOG("Filter lan type not support, skip filter, jobId=%s, subJobId=%s.",
            jobData.mainID.c_str(), jobData.subID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}