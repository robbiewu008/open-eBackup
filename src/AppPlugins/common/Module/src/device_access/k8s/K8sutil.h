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
#ifndef K8S_UTIL_H
#define K8S_UTIL_H
#include <string>
#include <mutex>
#include "common/JsonHelper.h"

namespace Module {
    namespace k8s {
        struct PodsInfo {
            std::string Name;
            std::string PodPhase;
            std::string Namespace;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(Name)
                SERIAL_MEMEBER(PodPhase)
                SERIAL_MEMEBER(Namespace)
            END_SERIAL_MEMEBER
        };

        struct PodInfoList {
            std::vector<PodsInfo> PodsInfos;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(PodsInfos)
            END_SERIAL_MEMEBER
        };

        struct NodesInfo {
            std::string Name;
            bool NodeReady{false};
            std::string Namespace;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(Name)
                SERIAL_MEMEBER(NodeReady)
                SERIAL_MEMEBER(Namespace)
            END_SERIAL_MEMEBER
        };

        struct NodeInfoList {
            std::vector<NodesInfo> NodsInfos;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(NodsInfos)
            END_SERIAL_MEMEBER
        };

        struct DBUserPD {
            std::string UserName;
            std::string PD;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(UserName)
                SERIAL_MEMEBER(PD)
            END_SERIAL_MEMEBER
        };

        struct CacheInfo {
            std::mutex MLock;
            DBUserPD NormalUserInfo;
            DBUserPD SuperUserInfo;

            BEGIN_SERIAL_MEMEBER
                SERIAL_MEMEBER(NormalUserInfo)
                SERIAL_MEMEBER(SuperUserInfo)
            END_SERIAL_MEMEBER
        };

        std::string GetRandomAvailableNode();

        uint64_t GetCurrentMillis();

//        bool GetAllPodsInfo(PodInfoList &infoList);

        bool GetAllNodesInfo(NodeInfoList &infoList);

        bool CheckDBUserPD(bool isNormal, DBUserPD &userPD, bool hasError);

        bool GetDBUserPD(bool isNormal, DBUserPD &userPD);

        bool GetNormalDBUserPD(DBUserPD &userPD);

        bool GetSuperDBUserPD(DBUserPD &userPD);

//        uint32_t GetRunningPodsNum(const std::string podName);

        void AddConfigMapItem(const std::string &cfgName, const std::string &cfgItem, const std::string &cfgVal);

        std::string GetConfigMapItem(const std::string &cfgName, const std::string &cfgItem);

        std::string
        GetConfigMapItem(const std::string &cfgName, const std::string &cfgItem, const std::string &defaultCfg);
    };  // namespace k8s
}
#endif