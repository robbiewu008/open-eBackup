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
package openbackup.oracle.livemount;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountScript;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.database.base.plugin.utils.StorageRepositoryUtil;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Oracle即时挂载provider
 *
 */
@Slf4j
@Component
public class OracleLiveMountInterceptorProvider implements LiveMountInterceptorProvider {
    private final AgentUnifiedService agentUnifiedService;
    private final OracleBaseService oracleBaseService;

    /**
     * 构造方法
     *
     * @param agentUnifiedService agentUnifiedService
     * @param oracleBaseService         baseService
     */
    public OracleLiveMountInterceptorProvider(AgentUnifiedService agentUnifiedService,
        OracleBaseService oracleBaseService) {
        this.agentUnifiedService = agentUnifiedService;
        this.oracleBaseService = oracleBaseService;
    }

    /**
     * 是否刷新资源
     *
     * @return 要或者不
     */
    @Override
    public boolean isRefreshTargetEnvironment() {
        return false;
    }

    /**
     * 即时挂载的检验+参数处理
     *
     * @param task LiveMountCreateTask
     */
    @Override
    public void initialize(LiveMountCreateTask task) {
        log.info("Oracle init live mount create task param start. task:{}", task.getTaskId());
        TaskResource targetObject = task.getTargetObject();
        ProtectedEnvironment databaseEnv = oracleBaseService.getEnvironmentById(targetObject.getParentUuid());
        fillTaskEnvExtendInfo(databaseEnv);
        task.setTargetEnv(BeanTools.copy(databaseEnv, new TaskEnvironment()));
        fillAgents(task, databaseEnv);
        // 设置node
        supplyNodes(task.getTargetEnv(), task.getAgents());
        // 设置node对应的oracle认证信息，底层需要根据节点获取到oracle认证信息
        oracleBaseService.setNodesAuth(task.getTargetEnv().getNodes(), Collections.singletonList(databaseEnv));
        // fileSystemShareInfos放入存储仓中
        JSONArray fileSystemShareInfoJsonArray = JSONArray.fromObject(
            task.getAdvanceParams().get(LiveMountConstants.FILE_SYSTEM_SHARE_INFO));
        buildTaskRepositories(JSONArray.toCollection(fileSystemShareInfoJsonArray,
            LiveMountFileSystemShareInfo.class), task);
        // 填充扩展参数
        fillAdvanceParams(task);
        log.info("Oracle live mount check success with repository.");
    }

    /**
     * 卸载的检验+参数处理
     *
     * @param task LiveMountCancelTask
     */
    @Override
    public void finalize(LiveMountCancelTask task) {
        log.info("Oracle init live mount cancel task param start. task:{}", task.getTaskId());
        // 设置环境参数
        TaskResource targetObject = task.getTargetObject();
        ProtectedEnvironment agentEnv = oracleBaseService.getEnvironmentById(targetObject.getRootUuid());
        // 设置agent
        fillTaskEnvExtendInfo(agentEnv);
        task.setTargetEnv(BeanTools.copy(agentEnv, TaskEnvironment::new));
        fillAgents(task, agentEnv);
        supplyNodes(task.getTargetEnv(), task.getAgents());
        oracleBaseService.setNodesAuth(task.getTargetEnv().getNodes(), Collections.singletonList(agentEnv));
        // 给仓库设置手动挂载标识
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(LiveMountConstants.MANUAL_MOUNT, LiveMountConstants.TRUE);
        StorageRepositoryUtil.getDataStorageRepository(task.getRepositories()).setExtendInfo(extendInfo);
        log.info("Oracle init live mount cancel task param success with repository.");
    }

    private void fillAdvanceParams(LiveMountCreateTask task) {
        Map<String, Object> advanceParams = task.getAdvanceParams();
        advanceParams.put(OracleConstants.CHANNELS, LegoNumberConstant.FOUR);
        advanceParams.put(OracleConstants.RECOVER_TARGET, OracleConstants.OTHER_HOST);
        LiveMountScript scripts = new LiveMountScript();
        scripts.setPreScript(advanceParams.getOrDefault(OracleConstants.PRE_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setPostScript(advanceParams.getOrDefault(OracleConstants.POST_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setFailPostScript(advanceParams.getOrDefault(OracleConstants.FAIL_POST_SCRIPT, StringUtils.EMPTY)
            .toString());
        task.setScripts(scripts);
    }

    private void fillTaskEnvExtendInfo(ProtectedEnvironment env) {
        if (ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.equalsSubType(env.getSubType())) {
            env.setExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        } else {
            env.setExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        }
    }

    private void fillAgents(BaseTask task, ProtectedEnvironment env) {
        List<Endpoint> endpoints;
        if (ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.equalsSubType(env.getSubType())) {
            endpoints = env.getDependencies().get(DatabaseConstants.AGENTS).stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .map(agent -> new Endpoint(agent.getUuid(), agent.getEndpoint(), agent.getPort()))
                .collect(Collectors.toList());
        } else {
            endpoints = Collections.singletonList(oracleBaseService.getAgentEndpoint(env));
        }
        task.setAgents(endpoints);
    }

    private void supplyNodes(TaskEnvironment taskEnvironment, List<Endpoint> agents) {
        List<TaskEnvironment> hostList = agents.stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        taskEnvironment.setNodes(hostList);
    }

    private void buildTaskRepositories(List<LiveMountFileSystemShareInfo> fileSystemShareInfos,
        LiveMountCreateTask task) {
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO, fileSystemShareInfos.get(0));
        extendInfo.put(LiveMountConstants.MANUAL_MOUNT, LiveMountConstants.TRUE);
        StorageRepository dataRepository = StorageRepositoryUtil.getDataStorageRepository(task.getRepositories());
        dataRepository.setExtendInfo(extendInfo);
        dataRepository.setProtocol(OracleConstants.NAS_SHARE_PROTOCOL);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.ORACLE.equalsSubType(subType)
                || ResourceSubTypeEnum.ORACLE_CLUSTER.equalsSubType(subType);
    }
}
