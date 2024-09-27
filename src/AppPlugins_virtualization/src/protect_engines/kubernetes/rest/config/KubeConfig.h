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

#include <boost/asio/ssl/context.hpp>
#include <utility>
#include <string>
#include <yaml-cpp/yaml.h>
#include "curl_http/CodeConvert.h"
#include "log/Log.h"
#include "protect_engines/kubernetes/common/KubeMacros.h"

namespace KubernetesPlugin {

struct ContextInfo {
    std::string clusterName;
    std::string userName;
};

struct ClusterInfo {
    std::string crt;
    std::string server;
};

struct UserInfo {
    std::string crt;
    std::string privateKey;
};

class KubeConfig {
public:
    KubeConfig(const KubeConfig &) = default;
    KubeConfig &operator=(const KubeConfig &) = default;

    /**
     * @brief 获取kubeconfig中当前环境的集群CA证书
     *
     * @return kubeconfig中的CA证书
     */
    std::string GetClusterCrt() const;

    /**
     * @brief 获取kubeconfig中当前环境的集群flexvolume的地址
     *
     * @return kubeConfig中的地址
     */
    std::string GetClusterServer() const;

    /**
     * @brief 获取ClusterServer中的IP和端口，格式为<schemas>://<ip or domain>:<port>
     *
     * @param ip   [OUT]IP地址
     * @param port [OUT]端口
     * @return true：成功 false：失败
     */
    bool GetClusterServer(std::string &ip, std::string &port) const;

    /**
     * @brief 获取kubeconfig中当前环境的客户端key
     *
     * @return kubeconfig中的客户端key
     */
    std::string GetUserKey() const;

    /**
     * @brief 获取kubeconfig中当前环境的客户端证书
     *
     * @return kubeconfig中的客户端证书
     */
    std::string GetUserCrt() const;

    /**
     * @brief 根据kubeconfig中的证书信息生成SSL Context
     *
     * @param sslCtx  [OUT]SSL上下文信息
     * @return true：成功 false：失败
     */
    bool SetSSLContext(boost::asio::ssl::context &sslCtx) const;

    /**
     * @brief 根据输入的kubeConfig文件，构造出KubeConfig结构体
     *
     * @param codedConfig 使用base64编码过的kubeConfig文件
     * @return [ true/false, KubeConfig ] 创建成功返回true，第二个KubeConfig有效。 创建失败返回false，说明解析失败
     */
    static std::pair<bool, KubeConfig> Create(const std::string &codedConfig);

private:
    KubeConfig(const ClusterInfo &clusterInfo, const UserInfo &userInfo) : m_clusterCA(clusterInfo.crt),
                                                                           m_clusterServer(clusterInfo.server),
                                                                           m_userKey(userInfo.privateKey),
                                                                           m_userCrt(userInfo.crt)
    {
    }

    KubeConfig()
    {
    }

    std::string m_clusterServer;
    std::string m_clusterCA;
    std::string m_userKey;
    std::string m_userCrt;
};
}