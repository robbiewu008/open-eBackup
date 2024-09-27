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
