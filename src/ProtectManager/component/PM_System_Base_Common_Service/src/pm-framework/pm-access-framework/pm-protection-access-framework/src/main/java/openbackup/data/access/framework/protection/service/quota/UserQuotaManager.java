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
package openbackup.data.access.framework.protection.service.quota;

import static openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant.SIZE;

import com.huawei.oceanprotect.base.cluster.sdk.service.MultiClusterSummaryService;
import com.huawei.oceanprotect.system.base.quota.enums.QuotaTaskTypeEnum;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;
import com.huawei.oceanprotect.system.base.user.service.UserInternalService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.CopyInfo;

import org.apache.logging.log4j.util.Strings;
import org.springframework.stereotype.Component;

import java.util.Optional;

/**
 * 用户配额管理器
 *
 */
@Slf4j
@Component
public class UserQuotaManager {
    private static final String DATA_SIZE_DEFAULT_VALUE = "0";

    private final UserQuotaService userQuotaService;

    private final MultiClusterSummaryService multiClusterSummaryService;

    private final ResourceService resourceService;
    private final UserInternalService userInternalService;

    public UserQuotaManager(UserQuotaService userQuotaService, MultiClusterSummaryService multiClusterSummaryService,
                            ResourceService resourceService, UserInternalService userInternalService) {
        this.userQuotaService = userQuotaService;
        this.multiClusterSummaryService = multiClusterSummaryService;
        this.resourceService = resourceService;
        this.userInternalService = userInternalService;
    }

    /**
     * 校验备份额度，失败则记录任务日志
     *
     * @param userId 用户id
     * @param resourceId 资源id
     */
    public void checkBackupQuota(String userId, String resourceId) {
        userQuotaService.checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_BACKUP, false);
        log.info("User: {} has enough quota of resource: {} to backup.", userId, resourceId);
    }

    /**
     * 校验对象存储额度，失败则记录任务日志
     *
     * @param userId 用户id
     * @param resourceId 资源id
     */
    public void checkCloudArchiveQuota(String userId, String resourceId) {
        userQuotaService.checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_ARCHIVE, false);
        log.info("User: {} has enough quota of resource: {} to cloudArchive.", userId, resourceId);
    }

    /**
     * 用户执行复制任务时 进行配额校验(校验备份配额)
     * 校验目标端该用户的备份额度是否充足
     * 充足则允许执行复制任务，否则报错
     *
     * @param clusterId 目标端集群id
     * @param userId userId
     */
    public void checkUserBackupQuotaInTargetWhenReplication(Integer clusterId, String userId) {
        multiClusterSummaryService.checkTargetQuota(clusterId, userId, Strings.EMPTY,
            QuotaTaskTypeEnum.TASK_BACKUP.value());
        log.info("User: {} has enough quota in Target cluster:{} to replication.", userId, clusterId);
    }

    /**
     * HCS用户执行复制任务时 进行配额校验
     * 校验目标端该用户的备份额度是否充足
     * 充足则允许执行复制任务，否则报错
     *
     * @param clusterId 目标端集群id
     * @param userId userId
     */
    public void checkHcsUserReplicationQuota(Integer clusterId, String userId) {
        multiClusterSummaryService.checkTargetQuota(clusterId, userId, Strings.EMPTY,
            QuotaTaskTypeEnum.TASK_REPLICATED.value());
        log.info("User: {} has enough quota in Target cluster:{} to replication.", userId, clusterId);
    }

    /**
     * 增加用户已使用配额
     *
     * @param jobId 任务id
     * @param copy 副本
     */
    public void increaseUsedQuota(String jobId, CopyInfo copy) {
        if (copy == null) {
            log.warn("Increase quota copy is null, job: {}.", jobId);
            return;
        }
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String quota = properties.getString(SIZE, DATA_SIZE_DEFAULT_VALUE);
        // 通用场景，资源ID需传null
        userQuotaService.updateUserUsedQuota(copy.getUserId(), null, copy.getGeneratedBy(), UpdateQuotaType.INCREASE,
            quota);
        log.info("Unified increase user: {} quota: {} success, job: {}.", copy.getUserId(), quota, jobId);
    }

    /**
     * 减少用户已使用配额
     *
     * @param jobId 任务id
     * @param copy 副本
     */
    public void decreaseUsedQuota(String jobId, CopyInfo copy) {
        if (copy == null) {
            log.warn("Decrease quota copy is null, job: {}.", jobId);
            return;
        }
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String quota = properties.getString(SIZE, DATA_SIZE_DEFAULT_VALUE);
        // 通用场景，资源ID需传null
        userQuotaService.updateUserUsedQuota(copy.getUserId(), null, copy.getGeneratedBy(), UpdateQuotaType.REDUCE,
            quota);
        log.info("Unified reduce user: {} quota: {} success, job: {}.", copy.getUserId(), quota, jobId);
    }

    /**
     * 获取资源所属用户id
     *
     * @param userId 当前资源id
     * @param rootUuid 父资源uuid
     * @return 最终userId
     */
    public String getUserId(String userId, String rootUuid) {
        log.info("UserQuota Start to get userId userId:{},rootUuid{}", userId, rootUuid);

        // 当前资源id不为空 或 顶层环境id为空 都取当前资源用户id
        if (!VerifyUtil.isEmpty(userId) || VerifyUtil.isEmpty(rootUuid)) {
            log.debug("UserQuota get userId:{}.", userId);
            return userId;
        }

        // 否则取顶端环境资源用户id
        Optional<ProtectedResource> rootResource = resourceService.getResourceById(false, rootUuid);
        if (rootResource.isPresent()) {
            log.debug("UserQuota get root resource userId:{}.", rootResource.get().getUserId());
            return rootResource.get().getUserId();
        }
        return userId;
    }
}
