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
package openbackup.data.access.framework.protection.handler;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.util.Applicable;

/**
 * Task Complete Handler
 *
 * @author l00272247
 * @since 2020-12-18
 */
public interface TaskCompleteHandler extends Applicable<String> {
    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage);


    /**
     * 判断任务状态是否为成功
     *
     * @param taskCompleteMessage taskCompleteMessage
     */
    default void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
    }
}
