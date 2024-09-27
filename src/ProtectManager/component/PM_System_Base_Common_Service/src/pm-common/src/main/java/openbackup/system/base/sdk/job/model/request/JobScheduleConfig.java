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

import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;

import java.util.List;

/**
 * Job Schedule Config
 *
 * @author l00272247
 * @since 2021-03-12
 */
@Data
public class JobScheduleConfig {
    @JsonAlias("job_type")
    private String jobType;

    private List<JobScheduleRule> rules;

    /**
     * create job schedule config
     *
     * @param jobType job type
     * @param rules rules
     * @return job schedule config
     */
    public static JobScheduleConfig create(String jobType, List<JobScheduleRule> rules) {
        JobScheduleConfig config = new JobScheduleConfig();
        config.setJobType(jobType);
        config.setRules(rules);
        return config;
    }
}
