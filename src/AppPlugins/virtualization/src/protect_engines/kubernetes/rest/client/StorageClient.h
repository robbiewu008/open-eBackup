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
#ifndef VIRTUALIZATION_PLUGIN_STORAGE_CLIENT_H
#define VIRTUALIZATION_PLUGIN_STORAGE_CLIENT_H

#include <string>
#include "protect_engines/kubernetes/common/KubeMacros.h"
#include "protect_engines/kubernetes/common/KubeCommonInfo.h"
#include "curl_http/CurlHttpClient.h"
#include "curl_http/HttpClientInterface.h"

namespace KubernetesPlugin {
class StorageClient {
public:
    StorageClient() {};

    StorageClient(const std::string &ip, int port, const AccessAuthParam &accessAuthParam,
        const StorageDeviceAuthData &storageDeviceAuthData, const std::vector<std::string> &ipList)
        : m_ip(ip), m_port(port), accessAuthParam(accessAuthParam),
        storageDeviceAuthData(storageDeviceAuthData), m_ipList(ipList) {}

    ~StorageClient();

    /**
    * @brief 根据IP、端口、认证信息创建StorageClient
    *
    * @param codedConfig 使用base64编码过的kubeConfig文件
    * @return 存储客户端
    */
    static std::pair<int32_t, std::shared_ptr<StorageClient>> Create(const std::string &ip, int port,
        const AccessAuthParam &accessAuthParam, const std::string &ipList, bool isReTry = true);

    static void CheckIpValidty(const std::string &ip, int port, const std::string &ipList,
        const AccessAuthParam &accessAuthParam, std::string &errorStr);

    /**
    * @brief 获取存储基本信息
    *
    * @return  存储基本信息
    */
    std::pair<int32_t, StorageDeviceInfo> GetDeviceBaseInfo();

    /**
    * @brief 获取LUN基本信息
    *
    * @return LUN基本信息
    */
    std::pair<int32_t, LunInfoData> GetLunInfoData(const std::string &lunName);

    /**
     * @brief 创建快照
     *
     * @param snapshotCreateParam 创建快照参数
     * @return
     */
    std::pair<int32_t, SnapshotInfoData> CreateSnapshot(SnapshotCreateParam &snapshotCreateParam);

    /**
    * @brief 查询快照
    *
    * @param snapshotId 快照ID
    * @return
    */
    std::pair<int32_t, SnapshotInfoData> QuerySnapshot(const std::string &snapshotId);

    /**
    * @brief 查询指定Lun的所有快照
    *
    * @param lunName Lun名称
    * @return
    */
    std::pair<int32_t, SnapshotInfoResponse> GetLunSnapshots(const std::string &lunName);

    /**
     * @brief 取消激活快照
     *
     * @param snapshotId 快照ID
     */
    int32_t StopSnapshot(const std::string &snapshotId);

    /**
     * @brief 激活一致性快照
     *
     * @param snapshotIds 快照ID集合
     */
    int32_t ActivateSnapshot(const std::vector <std::string> &snapshotIds);

    /**
     * @brief 删除快照
     *
     * @param snapshotId 快照ID
     */
    int32_t DeleteSnapshot(const std::string &snapshotId);

    /**
     * @brief 将快照从lun组移除
     *
     * @param snapshotId 快照ID
     */
    int32_t RemoveSnapshotFromLunGroupExp(const std::string &snapshotId);

    /**
     * @brief 关联查询Lun组
     *
     * @param objId 关联元素id
     *
     * @param objType 关联元素类型
     *
     * @param LunGroupList LUN组列表
     */
    int32_t QueryAssociateLunGroup(const std::string &objId, const std::string &objType,
        std::vector<LunGroupInfo> &LunGroupList);

    /**
     * @brief 将元素从Lun组移除
     *
     * @param objId 关联元素id
     *
     * @param objType 关联元素类型
     *
     * @param LunGroupList LUN组列表
     */
    int32_t RemoveObjFromLunGroup(const std::string &objId, const std::string &objType,
        std::vector<LunGroupInfo> &LunGroupList);

    /**
     * @brief 查询对应id的存储池
     *
     * @param poolId 关联存储池id
     *
     * @param poolData 查询存储池信息结果
     */
    int32_t QueryStoragePool(const std::string &poolId, StoragePoolData &poolData);

    std::string GetStorageSnId();

private:
    /**
     * @brief 退出登录
     *
     * @return
     */
    int32_t Logout();

    /**
    * @brief 用户Token过期或者权限失效时，进行重新登录
    *
    * @return
    */
    int32_t ReLogin();

    std::pair<int32_t, std::string> SendRequest(const std::string &method, const std::string &uri,
                                                 const std::string &body);

    static int32_t Send(const Module::HttpRequest &req, HttpResponseInfo &httpResponse,
        bool isRetry = true);

    static std::pair<int32_t, StorageDeviceAuthResponse> CheckAccessAuthentication(
        const std::string &ip, int port, const AccessAuthParam &accessAuth, bool isRetry = true);

    static void InitHttpRequest(Module::HttpRequest &req);

    static bool IsNetworkError(const uint32_t statusCode);

    static bool IsIpError(const uint32_t statusCode);

    static bool IsNoAuthToAccess(const uint32_t statusCode);

    static std::mutex CreateClientMutex; // 用于客户端的互斥创建

    static std::mutex ReCheckAccessAuthMutex; // 用于重新登录认证互斥访问

    std::string m_ip;
    std::vector<std::string> m_ipList;
    int m_port{0};
    AccessAuthParam accessAuthParam;
    StorageDeviceAuthData storageDeviceAuthData;
};

// 缓存客户端实例key:ip+port+username+userpassword  Value:客户端
static std::map <std::string, std::shared_ptr<StorageClient>> StorageClientMap{};
}

#endif // VIRTUALIZATION_PLUGIN_KUBECLIENT_H