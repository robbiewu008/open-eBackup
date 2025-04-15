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
package openbackup.eccoracle.restore;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.eccoracle.service.EccOracleBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * 恢复任务下发provider
 *
 */
@Slf4j
@Component
public class EccOracleSingleRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final EccOracleBaseService oracleBaseService;

    private final CopyRestApi copyRestApi;

    public EccOracleSingleRestoreProvider(EccOracleBaseService oracleBaseService, CopyRestApi copyRestApi) {
        this.oracleBaseService = oracleBaseService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.equalsSubType(resourceSubType);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("Start SAP_ON_ORACLE_SINGLE restore interceptor set parameters. Task id: {}.", task.getTaskId());
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);

        // 获取目标资源信息
        ProtectedResource resource = oracleBaseService.getResourceById(task.getTargetObject().getUuid());

        // 设置agents
        task.setAgents(oracleBaseService.getAgents(resource));

        // 设置环境nodes
        task.getTargetEnv().setNodes(oracleBaseService.getEnvNodes(resource));

        fillDeployType(task);
        fillAdvanceParams(task);
        log.info("End SAP_ON_ORACLE_SINGLE restore interceptor set parameters. Task id: {}.", task.getTaskId());
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }

    /**
     * 根据备份任务类型的不同，更新仓库
     * 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     *
     * @param task 通用备份框架备份参数对象
     */
    private void updateRepositories(RestoreTask task) {
        List<StorageRepository> repositories = task.getRepositories();
        String envUuid = task.getTargetEnv().getUuid();
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envUuid);
        // 适配Windows系统，下发到ubc的仓库协议需要设置为CIFS
        oracleBaseService.repositoryAdaptsWindows(repositories, environment);
        task.setRepositories(repositories);
    }

    private void fillDeployType(RestoreTask task) {
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private void fillAdvanceParams(RestoreTask task) {
        task.getAdvanceParams().put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
    }
}
