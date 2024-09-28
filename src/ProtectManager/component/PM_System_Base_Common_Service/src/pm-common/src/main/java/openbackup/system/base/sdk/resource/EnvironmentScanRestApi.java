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
package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 功能描述
 *
 */
@FeignClient(name = "EnvironmentScanRestApi",
    url = "${services.endpoints.protectmanager.protection-service}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface EnvironmentScanRestApi {
    /**
     * protection service 手动资源扫描
     *
     * @param envId 环境id
     * @param jobId 任务id
     * @param subtype 资源子类型
     */
    @ExterAttack
    @PutMapping("/environments/rescan/{envId}")
    void doScanResource(@PathVariable("envId") String envId, @RequestParam("job_id") String jobId,
        @RequestParam("subtype") String subtype);
}
