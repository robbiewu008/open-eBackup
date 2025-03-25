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
package openbackup.system.base.sdk.resource;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import openbackup.system.base.common.rest.VmwareFeignConfiguration;
import openbackup.system.base.sdk.resource.model.ManualBackupReq;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.List;

/**
 * Resource Group Protected Object RestApi
 *
 */
@FeignClient(name = "ResourceGroupProtectedObjectRestApi",
        url = "${services.endpoints.protectmanager.protection-service}/v1/internal",
        configuration = VmwareFeignConfiguration.class)
public interface ResourceGroupProtectedObjectRestApi {
    /**
     * 执行手动备份
     *
     * @param resourceId 保护对象资源id
     * @param userId     用户id
     * @param backupReq  手动备份请求参数
     * @return 手动备份任务id列表
     */
    @ExterAttack
    @PostMapping("/protected-objects/{resource_id}/action/backup")
    List<String> manualBackup(@PathVariable("resource_id") String resourceId, @RequestParam("user_id") String userId,
                              @RequestBody ManualBackupReq backupReq);
}
