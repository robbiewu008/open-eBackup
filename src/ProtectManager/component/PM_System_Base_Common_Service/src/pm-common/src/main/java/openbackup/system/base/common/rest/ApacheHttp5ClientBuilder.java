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
package openbackup.system.base.common.rest;

import openbackup.system.base.common.constants.PoolingHttpClientConstant;

import feign.hc5.ApacheHttp5Client;

import org.apache.hc.client5.http.impl.classic.HttpClients;
import org.apache.hc.client5.http.impl.io.PoolingHttpClientConnectionManagerBuilder;
import org.apache.hc.client5.http.ssl.NoopHostnameVerifier;
import org.apache.hc.client5.http.ssl.SSLConnectionSocketFactory;
import org.apache.hc.core5.pool.PoolConcurrencyPolicy;
import org.apache.hc.core5.pool.PoolReusePolicy;
import org.apache.hc.core5.ssl.SSLContextBuilder;
import org.apache.hc.core5.util.TimeValue;

import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

import javax.net.ssl.SSLContext;

/**
 * feign hc5客户端build类
 *
 */
public final class ApacheHttp5ClientBuilder {
    private ApacheHttp5ClientBuilder() {
    }

    /**
     * 创建ApacheHttp5Client ssl 免证书校验客户端
     *
     * @return ApacheHttp5Client
     * @throws NoSuchAlgorithmException NoSuchAlgorithmException
     * @throws KeyStoreException KeyStoreException
     * @throws KeyManagementException KeyManagementException
     */
    public static ApacheHttp5Client buildSslNoVerifyClient()
        throws NoSuchAlgorithmException, KeyStoreException, KeyManagementException {
        SSLConnectionSocketFactory sslConnectionSocketFactory = sslNoVerifyConnectionSocketFactory();
        return new ApacheHttp5Client(HttpClients.custom()
            .setConnectionManager(
                defaultPoolingHttpClientConnectionManagerBuilder()
                    .setSSLSocketFactory(sslConnectionSocketFactory)
                    .build())
            .build());
    }

    private static SSLConnectionSocketFactory sslNoVerifyConnectionSocketFactory()
        throws NoSuchAlgorithmException, KeyStoreException, KeyManagementException {
        SSLContext context = new SSLContextBuilder()
            .loadTrustMaterial(null, (chain, authType) -> true)
            .build();
        return new SSLConnectionSocketFactory(context, new NoopHostnameVerifier());
    }

    /**
     * 创建ApacheHttp5Client 默认客户端
     *
     * @return ApacheHttp5Client
     */
    public static ApacheHttp5Client buildDefaultClient() {
        return new ApacheHttp5Client(HttpClients.custom()
            .setConnectionManager(defaultPoolingHttpClientConnectionManagerBuilder().build())
            .build());
    }

    private static PoolingHttpClientConnectionManagerBuilder defaultPoolingHttpClientConnectionManagerBuilder() {
        return PoolingHttpClientConnectionManagerBuilder.create()
            .setConnPoolPolicy(PoolReusePolicy.LIFO)
            .setPoolConcurrencyPolicy(PoolConcurrencyPolicy.STRICT)
            .setMaxConnPerRoute(PoolingHttpClientConstant.MAX_CONN_PER_ROUTE)
            .setMaxConnTotal(PoolingHttpClientConstant.MAX_CONN_TOTAL)
            .setConnectionTimeToLive(TimeValue.ofMilliseconds(PoolingHttpClientConstant.TIME_TO_LIVE))
            .setValidateAfterInactivity(TimeValue.ofMilliseconds(PoolingHttpClientConstant.VALIDATE_AFTER_INACTIVITY));
    }

    /**
     * 创建ApacheHttp5Client ssl 证书校验客户端
     *
     * @param context context
     * @return ApacheHttp5Client
     */
    public static ApacheHttp5Client buildClientFromSslContext(SSLContext context) {
        SSLConnectionSocketFactory sslConnectionSocketFactory =
            new SSLConnectionSocketFactory(context, new NoopHostnameVerifier());
        return new ApacheHttp5Client(HttpClients.custom()
            .setConnectionManager(
                defaultPoolingHttpClientConnectionManagerBuilder().setSSLSocketFactory(sslConnectionSocketFactory)
                    .build())
            .build());
    }
}
