/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package openbackup.system.base.sdk.livemount;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;

/**
 * Live Mount Feign Api
 *
 * @author m00576658
 * @since 2021-03-09
 */
@FeignClient(name = "live-mount-internal-service", url = "${service.url.pm-live-mount}/v1/internal/live-mount",
        configuration = CommonFeignConfiguration.class)
public interface LiveMountFeignApi {
    /**
     * reset live mount user id
     *
     * @param userId user id
     */
    @PutMapping("/action/revoke/{user_id}")
    void revokeLiveMountUserId(@PathVariable("user_id")String userId);

    /**
     * reset live mount policy user id
     *
     * @param userId user id
     */
    @PutMapping("/policy/action/revoke/{user_id}")
    void revokeLiveMountPolicyUserId(@PathVariable("user_id")String userId);
}
