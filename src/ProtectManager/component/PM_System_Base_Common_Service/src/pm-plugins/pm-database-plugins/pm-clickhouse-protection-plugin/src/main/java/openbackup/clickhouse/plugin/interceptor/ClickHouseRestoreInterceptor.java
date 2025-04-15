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
package openbackup.clickhouse.plugin.interceptor;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;

import com.google.common.collect.Maps;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.apache.commons.collections.ListUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * ClickHouse恢复拦截器
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class ClickHouseRestoreInterceptor implements RestoreInterceptorProvider {
    private final ResourceService resourceService;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final CopyRestApi copyRestApi;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final ClickHouseAgentProvider clickHouseAgentProvider;

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(subType);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        advanceParams.put(ClickHouseConstant.ADVANCE_PARAMS_KEY_MULTI_POST_JOB, Boolean.TRUE.toString());
        task.setAdvanceParams(advanceParams);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        ProtectedResource targetResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "protectedResource is not exist"));

        targetResource.getDependencies().get(DatabaseConstants.CHILDREN).stream().forEach(child -> {
            AuthParamUtil.convertKerberosAuth(child.getAuth(), kerberosService,
                child.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID), encryptorService);
        });

        // 检查参数
        checkTargetNodes(task, targetResource);
        // 设置agent
        buildRestoreAgents(task, targetResource);
        // 填充节点信息
        supplyNodes(task, targetResource);
        // 填充目标环境扩展字段
        supplyTargetEnvExtendInfo(task);
        // 设置恢复模式
        setRestoreMode(task);
        // 细粒度恢复，补充子资源信息
        supplySubObjects(task);
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void checkTargetNodes(RestoreTask task, ProtectedResource targetResource) {
        log.info("check target nodes of task: {}", task.getTaskId());
        // 判断集群状态是否为离线，不离线则报错误
        if (!EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(task.getTargetEnv())) {
            log.error("target environment: {} link status is not online", task.getTargetEnv().getName());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "clickhouse cluster is not online.");
        }
        // 判断集群节点是否一致
        checkSameClusterNode(task.getCopyId(), targetResource);
        // 获取集群下节点
        List<ProtectedResource> targetNodes = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource resource : targetNodes) {
            // 判断目标节点是否都正常
            if (Integer.parseInt(resource.getExtendInfo().get(DatabaseConstants.STATUS))
                != ClusterEnum.StatusEnum.ONLINE.getStatus()) {
                log.error("target cluster node: {} status is not online", resource.getName());
                throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "node is not online.");
            }
        }
    }

    private void checkSameClusterNode(String copyId, ProtectedResource targetResource) {
        log.info("check is same cluster node of copy id: {} and target cluster: {}", copyId, targetResource.getName());
        DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(copyId);
        List<TaskEnvironment> srcNodes = copyInfo.getProtectEnv().getNodes();
        List<String> srcNodeInfos = srcNodes.stream()
            .map(node -> node.getExtendInfo().get(DatabaseConstants.CLUSTER_TARGET) + node.getExtendInfo()
                .get(ClickHouseConstant.SHARD_NUM) + node.getExtendInfo().get(ClickHouseConstant.SHARD_WEIGHT)
                + node.getExtendInfo().get(ClickHouseConstant.REPLICA_NUM))
            .sorted()
            .collect(Collectors.toList());
        // 获取目标集群下节点
        List<ProtectedResource> targetNodes = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<String> dstNodeInfos = targetNodes.stream()
            .map(node -> node.getExtendInfo().get(DatabaseConstants.CLUSTER_TARGET) + node.getExtendInfo()
                .get(ClickHouseConstant.SHARD_NUM) + node.getExtendInfo().get(ClickHouseConstant.SHARD_WEIGHT)
                + node.getExtendInfo().get(ClickHouseConstant.REPLICA_NUM))
            .sorted()
            .collect(Collectors.toList());
        log.info("src nodes info: {}, dst nodes info: {}", srcNodeInfos, dstNodeInfos);
        if (!ListUtils.isEqualList(dstNodeInfos, srcNodeInfos)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR,
                "src nodes info is not same with dst nodes info.");
        }
    }

    private void buildRestoreAgents(RestoreTask task, ProtectedResource targetResource) {
        log.info("build restore agents of task: {}", task.getTaskId());

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(targetResource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();

        task.setAgents(clickHouseAgentProvider.getSelectedAgents(agentSelectParam));
    }

    private void supplyNodes(RestoreTask task, ProtectedResource targetResource) {
        log.info("supply nodes of task: {}", task.getTaskId());
        // 获取集群下节点
        List<ProtectedResource> nodes = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<TaskEnvironment> nodeList = nodes.stream().map(this::toTaskEnvironment).collect(Collectors.toList());
        Map<String, String> originalAgentIds = getOriginalAgentIds(task);
        // 为每个节点添加agentId信息
        for (TaskEnvironment node : nodeList) {
            ProtectedResource agentResource = nodes.stream()
                .filter(n -> n.getUuid().equals(node.getUuid()))
                .collect(Collectors.toList())
                .get(0)
                .getDependencies()
                .get(DatabaseConstants.AGENTS)
                .get(0);
            Map<String, String> extendInfo = node.getExtendInfo();
            extendInfo.put(ClickHouseConstant.AGENT_ID, agentResource.getUuid());
            extendInfo.put(ClickHouseConstant.ORIGINAL_AGENT_ID, originalAgentIds.get(
                node.getExtendInfo().get(DatabaseConstants.CLUSTER_TARGET) + node.getExtendInfo()
                    .get(ClickHouseConstant.SHARD_NUM) + node.getExtendInfo().get(ClickHouseConstant.SHARD_WEIGHT)
                    + node.getExtendInfo().get(ClickHouseConstant.REPLICA_NUM)));
            node.setExtendInfo(extendInfo);
        }
        task.getTargetEnv().setNodes(nodeList);
    }

    private Map<String, String> getOriginalAgentIds(RestoreTask task) {
        log.info("get original agentIds of task: {}", task.getTaskId());
        DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(task.getCopyId());
        Map<String, String> originalAgentIds = Maps.newHashMap();

        List<TaskEnvironment> nodes = copyInfo.getProtectEnv().getNodes();
        for (TaskEnvironment node : nodes) {
            originalAgentIds.put(node.getExtendInfo().get(DatabaseConstants.CLUSTER_TARGET) + node.getExtendInfo()
                    .get(ClickHouseConstant.SHARD_NUM) + node.getExtendInfo().get(ClickHouseConstant.SHARD_WEIGHT)
                    + node.getExtendInfo().get(ClickHouseConstant.REPLICA_NUM),
                node.getExtendInfo().get(ClickHouseConstant.AGENT_ID));
        }

        return originalAgentIds;
    }

    private void supplyTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = task.getTargetEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void setRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("ClickHouse restore task: {}, copy id: {}, mode: {}", task.getTaskId(), task.getCopyId(),
            task.getRestoreMode());
    }

    private void supplySubObjects(RestoreTask task) {
        if (RestoreTypeEnum.FLR.getType().equals(task.getRestoreType())) {
            // 细粒度恢复
            DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(task.getCopyId());
            // 保护的对象，库or表集
            TaskResource protectObject = copyInfo.getProtectObject();
            String databaseName;
            if (ResourceTypeEnum.DATABASE.getType().equals(protectObject.getType())) {
                databaseName = protectObject.getName();
            } else if (ClickHouseConstant.TABLE_SET_TYPE.equals(protectObject.getType())) {
                databaseName = protectObject.getParentName();
            } else {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "protect object type is illegal");
            }

            task.getSubObjects().forEach(subObj -> subObj.setParentName(databaseName));
        }
    }

    private TaskEnvironment toTaskEnvironment(ProtectedResource protectedResource) {
        return JsonUtil.read(JsonUtil.json(protectedResource), TaskEnvironment.class);
    }
}
