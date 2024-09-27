/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author j00619968
 * @since 2024-01-23
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
