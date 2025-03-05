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
package openbackup.data.protection.access.provider.sdk.retry;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 重试任务接口
 *
 */
public interface JobRetry extends DataProtectionProvider<String> {
    /**
     * 任务重试
     *
     * @param taskGuiRequest gui下发的参数
     * @return 重试任务id
     */
    String retryJob(String taskGuiRequest);
}
