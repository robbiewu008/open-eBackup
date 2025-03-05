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

/**
 * 重试任务接口
 *
 */
public class JobRetryConstant {
    /**
     * 任务的扩展信息中保存的前端下发的参数，保存在t_job表的任务信息中的data字段中
     */
    public static final String TASK_GUI_REQUEST = "task_gui_request";

    /**
     * 任务类型，保存在t_job表的任务信息中的data字段中
     */
    public static final String TASK_TYPE = "task_type";
}
