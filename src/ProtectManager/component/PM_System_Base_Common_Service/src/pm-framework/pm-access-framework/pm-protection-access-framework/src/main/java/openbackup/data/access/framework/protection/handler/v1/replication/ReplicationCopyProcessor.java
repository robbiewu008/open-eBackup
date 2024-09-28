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
package openbackup.data.access.framework.protection.handler.v1.replication;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.util.Applicable;

import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * Replication Copy Processor
 *
 */
public interface ReplicationCopyProcessor extends Applicable<String> {
    /**
     * process replication copy
     *
     * @param taskCompleteMessage task complete message
     * @return replicated copy number
     */
    AtomicStampedReference<Boolean> process(TaskCompleteMessageBo taskCompleteMessage);
}
