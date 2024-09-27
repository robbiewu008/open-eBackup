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
package openbackup.system.base.sdk.job.constants;

/**
 * 任务 进度值常量
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-05-06
 */
public class JobProgress {
    /**
     * 任务下发到其他组件时的进度临界值
     */
    public static final int DELIVER_JOB_PROGRESS = 5;

    /**
     * 其他组件上报任务时的进度临界值
     */
    public static final int REPORT_JOB_PROGRESS = 96;
}
