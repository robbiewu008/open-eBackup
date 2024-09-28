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
package openbackup.system.base.sdk.cluster.api;

import openbackup.system.base.sdk.cluster.model.BackupStorageUnit;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClusterStorageVo;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.sdk.cluster.model.StorageServiceIpInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;

import java.util.List;

/**
 * ClusterApi 本地调用API接口定义
 *
 */
public interface ClusterNativeApi {
    /**
     * 查询当前节点（同一ESN）集群的详细信息
     *
     * @return 集群详细信息
     */
    ClusterDetailInfo queryCurrentGroupClusterDetails();

    /**
     * 查询当前（同一ESN的小集群）的节点数量
     *
     * @return 节点数量
     */
    int queryCurrentGroupNodeCount();

    /**
     * 查询整个集群（整个备份集群）的详细信息
     *
     * @return 集群详细信息
     */
    ClusterDetailInfo queryPrimaryGroupClusterDetails();

    /**
     * 查询本地集群中的存储信息(当前ESN的集群)
     *
     * @return 本地集群的存储信息 {@code ClusterDetailInfo}
     */
    ClusterDetailInfo queryDecryptCurrentGroupClusterDetails();

    /**
     * 获取当前节点
     *
     * @return local cluster
     */
    ClustersInfoVo getCurrentClusterVoInfo();

    /**
     * 查询外部集群列表
     *
     * @param requestParm 请求参数
     * @return 外部集群列表
     */
    List<ClustersInfoVo> getTargetClusterList(TargetClusterRequestParm requestParm);

    /**
     * 查询外部集群列表具体信息
     *
     * @param requestParm 请求参数
     * @return 外部集群列表
     */
    List<ClusterDetailInfo> queryTargetClusterListDetails(TargetClusterRequestParm requestParm);

    /**
     * 查询本地存储节点信息
     *
     * @return 本地存储节点信息
     */
    StorageServiceIpInfo getCurrentGroupStorageEndpoint();

    /**
     * 查询外部存储节点信息
     *
     * @param clusterId 集群id
     * @param role 端点端口角色 4:复制端口
     * @return 本地存储节点信息
     */
    StorageServiceIpInfo getTargetStorageEndpoint(int clusterId, int role);

    /**
     * 查询备份集群存储容量
     *
     * @param clusterId 集群id
     * @return 存储容量
     */
    ClusterStorageVo getBackupClusterStorage(int clusterId);

    /**
     * 根据ID查询外部集群信息
     *
     * @param clusterId 外部集群ID
     * @return TargetClusterVo
     */
    TargetClusterVo getTargetClusterVoById(Integer clusterId);

    /**
     * 查询外部集群角色
     *
     * @param clusterId 外部集群ID
     * @return 外部集群角色
     */
    Integer getTargetClusterRole(Integer clusterId);

    /**
     * 查询本地集群管理IP列表
     *
     * @return 本地集群管理IP列表
     */
    List<String> queryLocalClusterManageIpList();

    /**
     * 查询当前集群（备份集群当前GROUP）管理IP列表
     *
     * @return 当前集群管理IP列表
     */
    List<String> queryCurrentGroupManageIpList();

    /**
     * 查询外部集群端口
     *
     * @param clusterId 外部集群ID
     * @return 外部集群端口
     */
    Integer getTargetClusterPort(Integer clusterId);

    /**
     * 根据clusteId List 获取备份存储单元List
     *
     * @param requestParm requestParm
     * @return List<BackupStorageUnit> List<BackupStorageUnit>
     */
    List<BackupStorageUnit> getBackupStorageUnitList(TargetClusterRequestParm requestParm);

    /**
     * 获取当前ESN
     *
     * @return 当前ESN
     */
    String getCurrentEsn();

    /**
     * 查询存储单元列表
     *
     * @param unitIds List<String>
     * @return List<StorageUnitVo>
     */
    List<StorageUnitVo> getStorageUnitListByUnitIds(List<String> unitIds);

    /**
     * 更新存储池告警阈值
     *
     * @param unitId 存储单元名称
     * @param threshold 阈值
     */
    void updateStoragePoolThreshold(String unitId, int threshold);
}
