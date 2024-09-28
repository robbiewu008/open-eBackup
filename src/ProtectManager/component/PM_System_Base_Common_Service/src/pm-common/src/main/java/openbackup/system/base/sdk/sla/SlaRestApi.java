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
package openbackup.system.base.sdk.sla;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Sla REST调用接口
 *
 */
@FeignClient(
        name = "slaRestApiBase",
        url = "${services.endpoints.protectmanager.system-base}",
        configuration = CommonFeignConfiguration.class)
public interface SlaRestApi {
    /**
     * 验证sla所有权
     *
     * @param userId 用户id
     * @param uuidList sla id列表
     */
    @GetMapping("/v1/internal/sla/action/verify")
    @ResponseBody
    void verifySlaOwnership(
            @RequestParam("user_id") String userId, @RequestParam("sla_uuid_list") List<String> uuidList);
}
