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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTClusterUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBT副本删除Provider
 *
 */
@Slf4j
@Component
public class GaussDBTCopyDeleteInterceptorProvider extends AbstractDbCopyDeleteInterceptor {
    private final ProtectedEnvironmentService environmentService;

    /**
     * 构造器注入
     *
     * @param environmentService 环境服务
     * @param copyRestApi        copyRestApi
     * @param resourceService    resourceService
     */
    public GaussDBTCopyDeleteInterceptorProvider(ProtectedEnvironmentService environmentService,
                                                 CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
        this.environmentService = environmentService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(subType);
    }

    /**
     * 删除差异副本时，删除到下一次全量副本间的所有差异副本和有关联关系的日志副本
     *
     * @param copies       此副本之后的所有副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        log.info("gaussdbt get CumulativeIncrement AssociatedCopy");
        // 差异副本(增量差异)处理
        List<Copy> cumulativeCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.CUMULATIVE_INCREMENT);
        long thisCopyBackupTime = Long.parseLong(JSONObject.fromObject(thisCopy.getProperties())
                .getString(DatabaseConstants.COPY_BACKUP_TIME_KEY));
        // 全部日志副本
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        // 后一个日志副本
        Copy nextLogBackupCopy = copies.stream()
                .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
                .findFirst().orElse(null);
        // 若与下一个日志副本关联，删除到下一次全量副本间的所有差异副本和日志副本
        if (nextLogBackupCopy != null) {
            long logCopyStartTime = Long.parseLong(JSONObject.fromObject(nextLogBackupCopy.getProperties())
                    .getString(DatabaseConstants.LOG_COPY_BEGIN_TIME_KEY));
            if (thisCopyBackupTime == logCopyStartTime) {
                cumulativeCopies.addAll(logCopies);
            }
        }
        // 否则只删除到下一次全量副本间的差异副本
        return CopyUtil.getCopyUuidsBetweenTwoCopy(cumulativeCopies, thisCopy, nextFullCopy);
    }

    /**
     * 删除增量副本时，删除增量到下个增量或全量之间有关联关系的所有日志副本
     *
     * @param copies       此副本之后的所有副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        log.info("gaussdbt get DifferenceIncrement AssociatedCopy");
        // 增量副本（永久增量）处理
        long thisCopyBackupTime = Long.parseLong(JSONObject.fromObject(thisCopy.getProperties())
                .getString(DatabaseConstants.COPY_BACKUP_TIME_KEY));
        // 全部日志副本
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        // 后一个日志副本
        Copy nextLogBackupCopy = copies.stream()
                .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
                .findFirst().orElse(null);
        Copy nextDifferenceCopy = CopyUtil.getNextDifferenceCopy(copies, thisCopy.getGn());
        Copy nextCopy = CopyUtil.getSmallerCopy(nextFullCopy, nextDifferenceCopy);
        // 若与下一个日志副本关联，删除增量到下个增量或全量之间的所有日志副本
        if (nextLogBackupCopy != null) {
            long logCopyStartTime = Long.parseLong(JSONObject.fromObject(nextLogBackupCopy.getProperties())
                    .getString(DatabaseConstants.LOG_COPY_BEGIN_TIME_KEY));
            if (thisCopyBackupTime == logCopyStartTime) {
                return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, nextCopy);
            }
        }
        return Collections.emptyList();
    }

    /**
     * 删除全量副本时，要删除此副本到下一个全量副本之间的副本
     *
     * @param copies       此副本之后的所有副本
     * @param thisCopy     本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 前一个日志副本
        Copy previousLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
                thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation()).orElse(null);
        // 后一个日志副本
        Copy nextLogBackupCopy = copies.stream()
                .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
                .findFirst().orElse(null);
        // 如果之前没有日志副本或者之后没有日志副本
        if (previousLogBackupCopy == null || nextLogBackupCopy == null) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        long previousLogCopyEndTime = Long.parseLong(JSONObject.fromObject(previousLogBackupCopy.getProperties())
                .getString(DatabaseConstants.LOG_COPY_END_TIME_KEY));
        long nextLogCopyStartTime = Long.parseLong(JSONObject.fromObject(nextLogBackupCopy.getProperties())
                .getString(DatabaseConstants.LOG_COPY_BEGIN_TIME_KEY));
        // 如果后一个日志副本连不上前一个日志副本
        if (previousLogCopyEndTime < nextLogCopyStartTime) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        List<BackupTypeConstants> associatedTypes = new ArrayList<>(Arrays.asList(
                BackupTypeConstants.DIFFERENCE_INCREMENT, BackupTypeConstants.CUMULATIVE_INCREMENT));
        return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, associatedTypes);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("GaussDBT handle delete copy task. requestId: {}", task.getRequestId());
        // 设置高级参数挂载类型为：非全路径挂载
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(GaussDBTConstant.MOUNT_TYPE_KEY, MountTypeEnum.FULL_PATH_MOUNT.getMountType());
        task.setAdvanceParams(advanceParams);
        ProtectedEnvironment environment = null;
        try {
            environment = environmentService.getEnvironmentById(copy.getResourceId());
        } catch (LegoCheckedException e) {
            log.error("GaussDBT copy's resource is not exist.resourceId: {}", copy.getResourceId());
            return;
        }
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        List<TaskEnvironment> nodes = GaussDBTClusterUtil.getNodesFromEnv(environment);
        taskEnvironment.setNodes(nodes);
        task.setProtectEnv(taskEnvironment);
    }
}
