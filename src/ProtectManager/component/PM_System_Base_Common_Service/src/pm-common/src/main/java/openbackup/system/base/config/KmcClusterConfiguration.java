/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
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
 * @author z00842230
 * @since 2023-10-13
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
