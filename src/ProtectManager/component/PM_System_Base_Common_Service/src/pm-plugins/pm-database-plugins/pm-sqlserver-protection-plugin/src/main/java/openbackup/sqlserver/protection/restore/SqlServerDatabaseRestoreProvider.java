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
package openbackup.sqlserver.protection.restore;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * SQL Server数据库恢复
 *
 */
@Slf4j
@Component
public class SqlServerDatabaseRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    /**
     * 数据库重命名情况下不支持的数据库名称类型
     */
    private static final List<String> DATABASE_NAME_NOT_MATCH = Arrays.asList(SqlServerConstants.MASTER_DATABASE,
        SqlServerConstants.MODEL_DATABASE, SqlServerConstants.MSDB_DATABASE, SqlServerConstants.TEMPDB_DATABASE);

    private final SqlServerBaseService sqlServerBaseService;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    /**
     * sqlserver应用基本的Service有参构造方法
     *
     * @param sqlServerBaseService 资源服务接口
     * @param copyRestApi 副本rest api接口
     * @param resourceService resourceService
     */
    public SqlServerDatabaseRestoreProvider(SqlServerBaseService sqlServerBaseService, CopyRestApi copyRestApi,
        ResourceService resourceService) {
        this.sqlServerBaseService = sqlServerBaseService;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check sqlserver database restore task. taskId: {}", task.getTaskId());
        // 恢复任务参数校验
        sqlServerBaseService.checkRestoreTaskParam(task);
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);

        RestoreLocationEnum targetLocation = task.getTargetLocation();
        // 获取目标对象实例信息
        ProtectedResource instanceRes = getInstanceInfo(targetLocation, task);

        // 设置部署类型，数据库无论是单实例下的还是集群实例下的，只恢复数据库所在的那个节点
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        // 封装实例的auth信息给targetObject，注意： 此处不能用同一个对象。
        task.getTargetObject().setAuth(BeanTools.copy(instanceRes.getAuth(), Authentication::new));

        List<TaskEnvironment> nodeList = applyTaskEnvironment(instanceRes);
        List<Endpoint> endpointList = nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
        task.setAgents(endpointList);
        task.getTargetEnv().setNodes(nodeList);

        sqlServerBaseService.logRestoreAddData(task, copyRestApi);
        log.info("[SQL Server] restore object: {}, agents size: {}, nodes size: {}, requestId: {}",
            task.getTargetObject().getUuid(), task.getAgents().size(), task.getTargetEnv().getNodes().size(),
            task.getRequestId());
        return task;
    }

    private ProtectedResource getInstanceInfo(RestoreLocationEnum targetLocation, RestoreTask task) {
        ProtectedResource instanceRes;
        if (RestoreLocationEnum.ORIGINAL.equals(targetLocation)) {
            // 原位置恢复，父资源是实例（包括集群实例）
            instanceRes = sqlServerBaseService.getResourceByUuid(task.getTargetObject().getParentUuid());
            String newDatabaseName = task.getAdvanceParams().get(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_NAME);
            handleNewDatabaseName(newDatabaseName, task);
        } else {
            // 新位置恢复，父资源是环境，当前资源是实例
            instanceRes = sqlServerBaseService.getResourceByUuid(task.getTargetObject().getUuid());
            task.getTargetObject().setSubType(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        }
        log.info("[SQL Server] target object name: {}, location: {}, request id: {}.", task.getTargetObject().getName(),
            task.getTargetObject().getTargetLocation(), task.getRequestId());
        return instanceRes;
    }

    private void handleNewDatabaseName(String newDatabaseName, RestoreTask task) {
        TaskResource targetObject = task.getTargetObject();
        // 填充数据库中查到的资源名称路径信息到targetObject，覆盖
        resourceService.getResourceById(task.getTargetObject().getUuid()).ifPresent(database -> {
            targetObject.setName(database.getName());
            targetObject.setTargetLocation(database.getPath());
            targetObject.setPath(database.getPath());
        });
        if (StringUtils.isNotBlank(newDatabaseName)) {
            // 数据库重命名校验基本正则规则和黑名单
            checkNewDatabaseNameMatch(newDatabaseName);
            // 校验数据库是否重名
            checkDatabaseNameExist(newDatabaseName, task);
            // 填充重命名的数据库名称路径信息到targetObject，需要覆盖
            String newPath = newDatabaseName + targetObject.getPath().substring(targetObject.getName().length());
            targetObject.setTargetLocation(newPath);
            targetObject.setPath(newPath);
        }
    }

    private void checkNewDatabaseNameMatch(String newDatabaseName) {
        if (!Pattern.compile(SqlServerConstants.DATABASE_NAME_REGEX).matcher(newDatabaseName).matches()
            || DATABASE_NAME_NOT_MATCH.contains(newDatabaseName.toLowerCase(Locale.ENGLISH))) {
            log.error("Restore new database name is illegal, new name: {}", newDatabaseName);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Restore new database name is illegal.");
        }
    }

    /**
     * 校验数据库是否重名
     *
     * @param newDatabaseName 重命名的数据库名称
     * @param task 恢复任务信息
     */
    private void checkDatabaseNameExist(String newDatabaseName, RestoreTask task) {
        List<ProtectedResource> relatedResources = sqlServerBaseService.queryDatabasesInAlwaysOnOrInstance(
            task.getTargetObject().getParentUuid()).getRecords();
        if (VerifyUtil.isEmpty(relatedResources)) {
            return;
        }
        long count = relatedResources.stream()
            .filter(Objects::nonNull)
            .filter(protectedResource -> newDatabaseName.toUpperCase(Locale.ENGLISH)
                .equals(protectedResource.getName().toUpperCase(Locale.ENGLISH)))
            .count();
        if (count != 0) {
            log.error("Restore new database name already exists, new name: {}, request id: {}", newDatabaseName,
                task.getRequestId());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Restore new database name already exists.");
        }
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        // 数据库恢复到新位置，targetObj是新单实例-->锁：新位置的实例资源
        Set<String> relatedLockResources = new HashSet<>(Collections.singleton(task.getTargetObject().getUuid()));
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            // 数据库恢复到原位置，targetObj是原数据库-->锁：原位置的数据库资源，原位置的实例资源
            relatedLockResources.add(task.getTargetObject().getParentUuid());
        }
        log.info("[SQL Server] database get lock resources: {}, size: {}, requestId: {}",
            JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(), task.getRequestId());
        return sqlServerBaseService.buildLockResourceList(relatedLockResources);
    }

    private List<TaskEnvironment> applyTaskEnvironment(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(childNode -> {
                childNode.getExtendInfo().put(DatabaseConstants.INSTANCE, protectedResource.getName());
                childNode.setAuth(protectedResource.getAuth());
                return BeanTools.copy(childNode, TaskEnvironment::new);
            })
            .collect(Collectors.toList());
    }

    /**
     * 过滤数据库备份类
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType().equals(object);
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
