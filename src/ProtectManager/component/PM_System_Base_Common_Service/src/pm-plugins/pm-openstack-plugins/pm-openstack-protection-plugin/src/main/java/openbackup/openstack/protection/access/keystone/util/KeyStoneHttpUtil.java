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
package openbackup.openstack.protection.access.keystone.util;

import openbackup.openstack.protection.access.constant.KeyStoneConstant;

import lombok.extern.slf4j.Slf4j;
import okhttp3.HttpUrl;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.CommonX509TrustHandler;
import openbackup.system.base.common.scurity.BcmX509TrustManager;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.bouncycastle.util.Arrays;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

/**
 * KeyStoneHttp工具类
 *
 */
@Slf4j
public class KeyStoneHttpUtil {
    private static OkHttpClient httpClient;

    private static final int[] SUCCESS_CODES = {200, 201};

    private static final int CONNECT_TIMEOUT = 30;

    private static final int READ_TIMEOUT = 30;

    private static final int CALL_TIMEOUT = 120;

    static {
        TrustManager[] trustManagers = new TrustManager[]{new BcmX509TrustManager(new CommonX509TrustHandler())};
        SSLContext sslContext;
        try {
            sslContext = SSLContext.getInstance(KeyStoneConstant.SSL_VERSION);
            sslContext.init(null, trustManagers, SecureRandom.getInstanceStrong());
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "init openstack connection failed.");
        }
        OkHttpClient.Builder builder = new OkHttpClient.Builder();
        if (trustManagers[0] instanceof X509TrustManager) {
            builder.sslSocketFactory(sslContext.getSocketFactory(), (X509TrustManager) trustManagers[0]);
        }
        builder.hostnameVerifier((hostname, session) -> true);
        builder.connectTimeout(CONNECT_TIMEOUT, TimeUnit.SECONDS);
        builder.readTimeout(READ_TIMEOUT, TimeUnit.SECONDS);
        builder.callTimeout(CALL_TIMEOUT, TimeUnit.SECONDS);
        httpClient = builder.build();
    }

    /**
     * get请求方法
     *
     * @param url   地址
     * @param param 查询参数
     * @param token header中X-Auth-Token
     * @param host  header中Host
     * @return 响应参数
     */
    public static Response syncGetRequest(String url, Map<String, String> param, String token, String host) {
        HttpUrl.Builder httpUrl = HttpUrl.parse(url).newBuilder();
        for (String name : param.keySet()) {
            httpUrl.addQueryParameter(name, param.get(name));
        }
        Request request = new Request.Builder()
            .addHeader(KeyStoneConstant.TOKEN_HEADER, token)
            .addHeader(KeyStoneConstant.HOST, host)
            .url(httpUrl.build())
            .get()
            .build();
        return httpClientExecute(request);
    }

    /**
     * 指定请求头的Get请求
     *
     * @param url     地址
     * @param headers 请求头参数
     * @return 响应参数
     */
    public static Response syncGetRequestWithHeader(String url, Map<String, String> headers) {
        HttpUrl.Builder httpUrl = HttpUrl.parse(url).newBuilder();
        Request.Builder builder = new Request.Builder()
            .url(httpUrl.build())
            .get();
        for (String header : headers.keySet()) {
            builder.addHeader(header, headers.get(header));
        }
        Request request = builder.build();
        return httpClientExecute(request);
    }

    /**
     * 不带Token的POST请求，用于获取Token
     *
     * @param url            请求地址
     * @param requestContent 请求body转换后json字符串
     * @param host           header中的Host
     * @return 响应参数
     */
    public static Response syncPostRequest(String url, String requestContent, String host) {
        RequestBody requestBody = RequestBody.create(MediaType.parse("application/json"), requestContent);
        Request request = new Request.Builder()
            .addHeader(KeyStoneConstant.HOST, host)
            .url(url)
            .post(requestBody)
            .build();
        return httpClientExecute(request);
    }

    /**
     * post请求方法
     *
     * @param url            请求地址
     * @param requestContent 请求body转换后json字符串
     * @param token          header中X-Auth-Token
     * @param host           header中Host
     * @return 响应参数
     */
    public static Response syncPostRequest(String url, String requestContent, String token, String host) {
        RequestBody requestBody = RequestBody.create(MediaType.parse("application/json"), requestContent);
        Request request = new Request.Builder()
            .addHeader(KeyStoneConstant.HOST, host)
            .addHeader(KeyStoneConstant.TOKEN_HEADER, token)
            .url(url)
            .post(requestBody)
            .build();
        return httpClientExecute(request);
    }

    private static Response httpClientExecute(Request request) {
        try {
            Response response = httpClient.newCall(request).execute();
            if (Arrays.contains(SUCCESS_CODES, response.code())) {
                return response;
            }
            log.error("request failed. response code:{}, response msg:{}", response.code(), response.message());
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Connect time out.");
        } catch (IOException e) {
            log.error("OpenStack request error.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "IO error.");
        }
    }
}
