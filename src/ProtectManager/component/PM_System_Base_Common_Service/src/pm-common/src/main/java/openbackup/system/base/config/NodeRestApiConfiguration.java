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
package openbackup.system.base.config;

import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.config.api.PmConfigRestApi;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Scope;
import org.springframework.context.annotation.ScopedProxyMode;

/**
 * NodeRestApi配置类
 *
 */
@Configuration
public class NodeRestApiConfiguration {
    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 获取NodeRestApi
     *
     * @return nodeRestApi
     */
    @Bean("formDataNodeRestApi")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public NodeRestApi createFormDataNodeRestApiClient() {
        return FeignBuilder.buildInternalHttpsClientWithSpringMvcContract(NodeRestApi.class,
            feignClientConfig.getInternalClient());
    }

    /**
     * 获取NodeRestApi
     *
     * @return nodeRestApi
     */
    @Bean("defaultNodeRestApi")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public NodeRestApi createDefaultNodeRestApiClient() {
        return FeignBuilder.buildInternalHttpsClientWithSpringMvcContractDefaultEncoder(NodeRestApi.class,
            feignClientConfig.getInternalClient());
    }


    /**
     * 获取pmConfigRestApi
     *
     * @return nodeRestApi
     */
    @Bean("pmConfigRestApi")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public PmConfigRestApi createPmConfigRestApiClient() {
        return FeignBuilder.buildInternalHttpsClient(PmConfigRestApi.class, feignClientConfig.getInternalClient());
    }
}
