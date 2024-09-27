/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
