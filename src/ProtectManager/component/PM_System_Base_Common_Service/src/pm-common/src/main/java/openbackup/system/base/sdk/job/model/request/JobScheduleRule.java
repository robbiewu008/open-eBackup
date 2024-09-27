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
package openbackup.system.base.sdk.job.model.request;

import openbackup.system.base.sdk.job.model.JobStatusEnum;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * Job Schedule Rule
 *
 * @author l00272247
 * @since 2021-03-12
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class JobScheduleRule {
    private Object scope;
    @JsonAlias("global_job_limit")
    private Map<String, Integer> globalJobLimit;
    @JsonAlias("scope_job_limit")
    private int scopeJobLimit = 0;
    @JsonAlias("strict_scope")
    private boolean isStrictScope = true;
    @JsonAlias("major_priority")
    private int majorPriority = 0;
    @JsonAlias("minor_priorities")
    private List<String> minorPriorities;
    @JsonAlias("examine")
    private String examine;
    @JsonAlias("resume_status")
    private JobStatusEnum resumeStatus;
    @JsonAlias("pending_window")
    private long pendingWindow;

    @JsonAlias("strict_scope")
    public void setStrictScope(boolean isStrictScope) {
        // 单独为strictScope写set方法，避免获取不到值
        this.isStrictScope = isStrictScope;
    }
}
