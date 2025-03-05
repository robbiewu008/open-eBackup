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

import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.util.ClusterUriUtil;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.filesystem.QueryReplicationPairReq;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.filesystem.ReplicationPairResponse;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicationProtectionRemoveProcessor;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.model.DmeRemovePairRequest;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Protection Remove Listener
 *
 */
@Component
@Slf4j
public class UnifiedReplicationRemoveProcessor implements ReplicationProtectionRemoveProcessor {
    private static final List<String> SUPPORTED_RESOURCE_SUB_TYPES = Stream.of(ResourceSubTypeEnum.VMWARE,
            ResourceSubTypeEnum.ORACLE, ResourceSubTypeEnum.IMPORT_COPY)
        .map(ResourceSubTypeEnum::getType)
        .collect(Collectors.toList());

    @Value("${repository.storage.port}")
    private int storagePort;

    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    @Qualifier("targetClusterApiWithDmaProxyManagePort")
    private TargetClusterRestApi dmaTargetClusterRestApi;

    @Autowired
    private ArrayTargetClusterService arrayTargetClusterService;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private AuthNativeApi authNativeApi;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DmeReplicationRestApi dmeReplicationRestApi;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private RepCommonService repCommonService;

    /**
     * process protection remove event
     *
     * @param resourceEntity resource entity
     * @param targetCluster targetCluster
     */
    @Override
    public void process(ResourceEntity resourceEntity, TargetClusterVo targetCluster) {
        log.info("start dispatch remove dme pair");
        DmeRemovePairRequest request = buildDmeRemovePairRequest(resourceEntity, targetCluster);
        if (deployTypeService.isE1000()) {
            List<Copy> copies = copyRestApi.queryCopiesByResourceId(resourceEntity.getUuid());
            Map<String, List<Copy>> copyByUnit = copies.stream().collect(Collectors.groupingBy(Copy::getStorageUnitId));
            copyByUnit.keySet().forEach(unitId -> {
                repCommonService.fillLocalDevice(request.getLocalDevice(), unitId);
                Optional<StorageUnitVo> storageUnitVo = repCommonService.getStorageUnitVo(unitId);
                if (!storageUnitVo.isPresent()) {
                    throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Local storage not exist.");
                }
                String deviceId = storageUnitVo.get().getDeviceId();
                List<ReplicationPairResponse> replicationPairs = getPairs(resourceEntity, deviceId);
                if (!VerifyUtil.isEmpty(replicationPairs)) {
                    List<String> targetEsns = new ArrayList<>();
                    replicationPairs.forEach(pair -> targetEsns.add(pair.getRemoteDeviceEsn()));
                    request.setTargetEsns(targetEsns);
                }
                dmeReplicationRestApi.removePair(request);
            });
            log.info("Send local remove pair request to dme success");
            return;
        }
        memberClusterService.consumeInAllMembers(memberClusterService::dispatchRemoveReplicationPair, request);
        log.info("remove pair request send to dme success, resource id:{}", request.getResourceId());
    }

    @Override
    public void process(ResourceEntity resourceEntity, StorageUnitVo storageUnitVo) {
        log.info("start dispatch remove dme pair");
        List<String> esnList = Collections.singletonList(storageUnitVo.getDeviceId());
        DmeRemovePairRequest request = buildDmeRemovePairRequest(resourceEntity, esnList);
        dmeReplicationRestApi.removePair(request);
        log.info("remove pair request send to dme success, resource id:{}", request.getResourceId());
    }

    @Override
    public void process(ResourceEntity resourceEntity, NasDistributionStorageDetail nasDistributionStorageDetail) {
        List<String> esnList = nasDistributionStorageDetail.getUnitList()
            .stream()
            .map(BackupUnitVo::getBackupClusterVo)
            .map(BackupClusterVo::getStorageEsn)
            .collect(Collectors.toList());
        log.info("start dispatch intra remove dme pair");
        DmeRemovePairRequest request = buildDmeRemovePairRequest(resourceEntity, esnList);
        log.info("process remove request:{}", request);
        nasDistributionStorageDetail.getUnitList().forEach(backupUnitVo -> {
            BackupClusterVo backupClusterVo = backupUnitVo.getBackupClusterVo();
            DmeRemovePairRequest pairRequest = BeanTools.clone(request);
            List<String> updateEsnList = pairRequest.getTargetEsns()
                .stream()
                .filter(s -> !StringUtils.equals(backupClusterVo.getStorageEsn(), s))
                .collect(Collectors.toList());
            pairRequest.setTargetEsns(updateEsnList);
            log.info("Process remove request forward clusterId: {}, requestId: {}", backupClusterVo.getClusterId(),
                pairRequest.getResourceId());
            URI uri = ClusterUriUtil.buildURI(backupClusterVo.getIp(), backupClusterVo.getPmPort());
            String token = authNativeApi.generateClusterAdminToken();
            try {
                dmaTargetClusterRestApi.dispatchRemoveReplicationPair(uri, token, pairRequest);
                log.info("Process remove request forward clusterId: {} success", backupClusterVo.getClusterId());
            } finally {
                StringUtil.clean(token);
            }
        });
        log.info("remove intra pair request send to dme success, resource id:{}", request.getResourceId());
    }

