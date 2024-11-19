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

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ResourceRedisLockService;
import openbackup.system.base.sdk.resource.model.ResourceLockRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.validation.Valid;

/**
 * 资源Redis冗余锁内部接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/internal/resource/redis")
@Validated
public class ResourceRedisLockInternalController {
    @Autowired
    private ResourceRedisLockService resourceRedisLockService;

    /**
     * Redis 加锁，并同步资源锁
     *
     * @param request request
     * @return 加锁结果
     */
    @ExterAttack
    @PutMapping("/lock")
    public boolean lock(@RequestBody @Valid ResourceLockRequest request) {
        log.info("start redis resource lock ,lock_id :{}", request.getLockId());
        return resourceRedisLockService.lockAndMultiClusterSync(request.getLockId(), request.getResources());
    }

    /**
     * Redis 解锁，并同步资源锁
     *
     * @param request request
     * @return 解锁结果
     */
    @ExterAttack
    @DeleteMapping("/lock")
    public boolean unlock(@RequestBody @Valid ResourceLockRequest request) {
        log.info("start redis resource unlock ,lock_id :{}", request.getLockId());
        return resourceRedisLockService.unlockAndMultiClusterSync(request.getLockId());
    }
}
