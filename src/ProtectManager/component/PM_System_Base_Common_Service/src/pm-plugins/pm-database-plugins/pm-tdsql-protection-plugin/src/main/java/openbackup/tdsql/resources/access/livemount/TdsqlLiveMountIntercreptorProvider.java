/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.livemount;

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
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.service.TdsqlService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 功能描述 Tdsql即时挂载provider
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-01
 */
@Slf4j
@Component
public class TdsqlLiveMountIntercreptorProvider implements LiveMountInterceptorProvider {
    private final int nasShareProtocol = 1;
    private final AgentUnifiedService agentUnifiedService;
    private final TdsqlService tdsqlService;
    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param agentUnifiedService agentUnifiedService
     * @param tdsqlService tdsqlService
     * @param copyRestApi copyRestApi
     */
    public TdsqlLiveMountIntercreptorProvider(AgentUnifiedService agentUnifiedService, TdsqlService tdsqlService,
        CopyRestApi copyRestApi) {
        this.agentUnifiedService = agentUnifiedService;
        this.tdsqlService = tdsqlService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType().equals(subType);
    }

    /**
     * 即时挂载的检验+参数处理
     *
     * @param task LiveMountCreateTask
     */
    @Override
    public void initialize(LiveMountCreateTask task) {
        log.info("start live mount create task.");
        if (!task.getTargetEnv().getSubType().isEmpty()
            && TdsqlConstant.U_BACKUP_AGENT.equals(task.getTargetEnv().getSubType())) {
            task.getTargetEnv().setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
        }
        if (!task.getTargetObject().getSubType().isEmpty()
            && TdsqlConstant.U_BACKUP_AGENT.equals(task.getTargetObject().getSubType())) {
            task.getTargetObject().setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
        }
        TaskEnvironment env = task.getTargetEnv();
        if (env.getExtendInfo() == null) {
            env.setExtendInfo(new HashMap<>());
        }
        env.getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());

        // 覆盖默认的内置代理Agent
        String envUuid = TdsqlConstant.TDSQL_CLUSTERINSTACE.equals(
            env.getSubType()) ? env.getRootUuid() : env.getUuid();
        ProtectedEnvironment protectedEnvironment = tdsqlService.getEnvironmentById(envUuid);
        if (!protectedEnvironment.getSubType().isEmpty()
            && TdsqlConstant.U_BACKUP_AGENT.equals(protectedEnvironment.getSubType())) {
            protectedEnvironment.setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
        }
        Endpoint endpoint = tdsqlService.getAgentEndpoint(protectedEnvironment);
        task.setAgents(Collections.singletonList(endpoint));

        // 设置node
        supplyNodes(task.getTargetEnv(), task.getAgents());

        // 首次执行检查时，没有存储仓信息，可以直接退出，第二次创建即时挂载时，需要进行存储仓信息构造
        if (CollectionUtils.isEmpty(task.getRepositories())) {
            log.info("TDSQL live mount init create param success.");
            return;
        }

        // fileSystemShareInfos放入存储仓中
        JSONArray fileSystemShareInfoJsonArray = JSONArray.fromObject(
            task.getAdvanceParams().get(LiveMountConstants.FILE_SYSTEM_SHARE_INFO));
        buildTaskRepositories(JSONArray.toCollection(fileSystemShareInfoJsonArray,
            LiveMountFileSystemShareInfo.class), task);
        log.info("TDSQL live mount init create param success with repository.");
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());

        // 填充扩展参数
        fillAdvanceParams(task);

        // 检验即时挂载
        checkLiveMount(task, copy);
        log.info("TDSQL live mount check success with repository.");
    }

    private void fillAdvanceParams(LiveMountCreateTask task) {
        log.info("Begin to fill TDSQL script params.");
        Map<String, Object> advanceParams = task.getAdvanceParams();
        LiveMountScript scripts = new LiveMountScript();
        scripts.setPreScript(advanceParams.getOrDefault(TdsqlConstant.PRE_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setPostScript(advanceParams.getOrDefault(TdsqlConstant.POST_SCRIPT, StringUtils.EMPTY).toString());
        scripts.setFailPostScript(advanceParams.getOrDefault(TdsqlConstant.FAIL_POST_SCRIPT, StringUtils.EMPTY)
                .toString());
        task.setScripts(scripts);
        log.info("End to fill TDSQL script params.");
    }

    private void supplyNodes(TaskEnvironment taskEnvironment, List<Endpoint> agents) {
        List<TaskEnvironment> hostList = agents.stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        for (TaskEnvironment host : hostList) {
            if (!host.getSubType().isEmpty()
                && TdsqlConstant.U_BACKUP_AGENT.equals(host.getSubType())) {
                host.setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
            }
        }
        taskEnvironment.setNodes(hostList);
    }

    private void buildTaskRepositories(List<LiveMountFileSystemShareInfo> fileSystemShareInfos,
        LiveMountCreateTask task) {
        log.info("start to build task repositories.");
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO, fileSystemShareInfos.get(0));
        extendInfo.put(LiveMountConstants.MANUAL_MOUNT, LiveMountConstants.TRUE);
        StorageRepository dataRepository = StorageRepositoryUtil.getDataStorageRepository(task.getRepositories());
        dataRepository.setExtendInfo(extendInfo);
        dataRepository.setProtocol(nasShareProtocol);
    }

    /**
     * 检验即时挂载
     *
     * @param task 目标
     * @param copy 源
     */
    private void checkLiveMount(LiveMountCreateTask task, Copy copy) {
        log.info("start to check live mount.");
        TaskResource targetResource = task.getTargetObject();

        // 拦截部署模式 subType类型一致
        tdsqlService.checkSubType(copy, targetResource);
    }

    /**
     * 卸载的检验+参数处理
     *
     * @param task LiveMountCancelTask
     */
    @Override
    public void finalize(LiveMountCancelTask task) {
        log.info("start live mount cancel task.");
        if (!task.getTargetObject().getSubType().isEmpty()
            && TdsqlConstant.U_BACKUP_AGENT.equals(task.getTargetObject().getSubType())) {
            task.getTargetObject().setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
        }

        // 根据targetObject，设置TargetEnv
        TaskResource targetObject = task.getTargetObject();

        // 设置targetEnv
        String targetObjectRootUuid = TdsqlConstant.TDSQL_CLUSTERINSTACE.equals(
            targetObject.getSubType()) ? targetObject.getRootUuid() : targetObject.getUuid();
        ProtectedEnvironment agentEnv = tdsqlService.getEnvironmentById(targetObjectRootUuid);
        task.setTargetEnv(BeanTools.copy(agentEnv, TaskEnvironment::new));
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());

        // 设置agents
        ProtectedEnvironment protectedEnvironment = tdsqlService.getEnvironmentById(targetObjectRootUuid);
        if (!protectedEnvironment.getSubType().isEmpty()
            && TdsqlConstant.U_BACKUP_AGENT.equals(protectedEnvironment.getSubType())) {
            protectedEnvironment.setSubType(TdsqlConstant.TDSQL_CLUSTERINSTACE);
        }
        Endpoint endpoint = tdsqlService.getAgentEndpoint(protectedEnvironment);
        task.setAgents(Collections.singletonList(endpoint));

        // 设置node
        supplyNodes(task.getTargetEnv(), task.getAgents());

        // 给仓库设置手动挂载标识
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(LiveMountConstants.MANUAL_MOUNT, LiveMountConstants.TRUE);
        StorageRepositoryUtil.getDataStorageRepository(task.getRepositories()).setExtendInfo(extendInfo);
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
}
