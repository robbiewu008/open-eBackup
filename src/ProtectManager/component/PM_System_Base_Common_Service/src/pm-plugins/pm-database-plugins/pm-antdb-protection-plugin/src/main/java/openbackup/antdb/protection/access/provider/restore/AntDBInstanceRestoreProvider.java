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
package openbackup.antdb.protection.access.provider.restore;

import lombok.extern.slf4j.Slf4j;
import openbackup.antdb.protection.access.common.AntDBConstants;
import openbackup.antdb.protection.access.common.AntDBErrorCode;
import openbackup.antdb.protection.access.service.AntDBInstanceService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * AntDB实例恢复任务下发provider
 *
 */
@Slf4j
@Component
public class AntDBInstanceRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final AntDBInstanceService antDBInstanceService;

    private final ResourceService resourceService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param antDBInstanceService antdb实例业务类
     * @param protectedEnvironmentService protected environment service
     * @param copyRestApi copy REST API
     * @param resourceService 资源服务
     */
    public AntDBInstanceRestoreProvider(AntDBInstanceService antDBInstanceService,
        ProtectedEnvironmentService protectedEnvironmentService, CopyRestApi copyRestApi,
        ResourceService resourceService) {
        this.antDBInstanceService = antDBInstanceService;
        this.protectedEnvironmentService = protectedEnvironmentService;
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
        return Arrays.asList(ResourceSubTypeEnum.ANT_DB_INSTANCE.getType(),
            ResourceSubTypeEnum.ANT_DB_CLUSTER_INSTANCE.getType()).contains(subType);
    }

    /**
     * 设置恢复资源锁
     *
     * @param task 恢复任务信息
     * @return 锁定资源列表
     */
    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        List<LockResourceBo> lockResourceList = new ArrayList<>();
        lockResourceList.add(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
        return lockResourceList;
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 检查是否支持恢复到目标实例
        checkSupportRestore(task);
        // 设置恢复模式
        setRestoreTaskRestoreMode(task);
        // 设置高级参数：targetLocation等
        setRestoreTaskAdvancedParams(task);
        // 设置agents参数
        setRestoreTaskAgents(task);
        // 设置目标环境nodes参数
        setRestoreTaskTargetEnvNodes(task);
        // 设置目标环境扩展参数deployType
        setRestoreTaskTargetEnvExtendInfo(task);
        // 设置目标实例扩展参数deployType, version, osUsername参数
        setRestoreTaskTargetObjectExtendInfo(task);
        return task;
    }

    /**
     * 检查源实例和目标实例版本是否匹配
     *
     * @param sourceVersion 副本源实例版本
     * @param targetVersion 目标实例版本
     * @return boolean true：匹配；false：不匹配
     */
    private static boolean isVersionMatched(String sourceVersion, String targetVersion) {
        if (VerifyUtil.isEmpty(sourceVersion) || VerifyUtil.isEmpty(targetVersion)) {
            return false;
        }
        String[] srcVersions = sourceVersion.split("\\.");
        String[] tgtVersions = targetVersion.split("\\.");
        if (srcVersions.length < IsmNumberConstant.TWO || tgtVersions.length < IsmNumberConstant.TWO) {
            log.error("Illegal version exist in the source instance version and target instance version, "
                + "source version: {}, target version: {}.", sourceVersion, targetVersion);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "Illegal version exist in the source instance version and target instance version");
        }
        return StringUtils.equals(sourceVersion.trim(), targetVersion.trim());
    }

    /**
     * 检查是否支持恢复
     *
     * @param task 恢复任务
     */
    private void checkSupportRestore(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        String srcVersion = resourceJson.getString(DatabaseConstants.VERSION);
        ProtectedResource protectedResource = antDBInstanceService.getResourceById(task.getTargetObject().getUuid());
        String tgtVersion = protectedResource.getVersion();
        // 检查版本是否匹配
        if (!isVersionMatched(srcVersion, tgtVersion)) {
            log.error("The copy source instance version and the target instance version do not match,"
                + " source version: {}, target version: {}.", srcVersion, tgtVersion);
            throw new LegoCheckedException(AntDBErrorCode.VERSION_NOT_MATCH_BEFORE_RESTORE,
                "AntDB instance version do not match.");
        }

        Map<String, String> extendInfo = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO).toMap(String.class);
        String srcOsUser = extendInfo.get(AntDBConstants.DB_OS_USER_KEY);
        String tgtOsUser = protectedResource.getExtendInfoByKey(AntDBConstants.DB_OS_USER_KEY);
        // 不支持异用户恢复
        if (!StringUtils.equals(srcOsUser, tgtOsUser)) {
            log.error(
                "AntDB instance do not support restore to different user, source os user: {}," + " target os user: {}.",
                srcOsUser, tgtOsUser);
            throw new LegoCheckedException(AntDBErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE,
                "AntDB instance do not support restore to different user.");
        }
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }

        Map<String, String> advancedParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advancedParams.put(AntDBConstants.RESTORE_MODE_KEY, task.getRestoreMode());
        task.setAdvanceParams(advancedParams);
        log.info("Antdb restore target object uuid: {}, copy id: {}, mode: {}.", task.getTargetObject().getUuid(),
            task.getCopyId(), task.getRestoreMode());
    }

    /**
     * 添加恢复高级参数
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskAdvancedParams(RestoreTask task) {
        Map<String, String> advancedParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        // 添加恢复目标类型（targetLocation）参数 注：普通恢复框架默认不会带targetLocation参数到dme
        advancedParams.put(AntDBConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        advancedParams.put(AntDBConstants.RESTORE_TYPE_KEY, task.getRestoreType());

        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        ResourceEntity resource = JSONObject.fromObject(copy.getResourceProperties()).toBean(ResourceEntity.class);
        // 添加副本的数据库版本号
        advancedParams.put(AntDBConstants.COPY_PROTECT_OBJECT_VERSION_KEY, resource.getVersion());
        // 后置任务所有节点都执行
        advancedParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        task.setAdvanceParams(advancedParams);
    }

    /**
     * 实例恢复，设置Agents参数
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskAgents(RestoreTask task) {
        List<Endpoint> agents = new ArrayList<>();
        if (ResourceSubTypeEnum.ANT_DB_INSTANCE.equalsSubType(task.getTargetObject().getSubType())) {
            ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
                task.getTargetEnv().getUuid());
            agents.add(new Endpoint(environment.getUuid(), environment.getEndpoint(), environment.getPort()));
        } else {
            ProtectedResource targetResource = resourceService.getResourceById(task.getTargetObject().getUuid())
                .orElseThrow(
                    () -> new LegoCheckedException(AntDBErrorCode.SERVICE_IP_IS_INVALID, "Virtual IP is error."));
            List<ProtectedResource> protectedResources = targetResource.getDependencies()
                .get(DatabaseConstants.CHILDREN);
            for (ProtectedResource protectedResource : protectedResources) {
                agents.addAll(protectedResource.getDependencies()
                    .get(DatabaseConstants.AGENTS)
                    .stream()
                    .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                    .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
                    .collect(Collectors.toList()));
            }
        }
        task.setAgents(agents);
    }

    private TaskEnvironment toTaskEnvironment(ProtectedResource protectedResource) {
        return JsonUtil.read(JsonUtil.json(protectedResource), TaskEnvironment.class);
    }

    private List<TaskEnvironment> getClusterNodes(RestoreTask task) {
        ProtectedResource targetResource = resourceService.getResourceById(task.getTargetObject().getUuid())
            .orElseThrow(() -> new LegoCheckedException(AntDBErrorCode.SERVICE_IP_IS_INVALID, "Virtual IP is error."));
        List<ProtectedResource> protectedResources = targetResource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<ProtectedEnvironment> environments = new ArrayList<>();
        for (ProtectedResource protectedResource : protectedResources) {
            environments.addAll(protectedResource.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .collect(Collectors.toList()));
        }

        ProtectedResource clusterInstResource = antDBInstanceService.getResourceById(task.getTargetObject().getUuid());
        List<TaskResource> subTaskResources = clusterInstResource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(childNode -> BeanTools.copy(childNode, TaskResource::new))
            .collect(Collectors.toList());
        Map<String, String> hostIdRoleMap = subTaskResources.stream()
            .collect(Collectors.toMap(subInstance -> subInstance.getExtendInfo().get(DatabaseConstants.HOST_ID),
                subInstance -> subInstance.getExtendInfo().get(DatabaseConstants.ROLE)));
        for (ProtectedEnvironment env : environments) {
            env.getExtendInfo().put(DatabaseConstants.ROLE, hostIdRoleMap.get(env.getUuid()));
        }
        return environments.stream().map(this::toTaskEnvironment).collect(Collectors.toList());
    }

    /**
     * 实例恢复，设置目标环境nodes参数
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskTargetEnvNodes(RestoreTask task) {
        List<TaskEnvironment> nodes = new ArrayList<>();
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
            task.getTargetEnv().getUuid());
        if (ResourceSubTypeEnum.ANT_DB_INSTANCE.equalsSubType(task.getTargetObject().getSubType())) {
            nodes.add(toTaskEnvironment(environment));
        } else {
            nodes = getClusterNodes(task);
        }
        task.getTargetEnv().setNodes(nodes);
    }

    /**
     * 获取目标实例部署类型
     *
     * @param subType 恢复资源类型
     * @return 部署类型
     */
    private String getTargetObjectDeployType(String subType) {
        if (ResourceSubTypeEnum.ANT_DB_INSTANCE.equalsSubType(subType)) {
            return DatabaseDeployTypeEnum.SINGLE.getType();
        } else {
            return DatabaseDeployTypeEnum.AP.getType();
        }
    }

    /**
     * 实例恢复，设置目标环境扩展参数
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskTargetEnvExtendInfo(RestoreTask task) {
        task.getTargetEnv()
            .getExtendInfo()
            .put(DatabaseConstants.DEPLOY_TYPE, getTargetObjectDeployType(task.getTargetObject().getSubType()));
    }

    /**
     * 实例恢复，设置目标实例扩展参数
     *
     * @param task 恢复任务
     */
    private void setRestoreTaskTargetObjectExtendInfo(RestoreTask task) {
        String deployType = getTargetObjectDeployType(task.getTargetObject().getSubType());
        Map<String, String> targetObjectExtendInfo = task.getTargetObject().getExtendInfo();
        targetObjectExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        ProtectedResource resource = antDBInstanceService.getResourceById(task.getTargetObject().getUuid());
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, resource.getVersion());
        targetObjectExtendInfo.put(AntDBConstants.DB_OS_USER_KEY,
            resource.getExtendInfoByKey(AntDBConstants.DB_OS_USER_KEY));
        if (DatabaseDeployTypeEnum.SINGLE.getType().equals(deployType)) {
            targetObjectExtendInfo.put(AntDBConstants.DB_INSTALL_PATH_KEY,
                resource.getExtendInfoByKey(AntDBConstants.DB_INSTALL_PATH_KEY));
            targetObjectExtendInfo.put(DatabaseConstants.DATA_DIRECTORY,
                resource.getExtendInfoByKey(DatabaseConstants.DATA_DIRECTORY));
        } else {
            task.setSubObjects(buildTaskResourceBySubObjects(resource));
        }
    }

    /**
     * 添加目标集群实例的子实例信息
     *
     * @param clusterInstResource 集群实例资源信息
     * @return 子实例信息列表
     */
    private List<TaskResource> buildTaskResourceBySubObjects(ProtectedResource clusterInstResource) {
        List<TaskResource> subTaskResources = clusterInstResource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(childNode -> BeanTools.copy(childNode, TaskResource::new))
            .collect(Collectors.toList());
        for (TaskResource subResource : subTaskResources) {
            if (subResource.getExtendInfo().containsKey(DatabaseConstants.HOST_ID)) {
                ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
                    subResource.getExtendInfo().get(DatabaseConstants.HOST_ID));
                subResource.getExtendInfo().put(DatabaseConstants.END_POINT, environment.getEndpoint());
            }
        }
        return subTaskResources;
    }
}