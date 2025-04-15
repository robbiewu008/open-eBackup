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
package openbackup.goldendb.protection.access.interceptor;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.goldendb.protection.access.provider.GoldenDBAgentProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述 恢复
 *
 */
@Slf4j
@Component
public class GoldenDbRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final GoldenDbService goldenDbService;

    private final CopyRestApi copyRestApi;

    private final GoldenDBAgentProvider goldenDBAgentProvider;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 构造器
     *
     * @param goldenDbService goldenDbService
     * @param copyRestApi copyRestApi
     * @param goldenDBAgentProvider goldenDBAgentProvider
     * @param agentUnifiedService agentUnifiedService
     */
    public GoldenDbRestoreInterceptor(GoldenDbService goldenDbService, CopyRestApi copyRestApi,
        GoldenDBAgentProvider goldenDBAgentProvider, AgentUnifiedService agentUnifiedService) {
        this.goldenDbService = goldenDbService;
        this.copyRestApi = copyRestApi;
        this.goldenDBAgentProvider = goldenDBAgentProvider;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("begin goldenDB restoreTask intercept");
        ProtectedResource resource = BeanTools.copy(task.getTargetObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        List<Endpoint> endpointList = goldenDBAgentProvider.getSelectedAgents(agentSelectParam);

        task.setAgents(endpointList);
        ArrayList<TaskEnvironment> taskEnvironments = new ArrayList<>();
        // 设置nodes信息
        for (Endpoint endpoint : endpointList) {
            try {
                agentUnifiedService.getHost(endpoint.getIp(), endpoint.getPort());
                taskEnvironments.add(getNode(endpoint.getId()));
            } catch (LegoCheckedException e) {
                log.warn("job id: {}, agent: {}, is offline", task.getTaskId(), endpoint.getIp());
            }
        }

        // 后置任务所有节点都执行
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        // 恢复时，副本是否需要可写，除 DWS 之外，所有数据库应用都设置为 True
        advanceParams.put(DatabaseConstants.IS_COPY_RESTORE_NEED_WRITABLE, Boolean.TRUE.toString());
        task.setAdvanceParams(advanceParams);

        // 设置部署类型
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        task.getTargetEnv().setNodes(taskEnvironments);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复副本类型
        buildRestoreMode(task);
        log.info("end goldenDB restoreTask intercept");
        return task;
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void buildRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value())
            .contains(copy.getGeneratedBy())) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build GoldenDB copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private TaskEnvironment getNode(String agentUuid) {
        ProtectedEnvironment agent = goldenDbService.getEnvironmentById(agentUuid);
        TaskEnvironment taskEnv = new TaskEnvironment();
        taskEnv.setName(agent.getName());
        taskEnv.setUuid(agent.getUuid());
        taskEnv.setType(agent.getType());
        taskEnv.setSubType(agent.getSubType());
        taskEnv.setEndpoint(agent.getEndpoint());
        taskEnv.setExtendInfo(agent.getExtendInfo());
        return taskEnv;
    }
}
