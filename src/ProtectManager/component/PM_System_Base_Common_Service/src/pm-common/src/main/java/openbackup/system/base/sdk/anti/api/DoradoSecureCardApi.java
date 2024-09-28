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
package openbackup.system.base.sdk.anti.api;

import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.sdk.anti.model.SecureCardResp;
import openbackup.system.base.sdk.storage.DoradoFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * Dorado安全卡api
 *
 */
@FeignClient(value = "DoradoSecuryCardApi", url = "https://${repository.storage.ip}:${repository.storage.port}",
    configuration = DoradoFeignConfiguration.class)
public interface DoradoSecureCardApi {
    /**
     * 查询安全卡状态
     *
     * @param deviceId 设备id
     * @param baseToken 需要携带的会话iBaseToken
     * @param cookie cookie
     * @return 安全卡列表
     */
    @ExterAttack
    @RequestLine("GET /deviceManager/rest/{deviceId}/hardware_feature/active_hardware_feature")
    @Headers({"iBaseToken:{iBaseToken}", "Cookie:{Cookie}"})
    @ResponseBody
    StorageResponse<SecureCardResp> querySecureCard(@Param("deviceId") String deviceId,
        @Param("iBaseToken") String baseToken, @Param("Cookie") String cookie);
}
