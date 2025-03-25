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
package openbackup.saphana.protection.access.provider.backup;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.constant.SapHanaErrorCode;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * SAP HANA数据库备份Provider
 *
 */
@Component
@Slf4j
public class SapHanaDatabaseBackupProvider extends AbstractDbBackupInterceptor {
    private final SapHanaResourceService hanaResourceService;

    public SapHanaDatabaseBackupProvider(SapHanaResourceService hanaResourceService) {
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.equalsSubType(resourceSubType);
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        String dbResourceId = backupTask.getProtectObject().getUuid();
        String taskId = backupTask.getTaskId();
        log.info("Start sap hana database backup intercept to set parameters, task id: {}, resource id: {}.", taskId,
            dbResourceId);
        ProtectedResource dbResource = hanaResourceService.getResourceById(dbResourceId);
        // 设置存储仓
        backupTask.setRepositories(setRepositories(backupTask, dbResource));
        List<ProtectedResource> savedHostResourceList = SapHanaUtil.parseDbHostProtectedResourceList(dbResource);
        List<ProtectedEnvironment> currHostEnvList = hanaResourceService.queryEnvironments(savedHostResourceList);
        // 设置nodes
        backupTask.getProtectEnv().setNodes(SapHanaUtil.convertEnvListToTaskEnvList(currHostEnvList));
        List<Endpoint> agentList = SapHanaUtil.convertEnvListToEndpointList(currHostEnvList);
        // 设置agents
        backupTask.setAgents(agentList);
        // 设置保护环境扩展参数deployType
        setProtectEnvExtendInfo(backupTask, agentList);
        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.APPLICATION);
        log.info("End sap hana database backup interceptor setting parameters, task id: {}", taskId);
        return backupTask;
    }

    private void setProtectEnvExtendInfo(BackupTask backupTask, List<Endpoint> agentList) {
        String deployType = DatabaseDeployTypeEnum.SINGLE.getType();
        if (agentList.size() > IsmNumberConstant.ONE) {
            Map<String, String> dbExtendInfo = Optional.ofNullable(backupTask.getProtectObject().getExtendInfo())
                .orElseGet(HashMap::new);
            String hanaDbType = dbExtendInfo.get(SapHanaConstants.SAP_HANA_DB_TYPE);
            if (SapHanaConstants.SYSTEM_DB_TYPE.equals(hanaDbType)) {
                // 系统数据库集群是主备
                deployType = DatabaseDeployTypeEnum.AP.getType();
            } else {
                // 租户数据库集群是分布式
                deployType = DatabaseDeployTypeEnum.DISTRIBUTED.getType();
            }
        }
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        backupTask.getProtectEnv().setExtendInfo(envExtendInfo);
    }

    private List<StorageRepository> setRepositories(BackupTask backupTask, ProtectedResource dbResource) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        ProtectedResource instResource = hanaResourceService.getResourceById(dbResource.getParentUuid());
        // 如果实例开启了日志备份
        String enableLogBakStr = instResource.getExtendInfoByKey(SapHanaConstants.ENABLE_LOG_BACKUP);
        if ("true".equals(enableLogBakStr)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            Map<String, Object> repoExtendInfo = Optional.ofNullable(logRepository.getExtendInfo())
                .orElseGet(HashMap::new);
            repoExtendInfo.put(SapHanaConstants.PERSISTENT_MOUNT, true);
            logRepository.setExtendInfo(repoExtendInfo);
            repositories.add(logRepository);
        } else {
            if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
                log.error("The sap hana instance does not enable log backup, can not execute log backup"
                    + "database uuid: {}, instance uuid: {}.", dbResource.getUuid(), instResource.getUuid());
                throw new LegoCheckedException(SapHanaErrorCode.INSTANCE_NOT_ENABLE_LOG_BACKUP,
                    "The sap hana instance does not enable log backup.");
            }
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
            StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        return repositories;
    }
}
