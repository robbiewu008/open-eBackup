/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v1;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * The BackupFollowUpProvider
 *
 * @author g30003063
 * @since 2022/2/9
 */
public interface BackupFollowUpProvider extends DataProtectionProvider<String> {
    /**
     * 备份任务成功后续处理
     *
     * @param requestId 请求ID
     * @param jobId 任务ID
     * @param status 状态
     * @param copyId 副本ID
     */
    void handleSuccess(String requestId, String jobId, Integer status, String copyId);

    /**
     * 备份任务失败后续处理
     *
     * @param requestId 请求ID
     * @param jobId 任务ID
     * @param status 状态
     */
    void handleFailure(String requestId, String jobId, Integer status);
}
