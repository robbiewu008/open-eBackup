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
package openbackup.system.base.controller;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.sdk.auth.model.request.PresetAccountRequest;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.service.PresetAccountServiceImpl;

import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 功能描述
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/external-account/action/save")
public class PresetAccountController {
    private final PresetAccountServiceImpl presetAccountService;

    /**
     * 构造方法
     *
     * @param presetAccountService presetAccountService
     */
    public PresetAccountController(PresetAccountServiceImpl presetAccountService) {
        this.presetAccountService = presetAccountService;
    }

    /**
     * 保存预置账号密码
     *
     * @param presetAccountRequest presetAccountRequest
     * @return UuidObject
     */
    @ExterAttack
    @PutMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = "0x206403460010", target = "System", details = {"$1.userName"})
    public UuidObject savePresetAccountAndPwd(@RequestBody PresetAccountRequest presetAccountRequest) {
        log.info("start savePresetAccountAndPwd");
        return presetAccountService.savePresetAccountAndPwd(presetAccountRequest);
    }
}