    private DmeRemovePairRequest buildDmeRemovePairRequest(ResourceEntity resourceEntity, TargetClusterVo clusterVo) {
        log.info("start to build remote device esn");
        DmeRemovePairRequest request = new DmeRemovePairRequest();
        request.setLocalDevice(DmeLocalDevice.build(clusterInternalApi));
        if (ResourceSubTypeEnum.IMPORT_COPY.getType().equals(resourceEntity.getSubType())) {
            request.setResourceId(resourceEntity.getName());
        } else {
            request.setResourceId(resourceEntity.getUuid());
        }
        List<String> targetEsns = new ArrayList<>();
        if (!deployTypeService.isE1000()) {
            try {
                String deviceId = memberClusterService.getCurrentClusterEsn();
                List<ReplicationPairResponse> replicationPairs = getPairs(resourceEntity, deviceId);
                if (!VerifyUtil.isEmpty(replicationPairs)) {
                    log.info("Replication pair size:{}", replicationPairs.size());
                    replicationPairs.forEach(pair -> targetEsns.add(pair.getRemoteDeviceEsn()));
                    request.setTargetEsns(targetEsns);
                }
            } catch (LegoCheckedException | LegoUncheckedException e) {
                log.info("fail to get replication target esns", ExceptionUtil.getErrorMessage(e));
            }
        }
        log.info("target esn size:{}", targetEsns.size());
        request.setTargetEsns(targetEsns);
        DmeRemoteDevice remoteDevice = buildRemoteDevice(clusterVo);
        request.setRemoteDevice(remoteDevice);
        return request;
    }

    private List<ReplicationPairResponse> getPairs(ResourceEntity resourceEntity, String deviceId) {
        QueryReplicationPairReq param = new QueryReplicationPairReq();
        param.setLocalResType("40");
        param.setLocalResName(resourceEntity.getUuid());
        return repCommonService.queryReplicationPair(deviceId, UserUtils.getBusinessUsername(), param);
    }

    private DmeRemovePairRequest buildDmeRemovePairRequest(ResourceEntity resourceEntity, List<String> esnList) {
        DmeRemovePairRequest request = new DmeRemovePairRequest();
        request.setLocalDevice(DmeLocalDevice.build(clusterInternalApi));
        if (ResourceSubTypeEnum.IMPORT_COPY.getType().equals(resourceEntity.getSubType())) {
            request.setResourceId(resourceEntity.getName());
        } else {
            request.setResourceId(resourceEntity.getUuid());
        }
        request.setTargetEsns(esnList);
        return request;
    }

    private DmeRemoteDevice buildRemoteDevice(TargetClusterVo targetCluster) {
        DmeRemoteDevice remoteDevice = new DmeRemoteDevice();
        remoteDevice.setPort(storagePort);
        remoteDevice.setPortPm(targetCluster.getPort());
        remoteDevice.setEsn(targetCluster.getEsn());
        remoteDevice.setUserNamePm(targetCluster.getUsername());
        remoteDevice.setPasswordPm(targetCluster.getPassword());
        remoteDevice.setMgrIpList(targetCluster.getMgrIpList());
        String netplane = JSONObject.writeValueAsString(targetCluster.getNetplaneInfo());
        remoteDevice.setNetPlaneInfo(netplane);
        return remoteDevice;
    }

    /**
     * applicable
     *
     * @param object object
     * @return result
     */
    @Override
    public boolean applicable(String object) {
        return SUPPORTED_RESOURCE_SUB_TYPES.contains(object);
    }
}
