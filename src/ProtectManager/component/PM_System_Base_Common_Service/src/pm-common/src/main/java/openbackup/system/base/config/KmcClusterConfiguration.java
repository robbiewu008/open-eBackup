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
import openbackup.system.base.sdk.kmc.KmcClusterRestApi;
import openbackup.system.base.util.RequestUriUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Scope;
import org.springframework.context.annotation.ScopedProxyMode;

/**
 * 功能描述
 *
 */
@Slf4j
@Configuration
public class KmcClusterConfiguration {
    /**
     * 生成 KmcClusterApi, 添加超时时间为5分钟
     *
     * @param proxyProperties dme代理，域名和端口
     * @return KmcClusterRestApi
     */
    @Bean("kmcClusterApiWithDmaProxyCheckCert")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public KmcClusterRestApi createKmcClusterApiWithDmaProxyManagePort(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildDefaultMemberClusterClient(KmcClusterRestApi.class,
                RequestUriUtil.getDmaProxy(proxyProperties));
    }
}
