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

import feign.codec.Encoder;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.cluster.BackupClusterJobClient;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.util.RequestUriUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Scope;
import org.springframework.context.annotation.ScopedProxyMode;

/**
 * 目标集群feign请求bean 配置类
 *
 */
@Slf4j
@Configuration
public class TargetClusterConfiguration {
    @Autowired(required = false)
    private Encoder encoder;

    /**
     * 生成 TargetClusterRestApi, 使用DMA代理，用于复制目标集群之外的其他集群
     *
     * @param proxyProperties dma代理，域名和端口
     * @return TargetClusterRestApi
     */
    @Bean("targetClusterApiWithDmaProxy")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public TargetClusterRestApi createTargetRequestBean(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildTargetClusterClient(TargetClusterRestApi.class, encoder,
                RequestUriUtil.getDmaProxy(proxyProperties));
    }

    /**
     * 生成 TargetClusterRestApi, 使用DMA代理，校验是否有外部集群对应的证书
     *
     * @param proxyProperties dma代理，域名和端口
     * @return TargetClusterRestApi
     */
    @Bean("targetClusterApiWithDmaProxyManagePort")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public TargetClusterRestApi createTargetRequestBeanManagePort(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildDefaultTargetClusterClient(TargetClusterRestApi.class, encoder,
                RequestUriUtil.getDmaProxy(proxyProperties));
    }

    /**
     * 生成 TargetClusterRestApi, 使用备份网络平面代理，用于复制目标集群
     *
     * @return TargetClusterRestApi
     */
    @Bean("targetClusterApiWithRoute")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public TargetClusterRestApi createTargetApiWithDmeProxy() {
        return FeignBuilder.buildTargetClusterClient(TargetClusterRestApi.class, encoder, null);
    }

    /**
     * 生成 TargetClusterRestApi, 使用备份网络平面代理，校验是否有外部集群对应的证书
     *
     * @return TargetClusterRestApi
     */
    @Bean("defaultTargetClusterApiWithRoute")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public TargetClusterRestApi createDefaultTargetApiWithDmeProxy() {
        return FeignBuilder.buildDefaultTargetClusterClient(TargetClusterRestApi.class, encoder, null);
    }

    /**
     * 生成 TargetClusterRestApi, 添加超时时间为5分钟
     *
     * @param proxyProperties dme代理，域名和端口
     * @return TargetClusterRestApi
     */
    @Bean("memberClusterApiWithDmaProxyManagePort")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public TargetClusterRestApi createMemberClusterApiWithDmaProxyManagePort(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildMemberClusterClient(TargetClusterRestApi.class, encoder,
                RequestUriUtil.getDmaProxy(proxyProperties));
    }

    /**
     * 生成 BackupClusterJobClient, 添加超时时间为10秒
     *
     * @param proxyProperties dme代理，域名和端口
     * @return BackupClusterJobClient
     */
    @Bean("backupClusterJobClient")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public BackupClusterJobClient createBackupClusterJobClient(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildBackupClusterJobClient(BackupClusterJobClient.class, encoder,
                RequestUriUtil.getDmaProxy(proxyProperties));
    }
}
