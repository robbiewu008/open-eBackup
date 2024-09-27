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
package openbackup.sqlserver.protection.backup;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * SQL Server数据库备份拦截器实现类
 *
 * @author xwx1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-02
 */
@Slf4j
@Component
public class SqlServerBackupInterceptor extends AbstractDbBackupInterceptor {
    private final JobService jobService;

    private final SqlServerBaseService sqlServerBaseService;

    /**
     * SQL Server备份拦截器构造方法
     *
     * @param jobService job操作服务
     * @param sqlServerBaseService sqlserver保护基础服务
     */
    public SqlServerBackupInterceptor(JobService jobService, SqlServerBaseService sqlServerBaseService) {
        this.jobService = jobService;
        this.sqlServerBaseService = sqlServerBaseService;
    }

    /**
     * SQL Server数据库备份任务自定义
     * <p>
     * 1. 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     * 2. 设置备份任务拆分时所需要的部署类型
     *
     * @param backupTask 初始的备份对象
     * @return 经过应用拦截器后的备份对象
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        backupTask.setCopyFormat(DatabaseConstants.DIRECTORY);
        // 更新存储仓
        updateRepositories(backupTask);
        // 设置部署类型
        Map<String, String> envExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        // 针对数据库备份，设置auth信息
        TaskResource protectObject = backupTask.getProtectObject();
        if (ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType().equals(protectObject.getSubType())) {
            ProtectedResource resource = sqlServerBaseService.getResourceByUuid(protectObject.getParentUuid());
            protectObject.setAuth(resource.getAuth());
        }
        // 填充上一次备份任务状态
        getPreBackupJob(protectObject.getUuid()).ifPresent(
            job -> protectObject.getExtendInfo().put(DatabaseConstants.PRE_BACKUP_JOB_STATUS, job.getStatus()));
        return backupTask;
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()).contains(subType);
    }

    /**
     * 根据备份任务类型的不同，更新仓库
     * 全量备份需要设置data,cache 2种repository, 框架已经设置了data repository
     * 如果是日志备份需要设置log,cache仓库
     *
     * @param backupTask 通用备份框架备份任务
     */
    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (repositories.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "[SQL Server] backup task has no repository.");
        }
        // 数据库任务默认含数据仓，日志备份改为日志仓，均使用CIFS协议
        StorageRepository defaultRepository = repositories.get(IsmNumberConstant.ZERO);
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            defaultRepository.setType(RepositoryTypeEnum.LOG.getType());
        }
        defaultRepository.setProtocol(RepositoryProtocolEnum.CIFS.getProtocol());
        // 新增CACHE仓
        StorageRepository cacheRepository = BeanTools.copy(defaultRepository, StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        log.info("[SQL Server] backup task repositories size: {}", repositories.size());
    }

    /**
     * 配置备份任务的Agent资源
     * 如果是数据库粒度的资源备份，需要根据数据库的parentUuid找到对应的单实例资源，
     * 然后再根据单实例资源的parentUuid，找到对应的主机
     * 然后根据Agent主机，拿到Agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        List<Endpoint> agents = sqlServerBaseService.convertNodeListToAgents(backupTask.getProtectObject().getUuid());
        backupTask.setAgents(agents);
    }

    /**
     * mysql supply Nodes
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> taskEnvironments = sqlServerBaseService.queryNodeList(
            backupTask.getProtectObject().getUuid());
        backupTask.getProtectEnv().setNodes(taskEnvironments);
    }

    private Optional<JobBo> getPreBackupJob(String resourceUuid) {
        // 过滤条件
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setSourceId(resourceUuid);
        conditions.setTypes(Collections.singletonList(DatabaseConstants.BACKUP));
        conditions.setStatusList(Arrays.asList(DatabaseConstants.SUCCESS, DatabaseConstants.FAIL));
        // 排序规则
        SortingParamRequest sortRule = new SortingParamRequest();
        sortRule.setOrderBy(DatabaseConstants.START_TIME);
        sortRule.setOrderType(SortingParamRequest.DES);
        // 分页大小
        PagingParamRequest pageParam = new PagingParamRequest();
        pageParam.setPageSize(1);
        List<JobBo> records = jobService.queryJobs(conditions, pageParam, sortRule).getRecords();
        return records.stream().findFirst();
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}