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
package openbackup.data.access.framework.protectobject.controller;

import openbackup.data.access.framework.protectobject.model.ProtectionExecuteCheckReq;
import openbackup.data.access.framework.protectobject.service.ProjectObjectService;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * 保护对象controller
 *
 */
@Slf4j
@AllArgsConstructor
@RestController
@RequestMapping("/v1/internal/protected-objects")
public class InternalProtectObjectController {
    private ProjectObjectService projectObjectService;

    /**
     * 操作保护对象时的回调操作
     *
     * @param protectionExecuteCheckReq protectionExecuteCheckReq
     */
    @ExterAttack
    @PostMapping("/action/callback")
    public void checkProtectObject(@RequestBody ProtectionExecuteCheckReq protectionExecuteCheckReq) {
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);
    }

    /**
     * 保护前校验之前资源备份的位置是否与本次保护一致
     *
     * @param slaId slaId
     * @param resourceId resourceId
     */
    @ExterAttack
    @GetMapping("/check-before-protect")
    public void checkExistCopiesLocationBeforeProtect(@RequestParam("slaId") String slaId,
        @RequestParam("resourceId") String resourceId) {
        projectObjectService.checkExistCopiesLocationBeforeProtect(slaId, resourceId);
    }
}
