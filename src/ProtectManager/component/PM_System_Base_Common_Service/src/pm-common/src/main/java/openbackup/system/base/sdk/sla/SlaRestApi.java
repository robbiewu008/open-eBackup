/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.sla;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Sla REST调用接口
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@FeignClient(
        name = "slaRestApiBase",
        url = "${services.endpoints.protectmanager.system-base}",
        configuration = CommonFeignConfiguration.class)
public interface SlaRestApi {
    /**
     * 验证sla所有权
     *
     * @param userId 用户id
     * @param uuidList sla id列表
     */
    @GetMapping("/v1/internal/sla/action/verify")
    @ResponseBody
    void verifySlaOwnership(
            @RequestParam("user_id") String userId, @RequestParam("sla_uuid_list") List<String> uuidList);
}
