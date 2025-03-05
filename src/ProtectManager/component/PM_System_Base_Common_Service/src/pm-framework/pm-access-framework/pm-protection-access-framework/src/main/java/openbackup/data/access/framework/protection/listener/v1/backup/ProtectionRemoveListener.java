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
package openbackup.data.access.framework.protection.listener.v1.backup;

import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.TargetClusterService;
import com.huawei.oceanprotect.repository.entity.DistributionStorageUnitRelation;
import com.huawei.oceanprotect.repository.repository.NasDistributionStorageRepository;
import com.huawei.oceanprotect.repository.service.impl.NasDistributionStorageServiceImpl;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;
import com.huawei.oceanprotect.sla.common.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.BackupStorageTypeEnum;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicationProtectionRemoveProcessor;
import openbackup.data.access.framework.protection.service.replication.RepCommonService;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationRemoveProcessor;
import openbackup.data.protection.access.provider.sdk.backup.BackupProvider;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.system.base.common.cluster.BackupClusterConfigUtil;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.RedisConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.accesspoint.model.CleanRemoteRequest;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.protection.SlaRestApi;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.annotation.Resource;

/**
 * Protection Remove Listener
 *
 */
@Component
@Slf4j
public class ProtectionRemoveListener {
    private static final String EXTERNAL_SYSTEM_ID = "external_system_id";

    private static final String REPLICATION_TARGET_MODE = "replication_target_mode";

    private static final String NEW_SLA_KEY = "new_sla";

    private static final String OLD_SLA_KEY = "old_sla";

    private static final String RESOURCE_ID_KEY = "resource_id";

    private static final String SLA_ID_KEY = "sla_id";

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private ResourceRestApi resourceRestApi;

    @Autowired
    private SlaRestApi slaRestApi;

    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private UnifiedReplicationRemoveProcessor unifiedReplicationRemoveProcessor;

    @Resource
    private NasDistributionStorageServiceImpl nasDistributionStorageService;

    @Autowired
    private ProtectedObjectMapper protectedObjectMapper;

    @Autowired
    private TargetClusterService targetClusterService;

    @Autowired
    private NasDistributionStorageRepository nasDistributionStorageRepository;

    @Autowired
    private DmeReplicationRestApi dmeReplicationRestApi;

    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private IStorageDeviceRepository repository;

    @Value("${repository.storage.port}")
    private int storagePort;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private RepCommonService repCommonService;

