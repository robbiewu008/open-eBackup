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
package openbackup.database.base.plugin.provider;

import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.common.GeneralDbErrorCode;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.EnumUtil;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalInt;

/**
 * 通用数据库备份拦截器
 *
 */
@Slf4j
@Component
public class GeneralDbBackupInterceptor extends AbstractDbBackupInterceptor {
    private static final Map<String, String> backupLabelMap;

    static {
        backupLabelMap = new HashMap<>();
        backupLabelMap.put(BackupTypeConstants.FULL.getBackupType(), BackupConstant.FULL_LABEL);
        backupLabelMap.put(BackupTypeConstants.DIFFERENCE_INCREMENT.getBackupType(), BackupConstant.INCREMENTAL_LABEL);
        backupLabelMap.put(BackupTypeConstants.CUMULATIVE_INCREMENT.getBackupType(), BackupConstant.DIFF_LABEL);
        backupLabelMap.put(BackupTypeConstants.LOG.getBackupType(), BackupConstant.LOG_LABEL);
    }

    private final GeneralDbProtectAgentService generalDbProtectAgentService;

    public GeneralDbBackupInterceptor(GeneralDbProtectAgentService generalDbProtectAgentService) {
        this.generalDbProtectAgentService = generalDbProtectAgentService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object);
    }

    @Override
    protected OptionalInt obtainFormat(BackupTask backupTask) {
        Optional<AppConf> appConf = getAppConf(backupTask);
        Optional<Integer> formatOptional = appConf.map(AppConf::getCopy).map(AppConf.Copy::getFormat);
        return formatOptional.map(OptionalInt::of).orElse(OptionalInt.empty());
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 校验conf信息
        checkAppConf(backupTask);

        // 配置信息
        setConfInfo(backupTask);

        List<StorageRepository> repositories = backupTask.getRepositories();
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        backupTask.addRepository(cacheRepository);

        String backupType = backupTask.getBackupType();
        // 如果备份方式为WAL, 添加：3-LOG_REPOSITORY
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            if (!isLogBackupSendDataRepository(backupTask)) {
                repositories.remove(0);
            }
        }

        return backupTask;
    }

    private boolean isLogBackupSendDataRepository(BackupTask backupTask) {
        String confStr = backupTask.getProtectObject().getExtendInfo().get(GeneralDbConstant.EXTEND_SCRIPT_CONF);
        Optional<AppConf> appConf = GeneralDbUtil.getAppConf(confStr);
        return appConf.map(AppConf::getBackup).map(AppConf.Backup::isLogBackupNeedDataRepository).orElse(false);
    }

    private void checkAppConf(BackupTask backupTask) {
        String confStr = backupTask.getProtectObject().getExtendInfo().get(GeneralDbConstant.EXTEND_SCRIPT_CONF);
        Optional<AppConf> appConf = GeneralDbUtil.getAppConf(confStr);
        if (!appConf.isPresent()) {
            log.warn("can not found config script. request id: {}", backupTask.getRequestId());
            return;
        }
        List<AppConf.Backup.Support> supports = appConf.map(AppConf::getBackup)
            .map(AppConf.Backup::getSupports)
            .orElse(Collections.emptyList());
        String backupType = BackupTypeConstants.dmeBackTypeConvertBack(backupTask.getBackupType());
        for (AppConf.Backup.Support support : supports) {
            String version = backupTask.getProtectObject().getVersion();
            if (backupType.equals(support.getBackupType()) && !GeneralDbUtil.checkVersion(version,
                support.getMinVersion(), support.getMaxVersion())) {
                throw new LegoCheckedException(GeneralDbErrorCode.VERSION_DO_NOT_SUPPORT_BACKUP,
                    new String[] {version, backupLabelMap.get(backupType)}, "version do not support");
            }
        }
    }

    private void setConfInfo(BackupTask backupTask) {
        Optional<AppConf> appConf = getAppConf(backupTask);
        // 统计速率
        String speedStatistics = appConf.map(AppConf::getBackup)
            .map(AppConf.Backup::getSpeedStatistics)
            .orElse(SpeedStatisticsEnum.UBC.getType());

        SpeedStatisticsEnum speedStatisticsEnum = EnumUtil.get(SpeedStatisticsEnum.class, SpeedStatisticsEnum::getType,
            speedStatistics, true, true);
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask,
            speedStatisticsEnum == null ? SpeedStatisticsEnum.UBC : speedStatisticsEnum);

        // 是否多节点执行
        boolean isMultiPostJob = appConf.map(AppConf::getBackup).map(AppConf.Backup::getIsMultiPostJob).orElse(false);
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams())
            .orElse(Maps.newHashMap());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, String.valueOf(isMultiPostJob));
        backupTask.setAdvanceParams(advanceParams);
    }

    private Optional<AppConf> getAppConf(BackupTask backupTask) {
        Map<String, String> extendInfo = Optional.ofNullable(backupTask.getProtectObject().getExtendInfo())
            .orElse(Maps.newHashMap());
        String confStr = extendInfo.get(GeneralDbConstant.EXTEND_SCRIPT_CONF);
        if (VerifyUtil.isEmpty(confStr)) {
            return Optional.empty();
        }
        return GeneralDbUtil.getAppConf(confStr);
    }

    @Override
    protected void supplyAgent(BackupTask backupTask) {
        String resourceId = backupTask.getProtectObject().getUuid();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resourceId);
        List<Endpoint> endpoints = generalDbProtectAgentService.select(protectedResource);
        backupTask.setAgents(endpoints);
    }

    @Override
    protected void checkConnention(BackupTask backupTask) {
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        if (resource == null || resource.getExtendInfo() == null) {
            return false;
        }
        return resource.getExtendInfo().getOrDefault("script", "").equals("saphana");
    }
}
