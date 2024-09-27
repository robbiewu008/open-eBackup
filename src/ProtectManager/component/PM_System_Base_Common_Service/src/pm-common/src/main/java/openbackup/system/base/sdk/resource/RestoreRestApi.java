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

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.restore.model.RestoreRequestInternal;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

import java.util.List;

/**
 * 创建恢复任务v1内部接口
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-26
 */
@FeignClient(name = "RestoreRestApi", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface RestoreRestApi {
    /**
     * 调用内部接口下发恢复任务
     *
     * @param restoreRequest 恢复请求参数信息
     * @return 任务id列表
     */
    @ExterAttack
    @PostMapping("/internal/restores")
    List<UuidObject> createRestoreInternal(@RequestBody RestoreRequestInternal restoreRequest);
}
