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
package openbackup.system.base.sdk.livemount;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;

/**
 * Live Mount Feign Api
 *
 */
@FeignClient(name = "live-mount-internal-service", url = "${service.url.pm-live-mount}/v1/internal/live-mount",
        configuration = CommonFeignConfiguration.class)
public interface LiveMountFeignApi {
    /**
     * reset live mount user id
     *
     * @param userId user id
     */
    @PutMapping("/action/revoke/{user_id}")
    void revokeLiveMountUserId(@PathVariable("user_id")String userId);

    /**
     * reset live mount policy user id
     *
     * @param userId user id
     */
    @PutMapping("/policy/action/revoke/{user_id}")
    void revokeLiveMountPolicyUserId(@PathVariable("user_id")String userId);
}
