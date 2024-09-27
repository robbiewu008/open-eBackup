/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.dee.model.CommonResponse;
import openbackup.system.base.sdk.dee.model.ModifyEsClusterReq;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-08-12
 */
@FeignClient(name = "deeConfigRest", url = "${protectengine.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeConfigRest {
    /**
     * es集群信息配置接口
     *
     * @param modifyEsClusterReq 信息
     * @return 配置结果
     */
    @ExterAttack
    @PutMapping("/config/escluster")
    CommonResponse modifyEsCluster(@RequestBody @Validated ModifyEsClusterReq modifyEsClusterReq);
}