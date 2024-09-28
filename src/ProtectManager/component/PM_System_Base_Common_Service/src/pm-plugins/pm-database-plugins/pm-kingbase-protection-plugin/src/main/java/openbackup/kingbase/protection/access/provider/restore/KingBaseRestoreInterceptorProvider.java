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
package openbackup.kingbase.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.kingbase.protection.access.provider.common.KingbaseErrorCode;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * KingBase恢复拦截器
 *
 */
@Slf4j
@Component
public class KingBaseRestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private static final String DB_OS_USER_KEY = "osUsername";

    private final KingBaseService kingBaseService;

    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param kingBaseService kingbase业务类
     * @param copyRestApi copy REST API
     */
    public KingBaseRestoreInterceptorProvider(KingBaseService kingBaseService, CopyRestApi copyRestApi) {
        this.kingBaseService = kingBaseService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType(),
            ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType()).contains(subType);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("Start kingbase restore interceptor set parameters. uuid: {}", task.getTaskId());
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 检查是否支持恢复到目标实例
        checkSupportRestore(task);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);
        ProtectedResource instanceResource = kingBaseService.getResourceById(task.getTargetObject().getUuid());

        // 设置agents参数
        task.setAgents(kingBaseService.getAgentsByInstanceResource(instanceResource));

        // 设置目标环境nodes参数
        task.getTargetEnv().setNodes(kingBaseService.getEnvNodesByInstanceResource(instanceResource));

        // 设置目标环境扩展参数
        buildTargetEnvExtendInfo(task);

        // 设置目标实例扩展参数
        buildTargetObjectExtendInfo(task, instanceResource);

        // 设置子资源对象
        task.setSubObjects(kingBaseService.getSubInstances(instanceResource));

        // 设置高级参数
        buildRestoreAdvanceParams(task);
        log.info("End kingbase restore interceptor set parameters. uuid: {}", task.getTaskId());
        return task;
    }

    /**
     * 检查是否支持恢复
     *
     * @param task 恢复任务
     */
    private void checkSupportRestore(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        ProtectedResource protectedResource = kingBaseService.getResourceById(task.getTargetObject().getUuid());

        Map<String, String> extendInfo = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO).toMap(String.class);

        checkOsUser(extendInfo.get(DB_OS_USER_KEY), protectedResource.getExtendInfoByKey(DB_OS_USER_KEY), task);
        checkDatabaseMode(extendInfo.get(DatabaseConstants.DB_MODE_KEY),
            protectedResource.getExtendInfoByKey(DatabaseConstants.DB_MODE_KEY));
    }

    private void checkOsUser(String srcOsUser, String tgtOsUser, RestoreTask task) {
        // 不支持异用户恢复
        if (!StringUtils.equals(srcOsUser, tgtOsUser)) {
            log.error(
                "Kingbase instance do not support restore to different user, "
                    + "source os user: {}, target os user: {},"
                    + "task id: {}, copy id: {}",
                srcOsUser, tgtOsUser, task.getTaskId(), task.getCopyId());
            throw new LegoCheckedException(KingbaseErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE,
                "Kingbase instance do not support restore to different user.");
        }
    }

    private void checkDatabaseMode(String srcDbMode, String tgtDbMode) {
        // 兼容以前没有设置数据库模式字段值的副本能正常下发参数
        if (StringUtils.isEmpty(srcDbMode)) {
            log.info("Kingbase restore copy source database mode is empty. src mode: {}, tgt mode: {}.", srcDbMode,
                tgtDbMode);
            return;
        }
        if (!StringUtils.equals(srcDbMode, tgtDbMode)) {
            log.error(
                "Kingbase target database mode is different from source database mode. src mode: {}, tgt mode: {}.",
                srcDbMode, tgtDbMode);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT,
                "Kingbase target database mode is different from source database mode.");
        }
    }

    private void buildRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElseGet(HashMap::new);
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        advanceParams.put(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY, getCopyResourceVersion(task.getCopyId()));
        task.setAdvanceParams(advanceParams);
    }

    private String getCopyResourceVersion(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        ResourceEntity resource = JSONObject.fromObject(copy.getResourceProperties()).toBean(ResourceEntity.class);
        return resource.getVersion();
    }

    private void buildTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE,
            kingBaseService.getDeployType(task.getTargetObject().getSubType()));
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void buildTargetObjectExtendInfo(RestoreTask task, ProtectedResource instanceResource) {
        Map<String, String> targetObjectExtendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
            .orElseGet(HashMap::new);
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, instanceResource.getVersion());
        task.getTargetObject().setExtendInfo(targetObjectExtendInfo);
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }
}
