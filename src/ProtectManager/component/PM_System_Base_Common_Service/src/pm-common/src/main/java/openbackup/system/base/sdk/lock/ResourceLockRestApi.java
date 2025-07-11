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
package openbackup.system.base.sdk.lock;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 资源锁服务rest api定义
 *
 **/
@FeignClient(
        name = "resourceLockRestApi",
        url = "${services.endpoints.protectmanager.protection-service}",
        path = "/v1",
        configuration = CommonFeignConfiguration.class)
public interface ResourceLockRestApi {
    /**
     * 锁定资源
     *
     * @param lockRequest 锁定资源请求
     * @return 资源锁id
     */
    @ExterAttack
    @ResponseBody
    @PostMapping("/internal/locks")
    LockResponse lock(@RequestBody LockRequest lockRequest);

    /**
     * 解锁资源
     *
     * @param lockId 资源锁id
     * @param requestId 任务请求id
     */
    @DeleteMapping("/internal/locks/{lockId}")
    void unlock(@PathVariable String lockId, @RequestParam String requestId);

    /**
     * 清空资源锁
     */
    @DeleteMapping("/internal/locks")
    void clear();
}
