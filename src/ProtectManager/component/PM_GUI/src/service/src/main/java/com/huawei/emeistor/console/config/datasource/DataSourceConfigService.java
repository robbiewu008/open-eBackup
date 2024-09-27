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
