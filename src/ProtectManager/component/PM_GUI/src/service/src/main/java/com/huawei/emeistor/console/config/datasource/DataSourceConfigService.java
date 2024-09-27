/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package com.huawei.emeistor.console.config.datasource;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 数据库信息获取
 *
 * @author y30000858
 * @since 2021-01-22
 */
@FeignClient(name = "dataSourceService", url = "${service.url.infra}", configuration = FeignClientConfiguration.class)
public interface DataSourceConfigService {
    /**
     * 获取名为common-conf的ConfigMap的配置项
     *
     * @return DataSourceRes 配置信息
     */
    @GetMapping("/v1/infra/configmap/info?nameSpace=dpa&configMap=common-conf")
    @ResponseBody
    DataSourceRes getDataSourceConfig();

    /**
     * 获取名为common-secret的SecretMap配置项
     *
     * @return DataSourceRes 配置信息
     */
    @GetMapping("/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret")
    @ResponseBody
    DataSourceRes getDataSourceConfigSecreteMap();


    /**
     * 获取名为common-secret的SecretMap配置项
     *
     * @return DataSourceRes 配置信息
     */
    @GetMapping("/v1/infra/configmap/info?nameSpace=dpa&configMap=multicluster-conf&configKey=REDIS_CLUSTER")
    @ResponseBody
    DataSourceRes getRedisClusterInfo();
}
