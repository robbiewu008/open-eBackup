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
package openbackup.data.access.framework.protection.controller;

import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockResponse;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 资源锁接口
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/3
 */
@Slf4j
@RestController
@RequestMapping("")
public class LockController {
    @Autowired
    private ResourceLockRestApi resourceLockRestApi;

    /**
     * 同步加锁
     *
     * @param lockRequest 加锁的请求体
     * @return 加锁结果
     */
    @ExterAttack
    @PostMapping("/v1/internal/locks")
    public LockResponse lockWithLog(@RequestBody LockRequest lockRequest) {
        log.info("Try to lock, lock id is {}", lockRequest.getLockId());
        lockRequest.setLockId(lockRequest.getLockId() + "@log");
        lockRequest.setRequestId(lockRequest.getRequestId() + "@log");

        LockResponse lockResponse = resourceLockRestApi.lock(lockRequest);
        log.info("lock res is {}, lock id is {}", lockResponse.isSuccess(), lockRequest.getLockId());
        return lockResponse;
    }
}
