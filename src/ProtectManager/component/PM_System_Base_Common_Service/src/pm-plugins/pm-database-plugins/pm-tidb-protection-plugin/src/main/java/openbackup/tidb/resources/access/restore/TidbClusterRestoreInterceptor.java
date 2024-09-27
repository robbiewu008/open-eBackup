/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.restore;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 集群恢复
 *
 * @author w00426202
 * @since 2023-07-21
 */
@Slf4j
@Component
public class TidbClusterRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    final TidbService tidbService;

    final TidbAgentProvider tidbAgentProvider;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    private final DefaultProtectAgentSelector defaultSelector;

    private final List<String> copyGeneratedByEnumList = Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(),
        CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());

    /**
     * 构造器
     *
     * @param tidbService tidbService
     * @param tidbAgentProvider tidbAgentProvider
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param defaultSelector defaultSelector
     */
    public TidbClusterRestoreInterceptor(TidbService tidbService, TidbAgentProvider tidbAgentProvider,
        CopyRestApi copyRestApi, ResourceService resourceService, DefaultProtectAgentSelector defaultSelector) {
        this.tidbService = tidbService;
        this.tidbAgentProvider = tidbAgentProvider;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
        this.defaultSelector = defaultSelector;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_CLUSTER.getType().equals(object);
    }

    /**
     * 添加资源锁，同一节点恢复过程中不能进行备份操作
     *
     * @param task task
     * @return result
     */
    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        setRestoreTaskEndpoint(task);
        wrapTaskAfterInfo(task);
        TidbUtil.setTiupUuid(task.getTargetObject().getExtendInfo(), task.getTargetObject().getRootUuid(),
            resourceService, defaultSelector, tidbService);
        return task;
    }

    /**
     * 设置恢复任务endPoint信息
     *
     * @param task task
     */
    public void setRestoreTaskEndpoint(RestoreTask task) {
        ProtectedResource resource = BeanTools.copy(task.getTargetObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        List<Endpoint> endpointList = tidbAgentProvider.getSelectedAgents(agentSelectParam);

        task.setAgents(endpointList);
        List<TaskEnvironment> nodes = endpointList.stream()
            .map(endpoint -> convertToTaskEnvironment(endpoint))
            .collect(Collectors.toList());
        task.getTargetEnv().setNodes(nodes);
        log.info("tidb cluster restore begin.");
    }

    private TaskEnvironment convertToTaskEnvironment(Endpoint endpoint) {
        ProtectedEnvironment protectedEnvironment = resourceService.getResourceById(endpoint.getId())
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected resource is not exists!"));
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        return taskEnvironment;
    }

    /**
     * 设置恢复任务后置条件信息
     *
     * @param task task
     */
    public void wrapTaskAfterInfo(RestoreTask task) {
        // 如果是任意时间带点恢复，需要下发时间戳参数
        buildRestoreRepositories(task);

        // 设置部署类型
        task.getTargetEnv()
            .getExtendInfo()
            .put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复副本类型
        buildRestoreMode(task);
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void buildRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (copyGeneratedByEnumList.contains(copy.getGeneratedBy())) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build TDSQL copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    /**
     * 任意时间点恢复，下发时间戳
     *
     * @param task 恢复任务
     */
    private void buildRestoreRepositories(RestoreTask task) {
        log.info("TDSQL start to build restore repositories.");
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        String restoreTimestamp = advanceParams.get(TidbConstants.RESTORE_TIME_STAMP_KEY);
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        int backupType = copy.getBackupType();
        if (BackupTypeConstants.LOG.getAbBackupType() == backupType && VerifyUtil.isEmpty(restoreTimestamp)) {
            String timestamp = copy.getTimestamp();
            restoreTimestamp = timestamp.substring(0, timestamp.length() - 6);
            advanceParams.putIfAbsent(TidbConstants.RESTORE_TIME_STAMP_KEY, restoreTimestamp);
            task.setAdvanceParams(advanceParams);
        }
    }

    /**
     * 特性开关, 关闭验证
     *
     * @return RestoreFeature
     */
    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }
}
