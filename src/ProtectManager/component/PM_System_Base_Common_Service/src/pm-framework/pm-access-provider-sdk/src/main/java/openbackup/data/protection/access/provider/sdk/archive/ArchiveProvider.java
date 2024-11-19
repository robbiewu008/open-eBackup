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
package openbackup.data.protection.access.provider.sdk.archive;

import com.fasterxml.jackson.core.JsonProcessingException;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.job.Task;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 */
public interface ArchiveProvider extends DataProtectionProvider<String> {
    /**
     * restore methods that need to be implemented by specific providers, This method is responsible for the business
     * logic of copy restore.
     *
     * @param archiveObject the restore object
     * @return the restore task
     */
    Task archive(ArchiveObject archiveObject);

    /**
     * 生成恢复任务
     *
     * @param archiveRequest 恢复请求
     * @return 任务json
     * @throws JsonProcessingException json转换异常
     */
    String createArchiveTask(ArchiveRequest archiveRequest) throws JsonProcessingException;
}
