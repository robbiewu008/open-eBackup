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
package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.service.ResourceRedisLockService;
import openbackup.system.base.common.constants.CommonOperationCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.sdk.resource.model.ResourceLockRequest;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.validation.Valid;

/**
 * 资源Redis冗余锁
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/resources/redis")
public class ResourceRedisLockController {
    @Autowired
    private ResourceRedisLockService resourceRedisLockService;

    /**
     * Redis 加锁
     *
     * @param request request
     * @return 加锁结果
     */
    @ExterAttack
    @PutMapping("/lock")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.LOCK_RESOURCE, target = "resource",
        details = {"$1.lockId", "#join($1.resources.![resourceId], ', ')"})
    public boolean lock(@Valid @RequestBody ResourceLockRequest request) {
        log.info("start sync redis lock ,lock id :{}", request.getLockId());
        return resourceRedisLockService.acquireLock(request.getLockId(), request.getResources());
    }

    /**
     * Redis 解锁
     *
     * @param request request
     * @return 解锁结果
     */
    @ExterAttack
    @DeleteMapping("/lock")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.UNLOCK_RESOURCE, target = "resource", details = {"$1.lockId"})
    public boolean unlock(@Valid @RequestBody ResourceLockRequest request) {
        log.info("start sync redis unlock ,lock id :{}", request.getLockId());
        return resourceRedisLockService.unlock(request.getLockId());
    }
}
