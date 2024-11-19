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
package openbackup.gaussdbt.protection.access.provider;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * GaussDBT备份任务下发Provider
 *
 */
@Component
@Slf4j
public class GaussDBTBackupInterceptorProvider extends AbstractDbBackupInterceptor {
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
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
        }

        // 构建高级参数：非多文件系统、非全路径挂载、支持多任务
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(GaussDBTConstant.MULTI_FILE_SYSTEM_KEY, GaussDBTConstant.FALSE);
        advanceParams.put(GaussDBTConstant.MOUNT_TYPE_KEY, MountTypeEnum.FULL_PATH_MOUNT.getMountType());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        backupTask.setAdvanceParams(advanceParams);

        // 副本格式：0-快照格式 1-目录格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        return backupTask;
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        super.supplyNodes(backupTask);
        // 组装集群目标环境的节点信息
        List<TaskEnvironment> nodes = backupTask.getProtectEnv().getNodes();
        String nodeString = backupTask.getProtectObject().getExtendInfo().get(GaussDBTConstant.NODES_KEY);
        List<NodeInfo> nodeInfoList = JsonUtil.read(nodeString, new TypeReference<List<NodeInfo>>() {
        });
        List<TaskEnvironment> hostList = nodes.stream()
            .map(taskEnvironment -> nodeInfoList.stream()
                .filter(nodeInfo -> nodeInfo.getName().equals(taskEnvironment.getName()))
                .findFirst()
                .map(nodeInfo -> {
                    taskEnvironment.setUuid(nodeInfo.getUuid());
                    Map<String, String> extendInfo = Optional.ofNullable(taskEnvironment.getExtendInfo())
                        .orElse(new HashMap<>());
                    extendInfo.put(DatabaseConstants.ROLE, nodeInfo.getExtendInfo().get(DatabaseConstants.ROLE));
                    taskEnvironment.setExtendInfo(extendInfo);
                    return taskEnvironment;
                })
                .orElse(taskEnvironment))
            .collect(Collectors.toList());
        backupTask.getProtectEnv().setNodes(hostList);
        checkIsLogBackup(backupTask);
    }

    /**
     * PM侧校验日志备份，并将校验结果设置到任务的额外参数中
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkIsLogBackup(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            if (MapUtils.getBooleanValue(advanceParams, GaussDBTConstant.AUTO_FULL_BACKUP, false)) {
                advanceParams.put(GaussDBTConstant.IS_CHECK_BACKUP_JOB_TYPE, "true");
            }
        }
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(subType);
    }
}
