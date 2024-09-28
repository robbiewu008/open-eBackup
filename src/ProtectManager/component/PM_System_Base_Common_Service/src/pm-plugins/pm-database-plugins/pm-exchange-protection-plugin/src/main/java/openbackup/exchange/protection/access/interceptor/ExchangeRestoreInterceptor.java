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
package openbackup.exchange.protection.access.interceptor;

import static java.util.stream.Collectors.toList;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Exchange恢复拦截器
 *
 */
@Slf4j
@Component
public class ExchangeRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final ExchangeService exchangeService;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    /**
     * 构造器
     *
     * @param exchangeService exchangeService
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public ExchangeRestoreInterceptor(ExchangeService exchangeService, CopyRestApi copyRestApi,
                                      ResourceService resourceService) {
        this.exchangeService = exchangeService;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.EXCHANGE_DATABASE.getType().equals(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("begin exchange database restoreTask intercept");
        ProtectedEnvironment cluster = exchangeService.getEnvironmentById(task.getTargetEnv().getUuid());
        List<Endpoint> clusterAgent = getSupplyAgent(cluster.getDependencies());
        task.setAgents(clusterAgent);
        ArrayList<TaskEnvironment> taskEnvironments = new ArrayList<>();

        // 设置nodes信息
        clusterAgent.stream().map(Endpoint::getId).forEach(it -> {
            taskEnvironments.add(getNode(it));
        });

        // 设置部署类型
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        task.getTargetEnv().setNodes(taskEnvironments);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.APPLICATION);

        // 设置恢复副本类型
        buildRestoreMode(task);
        log.info("end goldenDB restoreTask intercept");
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        List<LockResourceBo> lockResourceBos = new ArrayList<>();
        // 锁db
        lockResourceBos.add(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
        // 锁其下所有mailbox
        resourceService.getResourceById(task.getTargetObject().getUuid()).ifPresent(dbResource -> {
            ResourceQueryParams params = new ResourceQueryParams();
            Map<String, Object> filter = new HashMap<>();
            filter.put(ExchangeConstant.ROOT_UUID, task.getTargetObject().getRootUuid());
            filter.put(ExchangeConstant.SUB_TYPE, ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType());
            params.setConditions(filter);
            params.setSize(ExchangeConstant.PAGE_SIZE);
            while (true) {
                PageListResponse<ProtectedResource> query = resourceService.query(params);
                query.getRecords().stream()
                        .filter(protectedResource -> protectedResource.getExtendInfo()
                                .get(ExchangeConstant.EXT_DATABASE_NAME).equals(dbResource.getName()))
                        .forEach(protectedResource ->
                                lockResourceBos.add(new LockResourceBo(protectedResource.getUuid(), LockType.WRITE)));
                if (query.getRecords().size() < params.getSize()) {
                    break;
                }
                params.setPage(params.getPage() + 1);
            }
        });

        return lockResourceBos;
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
        log.info("build Exchange copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    /**
     * 构造恢复任务中的资源对应的环境信息
     *
     * @param agentUuid agentId
     * @return taskEnv 恢复任务中的资源对应的环境信息
     */
    public TaskEnvironment getNode(String agentUuid) {
        ProtectedEnvironment agent = exchangeService.getEnvironmentById(agentUuid);
        TaskEnvironment taskEnv = new TaskEnvironment();
        taskEnv.setName(agent.getName());
        taskEnv.setUuid(agent.getUuid());
        taskEnv.setType(agent.getType());
        taskEnv.setSubType(agent.getSubType());
        taskEnv.setEndpoint(agent.getEndpoint());
        taskEnv.setExtendInfo(agent.getExtendInfo());
        return taskEnv;
    }

    private List<Endpoint> getSupplyAgent(Map<String, List<ProtectedResource>> dependencies) {
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::applyAgentEndpoint)
            .collect(toList());
    }

    private Endpoint applyAgentEndpoint(ProtectedEnvironment agentProtectedEnv) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentProtectedEnv.getEndpoint());
        endpoint.setPort(agentProtectedEnv.getPort());
        endpoint.setId(agentProtectedEnv.getUuid());
        endpoint.setAgentOS("windows");
        return endpoint;
    }
}
