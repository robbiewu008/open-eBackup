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
package openbackup.system.base.sdk.job.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;

import lombok.Data;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Job Guard Info
 *
 */
@Data
public class JobExamineInfo {
    private JobSchedulePolicy policy;
    private JobGuardStatus status;
    private List<String> cause;
    private JSONObject params;

    /**
     * get status
     *
     * @param items items
     * @return status
     */
    public static JobGuardStatus getStatus(List<JobExamineInfo> items) {
        if (VerifyUtil.isEmpty(items)) {
            return null;
        }
        List<JobGuardStatus> statusList = items.stream().map(JobExamineInfo::getStatus).collect(Collectors.toList());
        for (JobGuardStatus status : Arrays.asList(JobGuardStatus.CANCELLED, JobGuardStatus.PENDING)) {
            if (statusList.contains(status)) {
                return status;
            }
        }
        return null;
    }
}
