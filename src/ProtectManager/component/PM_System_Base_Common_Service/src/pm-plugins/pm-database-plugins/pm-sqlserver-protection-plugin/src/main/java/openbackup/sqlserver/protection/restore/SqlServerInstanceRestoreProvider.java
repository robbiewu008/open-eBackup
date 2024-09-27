/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.protection.restore;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * mysql单实例恢复任务下发provider
 *
 * @author xwx950025
 * @version [OceanProtect X8000 1.2.0]
 * @since 2022/7/11
 */
@Component
@Slf4j
public class SqlServerInstanceRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final SqlServerBaseService sqlServerBaseService;

    private final ProviderManager providerManager;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    /**
     * 构造函数
     *
     * @param sqlServerBaseService sqlServerBaseService
     * @param providerManager providerManager
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public SqlServerInstanceRestoreProvider(SqlServerBaseService sqlServerBaseService, ProviderManager providerManager,
        CopyRestApi copyRestApi, ResourceService resourceService) {
        this.sqlServerBaseService = sqlServerBaseService;
        this.providerManager = providerManager;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType().equals(subType);
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        // 恢复任务参数校验
        log.info("Pre check sqlserver instance restore task. taskId: {}", task.getTaskId());
        sqlServerBaseService.checkRestoreTaskParam(task);
        checkConnection(task);
    }

    /**
     * 拦截恢复请求，对恢复请求进行拦截
     *
     * @param task 恢复参数对象
     * @return 返回恢复任务
     */
    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);
        sqlServerBaseService.logRestoreAddData(task, copyRestApi);
        return supplyRestoreTask(task);
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        ProtectedResource resource = sqlServerBaseService.getResourceByUuid(task.getTargetObject().getUuid());
        task.getTargetObject().setAuth(resource.getAuth());

        // 设置agents
        ProtectedEnvironment agentEnv = BeanTools.copy(task.getTargetEnv(), ProtectedEnvironment::new);
        List<Endpoint> agents = Collections.singletonList(sqlServerBaseService.getAgentEndpoint(agentEnv));
        task.setAgents(agents);

        // 设置nodes
        sqlServerBaseService.supplyNodes(task);
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        // 单实例数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
        Set<String> relatedLockResources = new HashSet<>(Collections.singleton(task.getTargetObject().getUuid()));
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            // 单实例恢复到原位置，targetObj是原单实例-->锁：原位置单实例资源，实例下所有数据库
            // 单实例数据库级别恢复到原位置，targetObj是原单实例-->锁：原位置单实例资源，实例下所有数据库
            relatedLockResources.addAll(
                resourceService.queryRelatedResourceUuids(task.getTargetObject().getUuid(), new String[] {}));
        }
        log.info("[SQL Server] instance get lock resource ids: {}, size: {}, requestId: {}",
            JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(), task.getRequestId());
        return sqlServerBaseService.buildLockResourceList(relatedLockResources);
    }

    private void checkConnection(RestoreTask task) {
        ProtectedResource resource = sqlServerBaseService.getResourceByUuid(task.getTargetObject().getUuid());
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("[SQL Server] instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("[SQL Server] instance check connection failed. name: {}", resource.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
        }
        log.info("[SQL Server] instance check connection success, name: {}", resource.getName());
    }

    /**
     * 获取与此相关联的资源，用于恢复成功后下次转全量
     *
     * @param task RestoreTask
     * @return 关联资源，若包含自身，也需要返回
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        return sqlServerBaseService.findAssociatedResourcesToSetNextFull(task);
    }
}