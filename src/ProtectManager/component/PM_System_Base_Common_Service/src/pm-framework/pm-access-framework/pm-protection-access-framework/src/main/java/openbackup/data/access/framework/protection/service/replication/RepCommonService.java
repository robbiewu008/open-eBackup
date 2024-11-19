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
package openbackup.data.access.framework.protection.service.replication;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 复制公共方法
 *
 */

@Slf4j
@Component
public class RepCommonService {
    private static final int BASIC_DISK_PORT = -1;

    private static final int DEFAULT_PORT = 8088;

    @Autowired
    private StorageUnitService storageUnitService;

    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private ArrayTargetClusterService arrayTargetClusterService;

    /**
     * 填充本地设备
     *
     * @param localDevice 本地设备
     * @param unitId 存储单元ID
     */
    public void fillLocalDevice(DmeLocalDevice localDevice, String unitId) {
        Optional<StorageUnitVo> storageUnit = storageUnitService.getStorageUnitById(unitId);
        if (!storageUnit.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Local storage not exist.");
        }

        ClusterDetailInfo clusterDetail = clusterInternalApi.queryClusterDetails();
        ClusterDetailInfo localCluster = clusterDetail.getAllMemberClustersDetail().stream()
                .filter(clusterDetailInfo -> StringUtils.equals(
                        clusterDetailInfo.getStorageSystem().getStorageEsn(), storageUnit.get().getDeviceId()))
                .findFirst().orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "Local storage not exist."));
        localDevice.setPassword(localCluster.getStorageSystem().getPassword());
        localDevice.setUserName(localCluster.getStorageSystem().getUsername());
        int storagePort = localCluster.getStorageSystem().getStoragePort();
        int port = storagePort == BASIC_DISK_PORT ? DEFAULT_PORT : storagePort;
        localDevice.setPort(port);
        localDevice.setEsn(storageUnit.get().getDeviceId());
        localDevice.setMgrIp(clusterDetail.getSourceClusters().getMgrIpList());
    }

    /**
     * 获取目标存储单元信息
     *
     * @param cluster 目标集群
     * @param queryParams 查询参数
     * @return 目标存储单元信息
     */
    public List<StorageUnitVo> getStorageUnitVos(TargetClusterVo cluster, Map<String, String> queryParams) {
        List<StorageUnitVo> remoteStorageUnits =
            getRemoteStorageUnit(queryParams, Integer.parseInt(cluster.getClusterId())).getRecords();
        if (VerifyUtil.isEmpty(remoteStorageUnits)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The storage unit belongs to the target cluster not exist.");
        }
        return remoteStorageUnits;
    }

    /**
     * 获取目标存储单元信息
     *
     * @param queryParam 查询参数
     * @param clusterId 集群id
     * @return 目标存储单元信息
     */
    public PageListResponse<StorageUnitVo> getRemoteStorageUnit(Map<String, String> queryParam, Integer clusterId) {
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        TargetCluster targetCluster = clusterQueryService.getTargetClusterById(clusterId);
        try {
            response = arrayTargetClusterService.getStorageUnitInfo(targetCluster, queryParam, 0, 1);
            return response;
        } catch (LegoUncheckedException e) {
            log.error("get all dp users failed.", ExceptionUtil.getErrorMessage(e));
        }
        return response;
    }
}
