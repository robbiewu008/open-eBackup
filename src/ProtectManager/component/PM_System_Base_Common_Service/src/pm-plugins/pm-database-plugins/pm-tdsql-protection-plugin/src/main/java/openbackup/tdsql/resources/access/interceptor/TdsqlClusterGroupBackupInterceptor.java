/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.provider.TdsqlAgentProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述 分布式实例备份
 *
 * @author z00427109
 * @since 2023-05-30
 */
@Slf4j
@Component
public class TdsqlClusterGroupBackupInterceptor extends AbstractDbBackupInterceptor {
    private final TdsqlService tdsqlService;

    private final TdsqlAgentProvider tdsqlAgentProvider;

    /**
     * 构造器
     *
     * @param tdsqlService tdsqlService
     * @param tdsqlAgentProvider tdsqlAgentProvider
     */
    public TdsqlClusterGroupBackupInterceptor(TdsqlService tdsqlService, TdsqlAgentProvider tdsqlAgentProvider) {
        this.tdsqlService = tdsqlService;
        this.tdsqlAgentProvider = tdsqlAgentProvider;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType().equals(object);
    }

    /**
     * 填充agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        ProtectedResource resource = BeanTools.copy(backupTask.getProtectObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();
        List<Endpoint> endpointList = tdsqlAgentProvider.getSelectedAgents(agentSelectParam);
        backupTask.setAgents(endpointList);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 副本格式 日志备份用目录格式，全量、增量备份用快照格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        String backupType = backupTask.getBackupType();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 如果备份方式为日志备份, 添加：3-LOG_REPOSITORY 如果是日志备份，则只能在上一次进行全量备份的节点进行备份
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        } else {
            // 数据仓设置持续挂载参数: 1-DATA_REPOSITORY
            for (StorageRepository repo : repositories) {
                if (RepositoryTypeEnum.DATA.getType() != repo.getType()) {
                    continue;
                }
                Map<String, Object> repoExtendInfo = Optional.ofNullable(repo.getExtendInfo())
                    .orElseGet(HashMap::new);
                repoExtendInfo.put(TdsqlConstant.PERSISTENT_MOUNT, true);
                repoExtendInfo.put(TdsqlConstant.MANUAL_MOUNT, true);
                repoExtendInfo.put(TdsqlConstant.NEED_DELETE_DTREE, false);
                repo.setExtendInfo(repoExtendInfo);
            }
        }

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 获取oss信息下发给任务
        String clusterUuid = backupTask.getProtectObject().getParentUuid();
        log.info("clusterUuid is: {}", clusterUuid);
        ProtectedEnvironment clusterEnv = tdsqlService.getEnvironmentById(clusterUuid);
        OssNode ossNode = tdsqlService.getOssNode(clusterEnv).get(0);
        String ossUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        backupTask.getProtectObject().getExtendInfo().put(TdsqlConstant.OSS_URL, ossUrl);

        return backupTask;
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