    /**
     * Consume protection remove topic message
     *
     * @param payload payload
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.PROTECTION_REMOVE_EVENT, groupId = "replicationConsumerGroup")
    public void removeProtection(String payload, Acknowledgment acknowledgment) {
        removeRelatedReplication(payload);
        removeProtectionPlan(payload);
        removeIntraReplicationLink(payload);
    }

    private void removeIntraReplicationLink(String payload) {
        log.info("start remote intra remote link");
        // 域内复制删除复制链路
        JSONObject data = JSONObject.fromObject(payload);
        String slaId = data.getString(SLA_ID_KEY);
        boolean isRemoveLine = data.getBoolean("is_remove_line");
        if (!isRemoveLine) {
            log.info("Create protection not remote intra remote link");
            return;
        }
        SlaBo slaBo = slaRestApi.querySlaById(slaId);
        List<PolicyBo> policyBos = getIntraReplicationPolicies(slaBo);
        int size = policyBos.size();
        log.info("intra replication policy count: {}", size);
        // 如果当前域内sla没有被其它资源使用 则删除复制链路
        if (protectedObjectMapper.countBySlaId(slaId) == 0 && size > 0) {
            log.info("bound sla count is 0");
            HashSet<String> storageUnitIds = new HashSet<>();
            policyBos.forEach(policyBo -> {
                Object storageUnitGroupIdObject = JSONObject.fromObject(policyBo.getExtParameters())
                        .get(CopyPropertiesKeyConstant.KEY_REPLICATE_EXTERNAL_REPOSITORY_ID);
                if (storageUnitGroupIdObject instanceof String) {
                    String storageUnitGroupId = (String) storageUnitGroupIdObject;
                    List<String> storageUnitGroupIds = new ArrayList<>();
                    storageUnitGroupIds.add(storageUnitGroupId);
                    List<DistributionStorageUnitRelation> storageRelationList =
                            nasDistributionStorageRepository.getStorageRelationsByDistributionIds(storageUnitGroupIds);
                    storageUnitIds.addAll(storageRelationList.stream()
                            .map(DistributionStorageUnitRelation::getUnitId)
                            .collect(Collectors.toList()));
                }
            });
            log.info("cluster ids need to clean :{}", storageUnitIds);
            List<Integer> clusterIdList =
                    nasDistributionStorageRepository.getStorageUnitIdList(new ArrayList<>(storageUnitIds));
            List<TargetCluster> targetClusters = targetClusterService.getTargetClusterById(clusterIdList);
            targetClustersCleanRemoteRequest(targetClusters);
        }
    }

    private void targetClustersCleanRemoteRequest(List<TargetCluster> targetClusters) {
        targetClusters.forEach(targetCluster -> {
            CleanRemoteRequest cleanRemoteRequest = new CleanRemoteRequest();
            cleanRemoteRequest.setLocalDevice(buildLocalDevice());
            if (BackupClusterConfigUtil.getBackupClusterEsn().equals(targetCluster.getRemoteEsn())) {
                cleanRemoteRequest.setRemoteDevice(buildRemoteDevice(targetCluster));
                log.info("clean remote locally");
                dmeReplicationRestApi.cleanRemote(cleanRemoteRequest);
            } else {
                cleanRemoteRequest.setRemoteDevice(buildRemoteDevice(targetCluster));
                MemberClusterBo memberClusterByEsn =
                        memberClusterService.getMemberClusterByEsn(targetCluster.getRemoteEsn());
                try {
                    log.info("clean remote in device: {}", targetCluster.getRemoteEsn());
                    memberClusterService.dispatchCleanRemoteLink(memberClusterByEsn, cleanRemoteRequest);
                } catch (LegoCheckedException | LegoUncheckedException e) {
                    log.info("clean remote in device: {} fail", targetCluster.getRemoteEsn(),
                            ExceptionUtil.getErrorMessage(e));
                }
            }
        });
    }

    private static List<PolicyBo> getIntraReplicationPolicies(SlaBo slaBo) {
        // 获取域内复制策略
        return slaBo.getPolicyList().stream().filter(policyBo -> {
            Integer replicationMode =
                    policyBo.getIntegerFormExtParameters(REPLICATION_TARGET_MODE, ReplicationMode.EXTRA.getValue());
            return replicationMode.equals(ReplicationMode.INTRA.getValue());
        }).collect(Collectors.toList());
    }

    private DmeLocalDevice buildLocalDevice() {
        DmeLocalDevice localDevice = new DmeLocalDevice();
        localDevice.setMgrIp(clusterQueryService.queryCurrentGroupManageIpList());
        StorageDevice device = repository.findLocalStorage(false);
        localDevice.setUserName(device.getUserName());
        localDevice.setPassword(device.getPassword());
        localDevice.setPort(storagePort);
        return localDevice;
    }

    private List<DmeRemoteDevice> buildRemoteDevice(TargetCluster targetCluster) {
        ArrayList<DmeRemoteDevice> dmeRemoteDevices = new ArrayList<>();
        ClusterDetailInfo primaryGroupClustersDetail = clusterQueryService.getPrimaryGroupClustersDetail();
        primaryGroupClustersDetail.getAllMemberClustersDetail().stream().filter(
                clusterDetailInfo -> !clusterDetailInfo.getTargetClusterVo().getEsn()
                        .equals(targetCluster.getRemoteEsn()))
                .forEach(clusterDetailInfo -> {
                    DmeRemoteDevice dmeRemoteDevice = new DmeRemoteDevice();
                    dmeRemoteDevice.setPort(storagePort);
                    dmeRemoteDevice.setPortPm(clusterDetailInfo.getTargetClusterVo().getPort());
                    dmeRemoteDevice.setEsn(clusterDetailInfo.getTargetClusterVo().getEsn());
                    dmeRemoteDevice.setUserNamePm(clusterDetailInfo.getTargetClusterVo().getUsername());
                    dmeRemoteDevice.setPasswordPm(clusterDetailInfo.getTargetClusterVo().getPassword());
                    dmeRemoteDevice.setMgrIpList(clusterDetailInfo.getTargetClusterVo().getMgrIpList());
                    dmeRemoteDevice.setNetPlaneInfo(
                            JSONObject.writeValueAsString(clusterDetailInfo.getStorageSystem().getNetplaneInfo()));
                    dmeRemoteDevice.setNetworkInfo(
                        JSONObject.fromObject(clusterDetailInfo.getStorageSystem().getDeviceNetworkInfo()).toString());
                    dmeRemoteDevices.add(dmeRemoteDevice);
                });
        return dmeRemoteDevices;
    }

    /**
     * Consume protection change topic message
     *
     * @param payload payload
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.PROTECTION_CHANGE_EVENT)
    public void changeProtection(String payload, Acknowledgment acknowledgment) {
        removeRelatedReplication(payload);
    }

    private void removeProtectionPlan(String payload) {
        JSONObject data = JSONObject.fromObject(payload);
        String resourceId = data.getString(RESOURCE_ID_KEY);
        ResourceEntity resourceEntity = resourceRestApi.queryResource(resourceId);
        if (resourceEntity == null) {
            log.info("resourceEntity not found, no need to do anything.");
            return;
        }
        BackupProvider protectionProvider = providerManager.findProvider(BackupProvider.class,
            resourceEntity.getSubType(), null);
        if (protectionProvider != null) {
            ProtectedObject protectedObject = new ProtectedObject();
            protectedObject.setResourceId(resourceId);
            protectionProvider.remove(protectedObject);
        }
    }

    private void removeRelatedReplication(String payload) {
        JSONObject data = JSONObject.fromObject(payload);
        String resourceId = data.getString(RESOURCE_ID_KEY);
        String slaId = data.getString(SLA_ID_KEY);
        SlaBo slaBo = slaRestApi.querySlaById(slaId);
        ResourceEntity resourceEntity = resourceRestApi.queryResource(resourceId);
        if (resourceEntity == null) {
            log.info("resourceEntity not found, no need to do anything.");
            return;
        }
        ReplicationProtectionRemoveProcessor processor = providerManager.findProviderOrDefault(
            ReplicationProtectionRemoveProcessor.class, resourceEntity.getSubType(), unifiedReplicationRemoveProcessor);
        if (processor == null) {
            log.info("ProtectionRemoveProcessor not found, no need to do anything.");
            return;
        }

        List<PolicyBo> policies = slaBo.getPolicyList();
        if (VerifyUtil.isEmpty(policies)) {
            return;
        }
        List<PolicyBo> replicationPolicies = policies.stream()
                .filter(policy -> SlaPolicyTypeEnum.REPLICATION.getName().equals(policy.getType()))
                .collect(Collectors.toList());
        for (PolicyBo policy : replicationPolicies) {
            removeRelatedReplicationResource(resourceEntity, processor, policy);
        }
    }

    private void removeRelatedReplicationResource(ResourceEntity resourceEntity,
        ReplicationProtectionRemoveProcessor processor, PolicyBo policy) {
        log.info("policy id:{}", policy.getUuid());
        Integer mode = policy.getIntegerFormExtParameters(REPLICATION_TARGET_MODE, ReplicationMode.EXTRA.getValue());
        JsonNode storageInfo = policy.getExtParameters().get(ExtParamsConstants.STORAGE_INFO);
        // 移除保护，域外
        if (SlaConstants.EXTRA_REPLICATION_MODE.contains(mode)) {
            Optional<TargetClusterVo> targetClusterVo = getReplicationTargetCluster(policy);
            if (targetClusterVo.isPresent()) {
                TargetClusterVo targetCluster = targetClusterVo.get();
                if (deployTypeService.isE1000()) {
                    String storageId = storageInfo.get(ExtParamsConstants.STORAGE_ID).asText();
                    Map<String, String> queryParams = new HashMap<>();
                    queryParams.put("id", storageId);
                    StorageUnitVo storageUnitVo = repCommonService.getStorageUnitVos(targetCluster, queryParams).get(0);
                    targetCluster.setEsn(storageUnitVo.getDeviceId());
                }
                String externalSystemId = targetCluster.getClusterId();
                String key = RedisConstants.TARGET_CLUSTER_RELATED_TASKS + externalSystemId;
                RMap<String, String> map = redissonClient.getMap(key, StringCodec.INSTANCE);
                long count = map.entrySet()
                    .stream()
                    .filter(entry -> Objects.equals(resourceEntity.getUuid(), entry.getValue()))
                    .count();
                if (count > 0) {
                    log.info("Some running tasks exist. resource: {}, target cluster: {}", resourceEntity.getUuid(),
                        externalSystemId);
                    return;
                }
                processor.process(resourceEntity, targetCluster);
            }
        } else {
            String storageType = storageInfo.get(ExtParamsConstants.STORAGE_TYPE).asText();
            String storageId = storageInfo.get(ExtParamsConstants.STORAGE_ID).asText();
            if (BackupStorageTypeEnum.BACKUP_STORAGE_UNIT_GROUP.getValue().equals(storageType)) {
                Optional<NasDistributionStorageDetail> replicationStorage =
                    Optional.ofNullable(nasDistributionStorageService.getDetail(storageId));
                replicationStorage.ifPresent(detail -> processor.process(resourceEntity, detail));
            } else {
                Optional<StorageUnitVo> storageUnitVo = repCommonService.getStorageUnitVo(storageId);
                storageUnitVo.ifPresent(unit -> processor.process(resourceEntity, unit));
            }
        }
    }

    private Optional<TargetClusterVo> getReplicationTargetCluster(PolicyBo policy) {
        JsonNode externalSystemIdNode = policy.getExtParameters().findPath(EXTERNAL_SYSTEM_ID);
        if (externalSystemIdNode.isMissingNode()) {
            return Optional.empty();
        } else {
            return Optional.of(clusterInternalApi.queryTargetClusterDetailsByClusterId(externalSystemIdNode.asInt()));
        }
    }

    /**
     * on sla changed
     *
     * @param payload payload
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.SLA_CHANGED_EVENT)
    public void onSlaChanged(String payload, Acknowledgment acknowledgment) {
        JSONObject data = JSONObject.fromObject(payload);
        SlaBo oldSla = data.getBean(OLD_SLA_KEY, SlaBo.class);
        SlaBo newSla = data.getBean(NEW_SLA_KEY, SlaBo.class);
        List<String> policyUuidList = getPolicies(newSla).stream().map(PolicyBo::getUuid).collect(Collectors.toList());
        List<PolicyBo> policies = getPolicies(oldSla);

        for (PolicyBo policy : policies) {
            String policyUuid = policy.getUuid();
            if (!policyUuidList.contains(policyUuid)
                && SlaPolicyTypeEnum.REPLICATION.getName().equals(policy.getType())) {
                removeSlaRelatedReplicationResource(oldSla, policy);
            }
        }
    }

    private void removeSlaRelatedReplicationResource(SlaBo sla, PolicyBo policy) {
        int page = 0;
        List<ProtectedObjectInfo> objects;
        do {
            BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(sla.getUuid(), page,
                IsmNumberConstant.TEN);
            objects = data.getItems();
            for (ProtectedObjectInfo object : objects) {
                ResourceEntity resourceEntity = resourceRestApi.queryResource(object.getResourceId());
                ReplicationProtectionRemoveProcessor processor = providerManager.findProviderOrDefault(
                    ReplicationProtectionRemoveProcessor.class,
                    resourceEntity.getSubType(),
                    unifiedReplicationRemoveProcessor);
                removeRelatedReplicationResource(resourceEntity, processor, policy);
            }
            page++;
        } while (objects.size() >= IsmNumberConstant.TEN);
    }

    private List<PolicyBo> getPolicies(SlaBo oldSla) {
        return oldSla != null ? oldSla.getPolicyList() : Collections.emptyList();
    }
}
