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
package openbackup.exchange.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * Exchange单机备份拦截器
 *
 */
@Slf4j
@Component
public class ExchangeSingleNodeBackupInterceptor extends AbstractDbBackupInterceptor {
    private final ExchangeService exchangeService;

    /**
     * 构造器
     *
     * @param exchangeService exchangeService
     */
    public ExchangeSingleNodeBackupInterceptor(ExchangeService exchangeService) {
        this.exchangeService = exchangeService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType().equals(object);
    }

    /**
     * 填充agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        String envUuid = backupTask.getProtectEnv().getRootUuid();
        ProtectedEnvironment protectedEnvironment = exchangeService.getEnvironmentById(envUuid);
        List<Endpoint> supplyAgent = getSupplyAgent(protectedEnvironment.getDependencies());
        backupTask.setAgents(supplyAgent);

        // 日志备份加上检查任务类型校验
        checkIsLogBackup(backupTask);
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        exchangeService.setNextBackupTypeWhenLogBackFail(postBackupTask);
    }

    private List<Endpoint> getSupplyAgent(Map<String, List<ProtectedResource>> dependencies) {
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::applyAgentEndpoint)
            .collect(Collectors.toList());
    }

    private Endpoint applyAgentEndpoint(ProtectedEnvironment agentProtectedEnv) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentProtectedEnv.getEndpoint());
        endpoint.setPort(agentProtectedEnv.getPort());
        endpoint.setId(agentProtectedEnv.getUuid());
        endpoint.setAgentOS("windows");
        return endpoint;
    }

    /**
     * 不去检查实例连通性
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
        return;
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 全备、增备，设置副本格式快照格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());

        // 日志备份，设置副本格式为目录格式
        String backupType = backupTask.getBackupType();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        }

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 部署设置CIFS文件系统
        StorageRepository dataRepository = repositories.get(0);
        dataRepository.setProtocol(RepositoryProtocolEnum.CIFS.getProtocol());

        // 日志备份，追加log仓，默认携带ubc创建的目录格式data仓
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(dataRepository, StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 添加高级参数
        supplyAdvanceParams(backupTask);
        return backupTask;
    }

    /**
     * PM侧校验日志备份，并将校验结果设置到任务的额外参数中
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkIsLogBackup(BackupTask backupTask) {
        log.info("Check Is LogBackup, TaskType: {}", backupTask.getBackupType());
        if (ExchangeConstant.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            advanceParams.put(ExchangeConstant.IS_CHECK_BACKUP_JOB_TYPE, "true");
        }
    }


    /**
     * 添加高级参数
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void supplyAdvanceParams(BackupTask backupTask) {
        if (MapUtils.isEmpty(backupTask.getAdvanceParams()) || Objects.isNull(backupTask.getProtectObject())) {
            return;
        }

        Map<String, String> extendInfo = backupTask.getProtectObject().getExtendInfo();
        if (Objects.isNull(extendInfo)) {
            extendInfo = Maps.newHashMap();
            backupTask.getProtectObject().setExtendInfo(extendInfo);
        }

        extendInfo.putAll(backupTask.getAdvanceParams());
    }
}
