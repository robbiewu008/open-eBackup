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
package openbackup.data.access.framework.core.config;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.util.RequestUriUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.net.InetSocketAddress;
import java.net.Proxy;

/**
 * 功能描述: AgentRestApiConfig
 *
 */
@Slf4j
@Configuration
public class AgentRestApiConfig {
    @Value("${dme.ip}")
    private String dmeProxyIp;

    @Value("${dme.proxyPort}")
    private Integer dmeProxyPort;

    /**
     * 生成访问agent的 rest api
     *
     * @return agentRestApi
     */
    @Bean
    public AgentUnifiedRestApi agentRestApi() {
        return FeignBuilder.buildHttpsTarget(AgentUnifiedRestApi.class, null, true, true, getDmeProxy());
    }

    private Proxy getDmeProxy() {
        RequestUriUtil.verifyIpAndPort(dmeProxyIp, dmeProxyPort);
        return new Proxy(Proxy.Type.HTTP, new InetSocketAddress(dmeProxyIp, dmeProxyPort));
    }
}