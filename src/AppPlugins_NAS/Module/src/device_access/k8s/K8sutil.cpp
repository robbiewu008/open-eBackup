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
#include "device_access/k8s/K8sutil.h"
#include <chrono>
#include <unistd.h>
#include "log/Log.h"
#include "define/Defines.h"
#include "device_access/k8s/MSRestClient.h"
#include "curl_http/CurlHttpClient.h"
namespace Module {
    namespace {
        constexpr auto MODULE = "K8sUtil";
    }

    namespace k8s {

        std::string GetRandomAvailableNode() {
            char hostname[MAX_HOSTNAME_LENGTH] = {0};

            gethostname(hostname, sizeof(hostname));

            return std::string(hostname);
        }

        uint64_t GetCurrentMillis() {
            std::uint64_t val =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
            HCP_Log(INFO, MODULE) << "Get mils " << val << HCPENDLOG;
            return val;
        }

//        bool GetAllPodsInfo(PodInfoList &infoList) {
//            std::string hostName = GetPodNameByEnv("MY_POD_NAME");
//            std::string podName = TransPodName(hostName);
//            HCP_Log(DEBUG, MODULE) << DBG(hostName) << DBG(podName) << HCPENDLOG;
//
//            MSRestRequest req;
//            req.httpMethod = "GET";
//            req.serviceUrl =
//                    "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/internal/pod/info?appName=" + podName;
//            req.isVerify = INTERNAL_VERIFY;
//            Json::Value rsp;
//            std::string errorDes;
//            int iRet = MSRestCLient::Instance().SendReqeust(req, rsp, errorDes);
//            if (iRet != SUCCESS || !rsp.isArray()) {
//                HCP_Log(ERR, MODULE) << "get pod info failed, iRet:" << iRet
//                                     << ", errorDes:" << WIPE_SENSITIVE(errorDes) << HCPENDLOG;
//                return false;
//            }
//            PodsInfo info;
//            int podNumber = 0;
//            for (auto &pod : rsp) {
//                if (!pod.isMember("podName") || !pod.isMember("podStatus") || !pod.isMember("namespace")) {
//                    HCP_Log(ERR, MODULE) << "pod info is invalid:" << WIPE_SENSITIVE(pod) << HCPENDLOG;
//                    continue;
//                }
//                info.Name = pod["podName"].asString();
//                info.PodPhase = pod["podStatus"].asString();
//                info.Namespace = pod["namespace"].asString();
//                HCP_Log(DEBUG, MODULE) << DBG(info.Name) << DBG(info.PodPhase) << DBG(info.Namespace) << HCPENDLOG;
//                infoList.PodsInfos.push_back(info);
//                podNumber++;
//            }
//            if (podNumber == 0) {
//                HCP_Log(ERR, MODULE) << "Pods info is empty" << HCPENDLOG;
//                return false;
//            }
//            return true;
//        }

        bool GetAllNodesInfo(NodeInfoList &infoList) {
            MSRestRequest req;
            req.httpMethod = "GET";
            req.serviceUrl = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/collect/node/info";
            req.isVerify = INTERNAL_VERIFY;
            Json::Value rsp;
            std::string errorDes;
            int iRet = MSRestCLient::Instance().SendReqeust(req, rsp, errorDes);
            if (iRet != SUCCESS || !rsp.isArray()) {
                HCP_Log(ERR, MODULE) << "get nodes info failed, iRet:" << iRet
                                     << ", errorDes:" << WIPE_SENSITIVE(errorDes) << HCPENDLOG;
                return false;
            }
            NodesInfo info;
            int nodeNumber = 0;
            for (auto &node : rsp) {
                if (!node.isMember("hostname") || !node.isMember("nodeStatus")) {
                    HCP_Log(ERR, MODULE) << "node info is invalid:" << node << HCPENDLOG;
                    continue;
                }
                bool ready = false;
                if (node["nodeStatus"].asString() == "ready") {
                    ready = true;
                }
                info.Name = node["hostname"].asString();
                info.NodeReady = ready;
                info.Namespace = "dpa";
                infoList.NodsInfos.push_back(info);
                nodeNumber++;
            }
            if (nodeNumber == 0) {
                HCP_Log(ERR, MODULE) << "Nodes info is empty" << HCPENDLOG;
                return false;
            }
            for (auto &it : infoList.NodsInfos) {
                HCP_Log(INFO, MODULE) << "Got node " << it.Name << HCPENDLOG;
            }
            return true;
        }

