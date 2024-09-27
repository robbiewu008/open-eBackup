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
package openbackup.access.framework.resource.client;

import openbackup.access.framework.resource.client.model.AntiRansomwareVstoreListReq;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;

/**
 * 功能描述 新增或更新租户信息
 *
 * @author s30031954
 * @since 2022-12-29
 */
@FeignClient(name = "antiRansomwareExternalApi", url = "${pm-system-base.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface AntiRansomwareExternalApi {
    /**
     * 批量更新租户名称
     *
     * @param req 更新请求列表
     */
    @ExterAttack
    @PutMapping("/anti-ransomware/vstores")
    void updateVstoreNames(@Valid @RequestBody @NotNull AntiRansomwareVstoreListReq req);

    /**
     * 批量删除租户
     *
     * @param req 删除请求列表
     */
    @ExterAttack
    @DeleteMapping("/anti-ransomware/vstores")
    void deleteVstores(@Valid @RequestBody @NotNull AntiRansomwareVstoreListReq req);
}
