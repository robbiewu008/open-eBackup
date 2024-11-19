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
package openbackup.eccoracle.backup;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.restore.service.RestoreTaskHelper;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.eccoracle.service.EccOracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * ECC ORACLE 单机备份
 *
 */
@Component
@Slf4j
@AllArgsConstructor
public class EccOracleSingleBackupInterceptor extends AbstractDbBackupInterceptor {
    private EccOracleBaseService oracleBaseService;

    private JobService jobService;

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.equalsSubType(resourceSubType);
    }

    /**
     * intercept
     *
     * @param backupTask 备份任务参数对象{@link BackupTask}
     * @return BackupTask
     */
    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Start SAP_ON_ORACLE_SINGLE backup single interceptor set parameters. uuid: {}",
            backupTask.getTaskId());
        checkExistRunningInstanceRestoreJob(backupTask.getProtectObject().getUuid(),
            backupTask.getProtectObject().getName());

        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 设置存储仓
        updateRepositories(backupTask);

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        ProtectedResource resource = oracleBaseService.getResourceById(backupTask.getProtectObject().getUuid());

        // 设置保护环境扩展参数
        setProtectEnvExtendInfo(backupTask, resource);

        // 设置agents
        backupTask.setAgents(oracleBaseService.getAgents(resource));

        // 设置环境nodes
        backupTask.getProtectEnv().setNodes(oracleBaseService.getEnvNodes(resource));
        log.info("End SAP_ON_ORACLE_SINGLE backup single interceptor set parameters. uuid: {}", backupTask.getTaskId());
        return backupTask;
    }

    /**
     * 根据备份任务类型的不同，更新仓库
     * 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            repositories.remove(0);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        String envUuid = backupTask.getProtectEnv().getUuid();
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envUuid);
        // 适配Windows系统，下发到ubc的仓库协议需要设置为CIFS
        oracleBaseService.repositoryAdaptsWindows(repositories, environment);
        backupTask.setRepositories(repositories);
    }

    private void setProtectEnvExtendInfo(BackupTask backupTask, ProtectedResource resource) {
        Map<String, String> envExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        // 设置auth信息
        backupTask.getProtectObject().setAuth(resource.getAuth());
    }

    private void checkExistRunningInstanceRestoreJob(String resourceId, String dbName) {
        QueryJobRequest queryJobRequest = new QueryJobRequest();
        queryJobRequest.setSourceTypes(Collections.singletonList(ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.getType()));
        queryJobRequest.setStatusList(Collections.singletonList(JobStatusEnum.RUNNING.name()));
        queryJobRequest.setTypes(Collections.singletonList(JobTypeEnum.INSTANT_RESTORE.getValue()));
        List<JobBo> runningJobs = jobService.queryJobs(queryJobRequest, new PagingParamRequest()).getRecords();
        for (JobBo jobBo : runningJobs) {
            RestoreTask restoreTask = RestoreTaskHelper.parseFromJobMessage(jobBo.getMessage());
            if (resourceId.equals(restoreTask.getTargetObject().getUuid()) && dbName.toLowerCase(Locale.ROOT)
                .equals(jobBo.getSourceName().toLowerCase(Locale.ROOT))) {
                throw new LegoCheckedException(CommonErrorCode.EXIST_INSTANCE_RESTORE_TASK,
                    "exist instance restore job");
            }
        }
    }
}
