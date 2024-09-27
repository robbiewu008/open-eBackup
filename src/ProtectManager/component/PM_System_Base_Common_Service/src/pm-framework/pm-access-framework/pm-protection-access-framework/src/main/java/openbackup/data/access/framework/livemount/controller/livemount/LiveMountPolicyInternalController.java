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
package openbackup.data.access.framework.livemount.controller.livemount;

import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * Live Mount Policy Internal Controller
 *
 * @author m00576658
 * @since 2021-03-09
 */
@RestController
@RequestMapping("v1/internal/live-mount")
public class LiveMountPolicyInternalController {
    @Autowired
    PolicyService policyService;

    /**
     * reset live mount policy user id
     *
     * @param userId user id
     */
    @ExterAttack
    @PutMapping("/policy/action/revoke/{user_id}")
    public void revokeLiveMountPolicyUserId(@PathVariable("user_id") String userId) {
        if (VerifyUtil.isEmpty(userId)) {
            return;
        }
        policyService.revokeLiveMountPolicyUserId(userId);
    }
}
