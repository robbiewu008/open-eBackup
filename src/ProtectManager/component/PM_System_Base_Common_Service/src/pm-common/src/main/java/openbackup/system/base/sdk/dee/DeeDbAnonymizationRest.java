/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * dee database Anonymization
 *
 * @author y30044273
 * @since 2023-09-27
 */
@FeignClient(name = "deeDbAnonymizationRest", url = "${protectengine-e-dee-db-anonymization.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeDbAnonymizationRest {
    /**
     * 查询是否可以删除数据库脱敏策略
     *
     * @param dataBaseId 数据库资源
     * @return 是否可以删除
     */
    @GetMapping("/anonymization/job/isRunning")
    boolean isAnonymizationRunning(@RequestParam("dbId") String dataBaseId);
}
