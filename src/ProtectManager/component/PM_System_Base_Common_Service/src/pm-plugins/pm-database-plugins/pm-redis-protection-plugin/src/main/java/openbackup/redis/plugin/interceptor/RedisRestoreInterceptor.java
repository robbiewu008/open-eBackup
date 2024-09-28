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
package openbackup.redis.plugin.interceptor;

import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.ListUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Redis 数据库备份拦截器实现类
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class RedisRestoreInterceptor implements RestoreInterceptorProvider {
    private final ResourceService resourceService;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final CopyRestApi copyRestApi;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final RedisAgentProvider redisAgentProvider;

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param object 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.REDIS.getType().equals(object);
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 检查参数
        checkTargetNodes(task);
        // 设置agent
        buildRestoreAgents(task);
        // 填充节点信息
        supplyNodes(task);
        // 填充目标环境扩展字段
        supplyTargetEnvExtendInfo(task);
        // 设置恢复模式
        setRestoreMode(task);
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    /**
     * 特性开关
     *
     * @return RestoreFeature
     */
    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }

    /**
     * 检查目标节点是否符合恢复条件
     *
     * @param task 恢复任务
     */
    private void checkTargetNodes(RestoreTask task) {
        ProtectedResource targetResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not exist"));
        // 判断集群状态是否为离线，不离线则报错误
        if (EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(task.getTargetEnv())) {
            throw new LegoCheckedException(CommonErrorCode.REDIS_NODES_NOT_CLOSE, "redis cluster is online.");
        }
        // 判断槽位号是否一致
        checkSameSlots(task.getCopyId(), targetResource);
        // 获取集群下节点
        List<ProtectedResource> targetNodes = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource resource : targetNodes) {
            // 判断目标节点是否都关闭
            if (ClusterEnum.StatusEnum.ONLINE.getStatus() == Integer.parseInt(
                resource.getExtendInfo().get(DatabaseConstants.STATUS))) {
                throw new LegoCheckedException(CommonErrorCode.REDIS_NODES_NOT_CLOSE, "node is online.");
            }
            // 判断rdb存储位置是否存在
            if (StringUtils.isBlank(resource.getExtendInfo().get(RedisConstant.RDB_DIR)) || StringUtils.isBlank(
                resource.getExtendInfo().get(RedisConstant.RDB_DB_FILENAME))) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "rdb file path is empty.");
            }
            // 判断是否关闭了AOF持久化模式
            if (Integer.parseInt(resource.getExtendInfo().get(RedisConstant.AOF_ENABLED)) != 0) {
                throw new LegoCheckedException(CommonErrorCode.REDIS_NODE_AOF_ENABLE, "node aof enable");
            }
        }
    }

    private void checkSameSlots(String copyId, ProtectedResource targetResource) {
        DmeCopyInfo copyInfo = dmeUnifiedRestApi.getCopyInfo(copyId);
        List<TaskEnvironment> srcNodes = copyInfo.getProtectEnv().getNodes();
        List<String> srcSlots = srcNodes.stream()
            .filter(n -> StringUtils.isNotBlank(n.getExtendInfo().get(RedisConstant.SLOT)))
            .map(n -> n.getExtendInfo().get(RedisConstant.SLOT))
            .sorted()
            .collect(Collectors.toList());
        // 获取目标集群下节点
        List<ProtectedResource> targetNodes = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<String> dstSlots = targetNodes.stream()
            .filter(n -> StringUtils.isNotBlank(n.getExtendInfo().get(RedisConstant.SLOT)))
            .map(n -> n.getExtendInfo().get(RedisConstant.SLOT))
            .sorted()
            .collect(Collectors.toList());
        log.info("src slot: {}, dst slot: {}", srcSlots, dstSlots);
        if (!ListUtils.isEqualList(dstSlots, srcSlots)) {
            throw new LegoCheckedException(CommonErrorCode.REDIS_SRC_DST_SLOT_DIFF,
                "src slot is not same with dst slot.");
        }
    }

    /**
     * 添加恢复的agents到任务中
     *
     * @param task 恢复任务
     */
    private void buildRestoreAgents(RestoreTask task) {
        ProtectedResource protectedResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "protectedResource is not exist"));

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(protectedResource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();

        task.setAgents(redisAgentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 添加节点到任务中
     *
     * @param task 恢复任务
     */
    private void supplyNodes(RestoreTask task) {
        ProtectedResource protectedResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "protectedResource is not exist"));
        // 获取集群下节点
        List<ProtectedResource> redisNodes = protectedResource.getDependencies().get(DatabaseConstants.CHILDREN);
        redisNodes.stream()
            .forEach(redisNode -> AuthParamUtil.convertKerberosAuth(redisNode.getAuth(), kerberosService,
                redisNode.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                encryptorService));
        List<TaskEnvironment> nodeList = redisNodes.stream().map(this::toTaskEnvironment).collect(Collectors.toList());
        // 为每个节点添加agentId信息
        for (TaskEnvironment node : nodeList) {
            ProtectedResource agentResource = redisNodes.stream()
                .filter(n -> n.getUuid().equals(node.getUuid()))
                .collect(Collectors.toList())
                .get(0)
                .getDependencies()
                .get(DatabaseConstants.AGENTS)
                .get(0);
            Map<String, String> extendInfo = node.getExtendInfo();
            extendInfo.put(RedisConstant.AGENT_ID, agentResource.getUuid());
            node.setExtendInfo(extendInfo);
        }
        task.getTargetEnv().setNodes(nodeList);
    }

    private void supplyTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = task.getTargetEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void setRestoreMode(RestoreTask restoreTask) {
        Copy copy = copyRestApi.queryCopyByID(restoreTask.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            restoreTask.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            restoreTask.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("redis restore copy id: {}, mode: {}", restoreTask.getCopyId(), restoreTask.getRestoreMode());
    }

    private TaskEnvironment toTaskEnvironment(ProtectedResource protectedResource) {
        return JsonUtil.read(JsonUtil.json(protectedResource), TaskEnvironment.class);
    }
}
