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
package openbackup.database.base.plugin.interceptor;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.util.CollectionUtils;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.stream.Collectors;

/**
 * 数据备份插件通用功能
 *
 */
@Slf4j
public abstract class AbstractDbBackupInterceptor implements BackupInterceptorProvider {
    /**
     * providerManager
     */
    @Autowired
    protected ProviderManager providerManager;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private AgentUnifiedService agentUnifiedService;

    @Autowired
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @Autowired
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @Autowired
    private DataBaseAgentSelector dataBaseAgentSelector;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * intercept
     *
     * @param backupTask 备份任务参数对象{@link BackupTask}
     * @return BackupTask
     */
    @Override
    @ExterAttack
    public BackupTask initialize(BackupTask backupTask) {
        // format
        obtainFormat(backupTask).ifPresent(backupTask::setCopyFormat);
        setCopyRestoreNeedWritable(backupTask);
        // agent
        supplyAgent(backupTask);
        // 连通性
        checkConnention(backupTask);
        // node信息
        supplyNodes(backupTask);
        // sla信息
        supplySla(backupTask);
        return supplyBackupTask(backupTask);
    }

    /**
     * 副本格式
     *
     * @param backupTask backupTask
     * @return 副本格式
     */
    protected OptionalInt obtainFormat(BackupTask backupTask) {
        return OptionalInt.empty();
    }

    /**
     * 填充agent信息
     *
     * @param backupTask backupTask
     */
    @SuppressWarnings("unchecked")
    protected void supplyAgent(BackupTask backupTask) {
        AgentSelectParam agentSelectParam = buildAgentSelectParam(backupTask);
        backupTask.setAgents(dataBaseAgentSelector.getSelectedAgents(agentSelectParam));
    }

    /**
     * 构建Agent选择实体
     *
     * @param backupTask backupTask
     * @return AgentSelectParam agentSelectParam
     */
    protected AgentSelectParam buildAgentSelectParam(BackupTask backupTask) {
        ProtectedResource protectedResource = new ProtectedResource();
        BeanUtils.copyProperties(backupTask.getProtectObject(), protectedResource);
        protectedResource.setEnvironment(BeanTools.copy(backupTask.getProtectEnv(), new ProtectedEnvironment()));
        return AgentSelectParam.builder()
            .resource(protectedResource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(backupTask.getAdvanceParams())
            .build();
    }

    /**
     * 数据库各自应用信息
     *
     * @param backupTask backupTask
     * @return BackupTask
     */
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        return backupTask;
    }

    /**
     * 检查连通性
     *
     * @param backupTask backupTask
     */
    protected void checkConnention(BackupTask backupTask) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(
            backupTask.getProtectObject().getUuid());
        if (!resOptional.isPresent()) {
            return;
        }
        ResourceConnectionCheckProvider provider = providerManager.findProviderOrDefault(
            ResourceConnectionCheckProvider.class, resOptional.get(), resourceConnectionCheckProvider);
        ResourceCheckContext resourceCheckContext = provider.checkConnection(resOptional.get());
        if (resourceCheckContext == null || resourceCheckContext.getActionResults() == null) {
            return;
        }
        for (ActionResult actionResult : resourceCheckContext.getActionResults()) {
            if (!Objects.equals(actionResult.getCode(), 0L)) {
                throw new LegoCheckedException("backup connect failed.");
            }
        }
    }

    private void supplySla(BackupTask backupTask) {
        RMap<String, String> redis = redissonClient.getMap(backupTask.getRequestId(), StringCodec.INSTANCE);
        if (redis == null) {
            return;
        }
        backupTask.addParameter(DatabaseConstants.SLA_KEY, redis.get("sla"));
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    protected void supplyNodes(BackupTask backupTask) {
        List<Endpoint> agents = backupTask.getAgents();
        if (CollectionUtils.isEmpty(agents)) {
            return;
        }
        List<TaskEnvironment> hostList = agents.stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        if (backupTask.getProtectEnv() == null) {
            return;
        }
        backupTask.getProtectEnv().setNodes(hostList);
    }

    /**
     * 恢复时，副本是否需要可写，除 DWS 之外，所有数据库应用都设置为 True
     *
     * @param backupTask 备份任务
     */
    protected void setCopyRestoreNeedWritable(BackupTask backupTask) {
        if (deployTypeService.isPacific()) {
            Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams())
                .orElseGet(Maps::newHashMap);
            advanceParams.put(DatabaseConstants.IS_COPY_RESTORE_NEED_WRITABLE, Boolean.TRUE.toString());
            backupTask.setAdvanceParams(advanceParams);
        }
    }
}