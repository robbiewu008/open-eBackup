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
package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.common.model.job.JobBo;

/**
 * 任务强制中止回调扩展接口
 * 不同任务流程实现此接口，用于任务强制中止后恢复任务执行对象的状态
 * （比如将恢复任务将副本重置为正常）
 *
 */
public interface JobCallbackProvider extends DataProtectionProvider<String> {
    /**
     * 不同任务类型处理任务强制中止后的状态恢复
     *
     * @param job 任务信息
     */
    void doCallback(JobBo job);
}
