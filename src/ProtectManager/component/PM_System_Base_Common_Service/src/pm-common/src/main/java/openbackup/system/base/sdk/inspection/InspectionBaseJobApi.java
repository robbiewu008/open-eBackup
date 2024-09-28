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
package openbackup.system.base.sdk.inspection;

import openbackup.system.base.common.model.job.JobSummary;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 功能描述
 *
 */
@FeignClient(name = "InspectionBaseJobApi", url = "${pm-system-base.url}/v1",
        configuration = CommonFeignConfiguration.class)
public interface InspectionBaseJobApi {
    /**
     * Get jobs summary
     *
     * @param token 认证token
     * @return JobSummary obj
     */
    @ExterAttack
    @GetMapping(value = "/jobs/summary", headers = {"x-auth-token={token}"})
    JobSummary queryJobSummary(@RequestParam("token") String token);
}
