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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * Live Mount Policy Internal Controller
 *
 */
@RestController
@RequestMapping("v1/internal/live-mount")
@Slf4j
public class LiveMountInternalController {
    private static final String CALLBACK_CANCEL_LIVE_MOUNT_ID = "callback.cancel.live_mount_id";

    @Autowired
    LiveMountService liveMountService;

    /**
     * reset live mount user id
     *
     * @param userId user id
     */
    @ExterAttack
    @PutMapping("/action/revoke/{user_id}")
    public void revokeLiveMountUserId(@PathVariable("user_id") String userId) {
        if (VerifyUtil.isEmpty(userId)) {
            return;
        }
        liveMountService.revokeLiveMountUserId(userId);
    }

    /**
     * cancel job
     *
     * @param jobBo job
     */
    @ExterAttack
    @PutMapping("/action/cancel")
    public void cancelJob(@RequestBody JobBo jobBo) {
        if (!VerifyUtil.isEmpty(jobBo.getCopyId())) {
            log.info("cancel live mount fail. jobJsonString copy id is null");
        }
        String liveMountId = JSONObject.fromObject(jobBo.getData()).getString(CALLBACK_CANCEL_LIVE_MOUNT_ID);
        if (VerifyUtil.isEmpty(liveMountId)) {
            log.info("cancel live mount fail. jobJsonString data without live mount id");
        }
        liveMountService.cancelLiveMount(liveMountId);
    }
}
