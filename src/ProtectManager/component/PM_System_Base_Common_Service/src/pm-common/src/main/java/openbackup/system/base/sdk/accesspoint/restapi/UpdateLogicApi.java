/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.accesspoint.restapi;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.accesspoint.model.DmeLogicIpsRequest;
import openbackup.system.base.sdk.accesspoint.model.InitNetworkResult;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * JobCenter Client Service
 *
 * @author swx1010572
 * @since 2020-01-12
 */
@FeignClient(name = "UpdateLogicApi", url = "${service.url.pm-dm-access-point}/v1/internal/eb",
    configuration = CommonFeignConfiguration.class)
public interface UpdateLogicApi {
    /**
     * 初始化备份存储
     *
     * @param dmeLogicIpsRequest 备份存储参数
     * @return 初始化结果
     */
    @PutMapping("/updateLogicIps")
    InitNetworkResult replicationLogicIps(@RequestBody DmeLogicIpsRequest dmeLogicIpsRequest);
}
