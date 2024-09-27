/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.license;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * license service
 *
 * @author g00500588
 * @since 2021-01-29
 * */
@FeignClient(name = "LicenseService", url = "${service.url.pm-system-base}/v1/internal/license/",
        configuration = CommonFeignConfiguration.class)
public interface LicenseServiceApi {
    /**
     * functionLicense
     *
     * @param function function
     * @param resourceType resourceType
     * */
    @GetMapping("/function")
    void functionLicense(@RequestParam("function") String function,
                        @RequestParam("resourceType") String resourceType);
}
