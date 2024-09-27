/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.replication;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.util.ClusterUriUtil;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicationProtectionRemoveProcessor;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.DmeRemovePairRequest;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.net.URI;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Protection Remove Listener
 *
 * @author m00576658
 * @since 2021-01-04
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
        memberClusterService.consumeInAllMembers(memberClusterService::dispatchRemoveReplicationPair, request);
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
        try {
            TargetCluster targetCluster = clusterQueryService.getTargetClusterById(
                Integer.parseInt(clusterVo.getClusterId()));
            ClusterDetailInfo targetClusterInfo = arrayTargetClusterService.getTargetClusterDetailsInfo(targetCluster)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "cluster not exist"));
            if (targetClusterInfo.getAllMemberClustersDetail() == null) {
                targetEsns.add(targetClusterInfo.getStorageSystem().getStorageEsn());
            } else {
                targetClusterInfo.getAllMemberClustersDetail().forEach(clusterDetailInfo -> {
                    targetEsns.add(clusterDetailInfo.getStorageSystem().getStorageEsn());
                });
            }
        } catch (LegoCheckedException | LegoUncheckedException e) {
            log.info("fail to get replication target esns", ExceptionUtil.getErrorMessage(e));
        }
        log.info("target esn size:{}", targetEsns.size());
        request.setTargetEsns(targetEsns);
        DmeRemoteDevice remoteDevice = buildRemoteDevice(clusterVo);
        request.setRemoteDevice(remoteDevice);
        return request;
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
