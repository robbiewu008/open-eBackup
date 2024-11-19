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
package openbackup.system.base.sdk.infrastructure;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述: 基础设施查询逻辑
 *
 */
public interface InfrastructureService {
    /**
     * 查询节点 POD 信息
     *
     * @param podName POD名称
     * @return 基础设施返回的原生 POD 信息，未对其进行解析
     */
    List<NodePodInfo> queryNodePodInfo(String podName);

    /**
     * 查询pod和容器状态
     *
     * @return 基础设施返回的原生 POD 信息，未对其进行解析
     */
    List<NodePodInfo> queryInfraContainerInfo();

    /**
     * 查询节点网络信息，包括数据备份引擎IP、数据利用引擎IP、数据归档引擎IP、复制网络IP
     *
     * @return 节点网络信息，包括数据备份引擎IP、数据利用引擎IP、数据归档引擎IP、复制网络IP
     */
    Map<String, NodePodInfo> queryNodeNetInfo();

    /**
     *
     * 用于不向configmap中存网络信息的部署形态
     * 查询节点网络信息，包括数据备份引擎IP、数据利用引擎IP、数据归档引擎IP、复制网络IP
     *
     * @return 节点网络信息，包括数据备份引擎IP、数据利用引擎IP、数据归档引擎IP、复制网络IP
     */
    Map<String, NodePodInfo> queryNodeNetInfoV2();

    /**
     * 基础设施调用k8s接口获取宿主机节点信息
     *
     * @return 节点信息
     */
    List<NodeDetail> queryHostNodeInfo();

    /**
     * 获取pm节点的endpoint ip
     *
     * @return pm节点的endpoint ip
     */
    List<String> getAllPmEndpointIp();

    /**
     * 查询所有节点的所有的备份业务网络ip
     *
     * @return 查询结果
     */
    List<String> queryBackupEngineIps();

    /**
     * <节点名称， List<业务ip>>
     *
     * @return 查询结果
     */
    Map<String, List<String>> queryBackupEngineIpMap();

    /**
     * 获取备份，归档或者复制的业务网络配置信息
     *
     * @param netType 备份，归档，恢复
     * @return 查询结果
     */
    Map<String, List<String>> queryEngineIpMap(String netType);

    /**
     * 获取备份，归档或者复制的业务网络配置信息
     *
     * @param netType 备份，归档，恢复
     * @param nodeName 节点名称
     * @return 查询结果
     */
    List<String> queryEngineLogicIpList(String netType, String nodeName);

    /**
     * 通知dme业务网络变更，删除并等待protectengine的pod重启，重启后会监听新的业务网络
     */
    void notifyDmeBusinessNetworkChanged();

    /**
     * 获取备份，归档、复制的业务网络配置信息
     *
     * @return 查询结果
     */
    List<JSONObject> queryNetworkInfo();

    /**
     * 写业务网络信息到configmap
     *
     * @param netPlaneName 备份，归档或者复制
     * @param netPlaneInfo 备份，归档或者复制业务网络信息
     */
    void saveBusinessIpInfoToConfigMap(String netPlaneName, String netPlaneInfo);

    /**
     * 获取configMap中的值
     *
     * @param configMap configmap
     * @param key key
     * @return value
     */
    Optional<JSONObject> getConfigMapValue(String configMap, String key);
}