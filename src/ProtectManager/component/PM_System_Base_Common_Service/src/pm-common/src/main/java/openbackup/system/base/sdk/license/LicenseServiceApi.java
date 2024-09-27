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
package openbackup.system.base.sdk.license;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * license service
 *
 * @author g00500588
 * @since 2021-01-29
 * */
@FeignClient(name = "LicenseService", url = "${service.url.pm-system-base}/v1/internal/license/",
        configuration = CommonFeignConfiguration.class)
public interface LicenseServiceApi {
    /**
     * functionLicense
     *
     * @param function function
     * @param resourceType resourceType
     * */
    @GetMapping("/function")
    void functionLicense(@RequestParam("function") String function,
                        @RequestParam("resourceType") String resourceType);
}
