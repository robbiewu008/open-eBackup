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
package openbackup.system.base.sdk.job.util;

import openbackup.system.base.sdk.job.constants.JobProgress;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

/**
 * 任务更新工具类
 *
 *
 */
public class JobUpdateUtil {
    /**
     * 获取 进度为5 的任务更新体
     *
     * @return 进度为5 的任务更新体
     */
    public static UpdateJobRequest getDeliverReq() {
        return getUpdateJobRequest(JobProgress.DELIVER_JOB_PROGRESS);
    }

    /**
     * 获取 进度为96 的任务更新体
     *
     * @return 进度为96 的任务更新体
     */
    public static UpdateJobRequest getReportReq() {
        return getUpdateJobRequest(JobProgress.REPORT_JOB_PROGRESS);
    }

    private static UpdateJobRequest getUpdateJobRequest(int progress) {
        UpdateJobRequest jobRequest = new UpdateJobRequest();
        jobRequest.setProgress(progress);
        return jobRequest;
    }
}
