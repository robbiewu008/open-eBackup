/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.livemount;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;

/**
 * Live Mount Ownership Verify Client Rest Api
 *
 * @author l00272247
 * @since 2020-11-28
 */
@FeignClient(name = "live-mount-ownership-verify-client-rest-api",
    url = "${service.url.pm-live-mount}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface LiveMountOwnershipVerifyClientRestApi extends LiveMountOwnershipVerifyRestApi {
}
