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
package openbackup.data.protection.access.provider.sdk.restore;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.job.Task;

import com.fasterxml.jackson.core.JsonProcessingException;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 * @author j00364432
 * @version [OceanStor 100P 8.1.0]
 * @since 2020-06-19
 */
public interface RestoreProvider extends DataProtectionProvider<RestoreObject> {
    /**
     * restore methods that need to be implemented by specific providers, This method is responsible for the business
     * logic of copy restore.
     *
     * @param restoreObject the restore object
     * @return the restore task
     */
    Task restore(RestoreObject restoreObject);

    /**
     * 生成恢复任务
     *
     * @param restoreObject 恢复请求
     * @return 任务json
     * @throws JsonProcessingException json转换异常
     */
    String createRestoreTask(RestoreRequest restoreObject) throws JsonProcessingException;
}