        CacheInfo cacheInfo;

        DBUserPD GetNormalInfo(CacheInfo &cacheUserPD) {
            std::lock_guard<std::mutex> lockGuard(cacheUserPD.MLock);
            DBUserPD userPD = cacheUserPD.NormalUserInfo;
            return userPD;
        }

        DBUserPD GetSuperInfo(CacheInfo &cacheUserPD) {
            std::lock_guard<std::mutex> lockGuard(cacheUserPD.MLock);
            DBUserPD userPD = cacheUserPD.SuperUserInfo;
            return userPD;
        }

        void UpdateNormalInfo(CacheInfo &cacheUserPD, DBUserPD userPD) {
            std::lock_guard<std::mutex> lockGuard(cacheUserPD.MLock);
            cacheUserPD.NormalUserInfo = userPD;
        }

        void UpdateSuperInfo(CacheInfo &cacheUserPD, DBUserPD userPD) {
            std::lock_guard<std::mutex> lockGuard(cacheUserPD.MLock);
            cacheUserPD.SuperUserInfo = userPD;
        }

        bool CheckDBUserPD(bool isNormal, DBUserPD &userPD, bool hasError) {
            if (userPD.UserName == "" || userPD.PD == "") {
                HCP_Log(ERR, MODULE) << "Get db user PD failed" << HCPENDLOG;
                hasError = true;
            }

            if (hasError) {
                HCP_Log(ERR, MODULE) << "Get db user info from k8s fail, use cache info" << HCPENDLOG;
                if (isNormal) {
                    userPD = GetNormalInfo(cacheInfo);
                } else {
                    userPD = GetSuperInfo(cacheInfo);
                }
                if (userPD.PD == "") {
                    return false;
                }
            } else {
                HCP_Log(INFO, MODULE) << "Update db user cacheinfo" << HCPENDLOG;
                if (isNormal) {
                    UpdateNormalInfo(cacheInfo, userPD);
                } else {
                    UpdateSuperInfo(cacheInfo, userPD);
                }
            }
            return true;
        }

        bool GetDBUserPD(bool isNormal, DBUserPD &userPD) {
            MSRestRequest req;
            req.httpMethod = "GET";
            req.serviceUrl =
                    "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret";
            req.isVerify = INTERNAL_VERIFY;
            Json::Value rsp;
            std::string errorDes;
            int iRet = MSRestCLient::Instance().SendReqeust(req, rsp, errorDes);
            bool hasError = false;
            if (iRet != SUCCESS || !rsp.isArray()) {
                HCP_Log(ERR, MODULE) << "Get secretdata failed,  errorDes:" << WIPE_SENSITIVE(errorDes) << HCPENDLOG;
                hasError = true;
            }
            std::string nameField = "database.generalUsername";
            std::string pdField = "database.generalPassword";
            if (!isNormal) {
                nameField = "database.superUsername";
                pdField = "database.superPassword";
            }
            for (auto &dataMap : rsp) {
                if (dataMap.isMember(nameField)) {
                    userPD.UserName = dataMap[nameField].asString();
                }
                if (dataMap.isMember(pdField)) {
                    userPD.PD = dataMap[pdField].asString();
                }
            }
            if (!CheckDBUserPD(isNormal, userPD, hasError)) {
                HCP_Log(ERR, MODULE) << "DB password is null." << HCPENDLOG;
                return false;
            }
            return true;
        }

        bool GetNormalDBUserPD(DBUserPD &userPD) {
            bool ret = GetDBUserPD(true, userPD);
            if (!ret) {
                HCP_Log(ERR, MODULE) << "Get normal db user PD failed" << HCPENDLOG;
            }
            return ret;
        }

