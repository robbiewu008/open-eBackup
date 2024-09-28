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

import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Live Mount Ownership Verify Rest Api
 *
 */
public interface LiveMountOwnershipVerifyRestApi {
    /**
     * verify resource ownership
     *
     * @param userId            user id
     * @param liveMountUuidList live mount uuid list
     */
    @ExterAttack
    @GetMapping("/live-mount/action/verify")
    @ResponseBody
    void verifyLiveMountOwnership(@RequestParam("user_id") String userId,
        @RequestParam("live_mount_uuid_list") List<String> liveMountUuidList);

    /**
     * verify resource ownership
     *
     * @param userId   user id
     * @param uuidList live mount policy uuid list
     */
    @ExterAttack
    @GetMapping("/live-mount-policy/action/verify")
    @ResponseBody
    void verifyLiveMountPolicyOwnership(@RequestParam("user_id") String userId,
        @RequestParam("policy_uuid_list") List<String> uuidList);
}
