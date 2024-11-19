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
package openbackup.cnware.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * CNware 备份Provider
 *
 */
@Component
@Slf4j
public class CnwareBackupProvider implements BackupInterceptorProvider {
    private final CnwareCommonService cnwareCommonService;

    private final ResourceService resourceService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param cnwareCommonService cnwareCommonService
     */
    public CnwareBackupProvider(ResourceService resourceService, CnwareCommonService cnwareCommonService) {
        this.resourceService = resourceService;
        this.cnwareCommonService = cnwareCommonService;
    }

    /**
     * 应用备份拦截器，对备份请求进行拦截，对备份参数信息修改或扩展
     *
     * @param backupTask 备份任务参数对象{@link BackupTask}
     * @return 返回备份任务backupTask参数
     */
    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Start cnware backup, Backup taskId: {}", backupTask.getTaskId());

        // 设置备份格式：0-快照格式（原生格式） 1-目录格式（非原生）
        setCopyFormat(backupTask);

        // 填充磁盘卷信息
        fillProtectSubObjects(backupTask);

        // 设置存储仓信息
        setRepositories(backupTask);

        // 设置速率统计方式
        fillSpeedStatisticsType(backupTask);

        // 填充生产存储剩余容量阈值
        fillAvailableCapacityThreshold(backupTask);
        log.info("Cnware backup initialize finished, taskId: {}, backupType: {}, resourceId: {}.",
            backupTask.getTaskId(), backupTask.getBackupType(), backupTask.getProtectObject().getUuid());
        return backupTask;
    }

    private void fillAvailableCapacityThreshold(BackupTask backupTask) {
        log.info("Start to fill available capacity threshold, taskId:{}, resourceId:{}.",
            backupTask.getTaskId(), backupTask.getProtectObject().getUuid());
        if (!backupTask.getAdvanceParams().containsKey(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD)) {
            backupTask.getAdvanceParams().put(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD,
                String.valueOf(IsmNumberConstant.TWENTY));
            log.info("Fill available capacity threshold end, taskId:{}, resourceId:{}, available capacity threshold:{}",
                backupTask.getTaskId(), backupTask.getProtectObject().getUuid(),
                backupTask.getAdvanceParams().get(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD));
            return;
        }
        int capacityThreshold = Integer.parseInt(
            backupTask.getAdvanceParams().get(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD));
        if (capacityThreshold < IsmNumberConstant.ZERO || capacityThreshold > IsmNumberConstant.HUNDRED) {
            backupTask.getAdvanceParams().put(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD,
                String.valueOf(IsmNumberConstant.TWENTY));
        }
        log.info("Fill available capacity threshold end, taskId:{}, resourceId:{}, available capacity threshold:{}.",
            backupTask.getTaskId(), backupTask.getProtectObject().getUuid(),
            backupTask.getAdvanceParams().get(CnwareConstant.AVAILABLE_CAPACITY_THRESHOLD));
    }

    private void fillProtectSubObjects(BackupTask backupTask) {
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        String allDisk = advanceParams.get(CnwareConstant.ALL_DISK);
        if (Boolean.TRUE.toString().equals(allDisk)) {
            log.info("AllDisk backup, Backup taskId: {}", backupTask.getTaskId());
            return;
        }
        String diskInfoJson = advanceParams.get("disk_info");
        if (VerifyUtil.isEmpty(diskInfoJson)) {
            log.debug("Disk info is empty, taskId: {}.", backupTask.getTaskId());
            return;
        }
        List<TaskResource> resources = JSONArray.toCollection(JSONArray.fromObject(diskInfoJson), TaskResource.class);
        backupTask.setProtectSubObjects(resources);
    }

    private void setRepositories(BackupTask task) {
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        List<StorageRepository> repositories = task.getRepositories();
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        task.addRepository(cacheRepository);
    }

    private void fillSpeedStatisticsType(BackupTask task) {
        // 备份速率统计方式以UBC为准还是自己计算
        TaskUtil.setBackupTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
    }

    private static void setCopyFormat(BackupTask backupTask) {
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
    }

    /**
     * detect object applicable
     *
     * @param subType subType
     * @return detect result
     */
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(subType);
    }
}
