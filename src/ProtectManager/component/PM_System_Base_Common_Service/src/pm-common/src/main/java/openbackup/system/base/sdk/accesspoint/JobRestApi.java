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
package openbackup.system.base.sdk.accesspoint;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.accesspoint.model.StopPlanBo;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * JobCenter Client Service
 *
 */
@FeignClient(name = "PlanRestApi", url = "${service.url.pm-dm-access-point}/v1/internal/jobs",
    configuration = CommonFeignConfiguration.class)
public interface JobRestApi {
    /**
     * stop plan
     *
     * @param jobId      plan id
     * @param stopPlanBo job stop bo
     */
    @PutMapping("/{jobId}/action/stop")
    void stopTask(@PathVariable("jobId") String jobId, @RequestBody StopPlanBo stopPlanBo);
}