        bool GetSuperDBUserPD(DBUserPD &userPD) {
            bool ret = GetDBUserPD(false, userPD);
            if (!ret) {
                HCP_Log(ERR, MODULE) << "Get super db user PD failed" << HCPENDLOG;
            }
            return ret;
        }

//        uint32_t GetRunningPodsNum(const std::string podName) {
//            PodInfoList infoList;
//            if (!GetAllPodsInfo(infoList)) {
//                HCP_Log(ERR, MODULE) << "GetAllPodsInfo failed" << HCPENDLOG;
//                return 0;
//            }
//            uint32_t num = 0;
//            for (auto &it : infoList.PodsInfos) {
//                if (it.PodPhase == "Running" && it.Name.find(podName) != std::string::npos) {
//                    num++;
//                }
//            }
//            return num;
//        }

        std::string GetConfigMapItem(const std::string &cfgName, const std::string &cfgItem) {
            MSRestRequest req;
            req.httpMethod = "GET";
            req.serviceUrl =
                    "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/configmap/info?nameSpace=dpa&configMap=" +
                    cfgName;
            req.isVerify = INTERNAL_VERIFY;
            Json::Value rsp;
            std::string errorDes;
            int iRet = MSRestCLient::Instance().SendReqeust(req, rsp, errorDes);
            if (iRet != SUCCESS || !rsp.isArray()) {
                HCP_Log(ERR, MODULE) << "get configmap info failed, errorDes:" << WIPE_SENSITIVE(errorDes) << HCPENDLOG;
                return "";
            }
            std::string cfgValue;
            for (auto &cfg : rsp) {
                if (cfg.isMember(cfgItem)) {
                    cfgValue = cfg[cfgItem].asString();
                }
            }
            if (cfgValue.empty()) {
                HCP_Log(INFO, MODULE) << "cfgValue is empty" << HCPENDLOG;
            }
            return cfgValue;
        }

        void AddConfigMapItem(const std::string &cfgName, const std::string &cfgItem, const std::string &cfgVal) {
            HttpRequest req;
            req.method = "POST";
            req.url =
                    "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/configmap/create?nameSpace=dpa&configMap=" +
                    cfgName + "&configKey=" + cfgItem + "&configValue=" + cfgVal;
            req.isVerify = INTERNAL_VERIFY;

            Json::Value rsp;
            const uint32_t timeout = 10;
            if (!SendHttpRequest(req, rsp, timeout, false)) {
                HCP_Log(ERR, MODULE) << "SendHttpRequest failed!" << DBG(cfgItem) << HCPENDLOG;
            } else {
                HCP_Log(INFO, MODULE) << "Add config success." << DBG(cfgItem) << HCPENDLOG;
            }
        }

        std::string
        GetConfigMapItem(const std::string &cfgName, const std::string &cfgItem, const std::string &defaultCfg) {
            HttpRequest req;
            req.method = "GET";
            req.url =
                    "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/configmap/info?nameSpace=dpa&configMap=" +
                    cfgName + "&configKey=" + cfgItem;
            req.isVerify = INTERNAL_VERIFY;

            Json::Value rsp;
            const uint32_t timeout = 10;
            if (!SendHttpRequest(req, rsp, timeout, false)) {
                HCP_Log(ERR, MODULE) << "SendHttpRequest failed!" << HCPENDLOG;
                return "";
            }

            if (!rsp.isMember("data")) {
                HCP_Log(ERR, MODULE) << "No data in json rsp!" << HCPENDLOG;
                return "";
            }

            if (!rsp["data"].isArray()) { // no such config item.add it!
                AddConfigMapItem(cfgName, cfgItem, defaultCfg);
                return defaultCfg;
            }

            std::string cfgVal;
            try { // avoid crash by invalid config because of attack or human mistake
                cfgVal = rsp["data"][0][cfgItem].asString();
                HCP_Log(DEBUG, MODULE) << DBG(cfgVal) << HCPENDLOG;
            } catch (std::exception &e) {
                HCP_Log(ERR, MODULE) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            }

            return cfgVal;
        }
    }; // namespace k8s
}; // namespace Module
