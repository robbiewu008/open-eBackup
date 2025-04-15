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
package openbackup.mysql.resources.access.livemount;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
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
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * mysql即时挂载provider
 *
 */
@Slf4j
@Component
public class MysqlLiveMountInterceptorProvider implements LiveMountInterceptorProvider {
    private final int nasShareProtocol = 1;
    private final MysqlBaseService mysqlBaseService;
    private final AgentUnifiedService agentUnifiedService;
    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param mysqlBaseService mysqlBaseService
     * @param agentUnifiedService agentUnifiedService
     * @param copyRestApi copyRestApi
     */
    public MysqlLiveMountInterceptorProvider(MysqlBaseService mysqlBaseService, AgentUnifiedService agentUnifiedService,
            CopyRestApi copyRestApi) {
        this.mysqlBaseService = mysqlBaseService;
        this.agentUnifiedService = agentUnifiedService;
        this.copyRestApi = copyRestApi;
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
        TaskEnvironment env = task.getTargetEnv();
        if (env.getExtendInfo() == null) {
            env.setExtendInfo(new HashMap<>());
        }
        env.getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());

        // 覆盖默认的内置代理Agent
        String envUuid = MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(
                env.getSubType()) ? env.getRootUuid() : env.getUuid();
        Endpoint endpoint = mysqlBaseService.getAgentEndpoint(mysqlBaseService.getEnvironmentById(envUuid));
        task.setAgents(Collections.singletonList(endpoint));

        // 设置node
        supplyNodes(task.getTargetEnv(), task.getAgents());

        // 首次执行检查时，没有存储仓信息，可以直接退出，第二次创建即时挂载时，需要进行存储仓信息构造
        if (CollectionUtils.isEmpty(task.getRepositories())) {
            log.info("mysql live mount init create param success.");
            return;
        }

        // fileSystemShareInfos放入存储仓中
        JSONArray fileSystemShareInfoJsonArray = JSONArray.fromObject(
                task.getAdvanceParams().get(LiveMountConstants.FILE_SYSTEM_SHARE_INFO));
        buildTaskRepositories(JSONArray.toCollection(fileSystemShareInfoJsonArray,
                LiveMountFileSystemShareInfo.class), task);
        log.info("mysql live mount init create param success with repository.");
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());

        // 填充扩展参数
        fillAdvanceParams(task);

        // 检验即时挂载
        checkLiveMount(task, copy);
        log.info("mysql live mount check success with repository.");
    }

    /**
     * 即时挂载不支持跨操作系统，跨MySQL部署模式，跨版本，跨MySQL以及MariaDB。
     *
     * @param task 目标
     * @param copy 源
     */
    private void checkLiveMount(LiveMountCreateTask task, Copy copy) {
        TaskResource targetResource = task.getTargetObject();
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        Map<String, String> sourceExtendInfo = Optional.ofNullable(
                resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO).toMap(String.class))
                .orElse(Collections.emptyMap());
        Map<String, String> targetExtendInfo = Optional.ofNullable(targetResource.getExtendInfo())
                .orElse(Collections.emptyMap());

        // 拦截MySQL部署的操作系统
        mysqlBaseService.checkDeployOperatingSystem(sourceExtendInfo, targetExtendInfo);

        // 拦截部署模式 subType类型一致
        mysqlBaseService.checkSubType(copy, targetResource);

        // 拦截版本,跨MySQL以及MariaDB
        mysqlBaseService.checkVersion(targetResource, resourceJson);
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
        dataRepository.setProtocol(nasShareProtocol);
    }

    /**
     * 卸载的检验+参数处理
     *
     * @param task LiveMountCancelTask
     */
    @Override
    public void finalize(LiveMountCancelTask task) {
        // 根据targetObject，设置TargetEnv
        TaskResource targetObject = task.getTargetObject();

        // 设置targetEnv
        ProtectedEnvironment agentEnv = mysqlBaseService.getEnvironmentById(targetObject.getParentUuid());
        task.setTargetEnv(BeanTools.copy(agentEnv, TaskEnvironment::new));
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());

        // 设置agents
        Endpoint endpoint = mysqlBaseService.getAgentEndpoint(
                mysqlBaseService.getEnvironmentById(targetObject.getParentUuid()));
        task.setAgents(Collections.singletonList(endpoint));

        // 设置node
        supplyNodes(task.getTargetEnv(), task.getAgents());

        // 给仓库设置手动挂载标识
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(LiveMountConstants.MANUAL_MOUNT, LiveMountConstants.TRUE);
        StorageRepositoryUtil.getDataStorageRepository(task.getRepositories()).setExtendInfo(extendInfo);
    }

    private void fillAdvanceParams(LiveMountCreateTask task) {
        Map<String, Object> advanceParams = task.getAdvanceParams();
        LiveMountScript scripts = new LiveMountScript();
        scripts.setPreScript(advanceParams.getOrDefault(MysqlConstants.PRE_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setPostScript(advanceParams.getOrDefault(MysqlConstants.POST_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setFailPostScript(advanceParams.getOrDefault(MysqlConstants.FAIL_POST_SCRIPT, StringUtils.EMPTY)
                .toString());
        task.setScripts(scripts);
        log.info("Mysql fill the script params success.");
    }

    /**
     * 即时挂载只针对单实例
     *
     * @param subType 资源子类型
     * @return 是否可应用此provider
     */
    @Override
    public boolean applicable(String subType) {
        return MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(subType);
    }
}
