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
#pragma once

#include <vector>
#include <ApplicationProtectBaseDataType_types.h>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include "config/KubeConfig.h"
#include "client/KubeClient.h"
#include "protect_engines/kubernetes/common/KubeMacros.h"

namespace KubernetesPlugin {

class KubernetesApi {
public:
    explicit KubernetesApi(KubeClient &kubeClient, KubeConfig kubeConfig) : client(kubeClient),
                                                                            config(kubeConfig)
    {
    };

    /**
     * @brief 查询集群所有NameSpaces(含sts的）
     *
     * @return
     */
    virtual std::pair<int, std::vector<ApplicationResource>> ListNameSpaces();

    /**
     * @brief 查询集群中对应namespace下的StateFulSet
     *
     * @param nameSpace 父NameSpace
     * @return
     */
    virtual std::pair<int, std::vector<ApplicationResource>> ListStatefulSet(const Application &nameSpace);

    /**
     * @brief 查询stateful set下面管理的pod
     *
     * @param statefulSet statefulSet
     * @return statefulset管理的POD
     */
    std::pair<int, std::vector<ApplicationResource>> ListPods(const Application &statefulSet);

    /**
     * @brief 查询pod内挂载的PV信息
     *
     * @param pod pod
     * @param volumeNames 挂载卷名称
     * @return pod内挂载的PV
     */
    virtual std::pair<int, std::vector<Pv>> ListPvs(const ApplicationResource &pod,
                                                    const std::vector<std::string> &volumeNames);

    /**
     * @brief 查询所有存储信息
     *
     * @return FlexVolume中所有符合条件的存储ip
     */
    virtual std::pair<int, std::set<std::string>> ListStorages();

    /**
     * @brief 停止StateFulSet下所有的POD
     *
     * @param sts StateFulSet
     * @return 成功或失败
     */
    int StopStateFulSet(const StateFulSet &sts);

    /**
     * @brief 恢复StateFulSet下的POD
     *
     * @param sts StateFulset
     * @return 成功或失败
     */
    int RestoreStateFulSet(const StateFulSet &sts);

    /**
     * @brief 使用WebSocket在容器中执行命令，类似于kubectl exec
     *        注意：该接口仅能执行单条命令，不能多条命令串联执行，命令结果需以回显的方式提供，否则无法获取执行结果
     *
     * @param nameSpace Pod所在的NameSpace名称
     * @param pod       Pod名称
     * @param container 容器名称
     * @param command   被执行的命令，仅单条命令，如sh a.sh，不支持{command1};{command2}等串联方式
     * @return 命令执行结果，需以回显的方式提供
     */
    std::string KubeExec(const std::string &nameSpace, const std::string &pod, const std::string &container,
                         const std::string &command);

private:
    int SetReplicas(const StateFulSet &sts, int val);

    std::vector<ApplicationResource> GetNameSpacesFromHttpBody(const std::string &stsBody,
                                                               const std::string &namespaceBody);

    static std::vector<StateFulSet> GetStatefulSetsFromHttpBody(const std::string &stsBody);

    static StateFulSet GetSingleStatefulSetsFromHttpBody(const std::string &stsBody);

    static std::vector<Pod> GetPodsFromHttpBody(const std::string &podBody, const std::string &stsName);

    static void FulfillPvsUnderPod(const std::string &pvBody, Pod &pod);

    static std::pair<int, ApplicationResource> ConvertStateFulSetToAppResource(StateFulSet &stateFulSet,
                                                                               const std::string &nameSpaceId);

    std::pair<int32_t, std::vector<Pv>> GetPvcList(const ApplicationResource &pod,
                                                   const std::vector<std::string> &volumeNames);

    std::pair<int32_t, std::vector<Pv>> GetPvsByPvcNames(const std::vector<Pv> &pvcList);

    std::pair<int, std::string> GetHttpBodyFromFlexVolume(const std::string &url);

    KubeClient &client;
    KubeConfig config;

    Module::HttpRequest CreateRequest(const std::string &method, const std::string &url);

    std::string CreateWebsocketRequest(const std::string &nameSpace, const std::string &pod,
                                       const std::string &container, const std::string &command);

    std::string GetWebsocketResponse(
            boost::beast::websocket::stream <boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>
            &ws);

    std::pair<int, std::vector<ApplicationResource>> FulFillStsResourceList(const std::string &stateFulHttpBody,
                                                                            const std::string &podsHttpBody,
                                                                            const std::string &pvsHttpBody,
                                                                            const bool &isGetSingleSts,
                                                                            const Application &nameSpace);
};

}