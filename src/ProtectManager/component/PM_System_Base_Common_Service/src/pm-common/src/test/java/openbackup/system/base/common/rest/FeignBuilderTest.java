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

import static com.github.tomakehurst.wiremock.client.WireMock.aResponse;
import static com.github.tomakehurst.wiremock.client.WireMock.get;
import static com.github.tomakehurst.wiremock.client.WireMock.stubFor;
import static org.hamcrest.Matchers.is;
import static org.junit.Assume.assumeThat;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.rest.CommonDecoder;
import openbackup.system.base.common.rest.CommonRetryer;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.util.RequestUriUtil;

import com.github.tomakehurst.wiremock.junit.WireMockRule;

import feign.Client;
import feign.FeignException;
import feign.Request;
import feign.RequestLine;
import feign.Retryer;
import feign.codec.Encoder;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;

import java.io.File;
import java.net.Proxy;
import java.net.URI;
import java.nio.file.Paths;

/**
 * 测试FeignBuilder中默认重试次数，默认超时时间。
 *
 */
@PrepareForTest( {FeignBuilder.class, Client.class, Client.Default.class, File.class, Paths.class})
public class FeignBuilderTest {
    private static final int PORT_NUM = 18089;

    private static final String URL = "http://127.0.0.1:" + PORT_NUM;

    private static final URI DEST = URI.create(URL);
    // 測試時間

    /**
     * FeignClient连接默认超时时间(ms)
     */
    private static final int CONNECT_TIMEOUT = 3 * 1000; // 30s

    /**
     * FeignClient读取默认超时(ms)
     */
    private static final int READ_TIMEOUT = 2 * 1000; // 2分钟

    /**
     * FeignClient Retry 间隔周期(ms)
     */
    private static final int PERIOD = 6 * 1000; // 1分钟

    /**
     * FeignClient Retry 最大间隔周期(ms)
     */
    private static final int MAX_PERIOD = 6 * 1000; // 1分钟

    /**
     * FeignClient Retry 重试次数
     */
    private static final int MAX_ATTEMPTS = 1;

    @Rule
    public final WireMockRule wireMockRule = new WireMockRule(PORT_NUM);

    @Rule
    public final ExpectedException exceptionRule = ExpectedException.none();

    private TestClient api;

    /**
     * 测试客户端
     */
    interface TestClient {
        /**
         * 测试请求GET
         *
         * @param host 地址
         */
        @RequestLine("GET /test/all")
        void getTest(URI host);
    }

    /**
     * 用例场景：默认FeignClient重试次数测试。
     * 前置条件：Server返回500系统错误
     * 检查点： 1.是否抛出指定异常。2. 是否重试3次。
     */
    @Test
    public void test_get_retry_3times_success() throws Exception {
        exceptionRule.expect(LegoUncheckedException.class);
        exceptionRule.expectMessage("Server Error");

        Client.Default mockClient = Mockito.mock(Client.Default.class);
        PowerMockito.whenNew(Client.Default.class).withAnyArguments().thenReturn(mockClient);

        api = FeignBuilder.build(TestClient.class, CommonDecoder.decoder(), CommonDecoder::errorDecode, URL, null);
        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(500)));
        api.getTest(DEST);
        Mockito.verify(mockClient, Mockito.times(MAX_ATTEMPTS))
            .execute(Mockito.any(Request.class), Mockito.any(Request.Options.class));
    }

    /**
     * 用例场景：调用默认FeignClient，搭配CommonRetryer，重试次数测试。
     * 前置条件：默认FeignClient，并使用CommonRetryer,Server返回500系统错误
     * 检查点： 1.是否抛出指定异常。2. 是否重试3次。
     */
    @Test
    public void test_get_common_retry_3times_success() throws Exception {
        exceptionRule.expect(FeignException.class);
        exceptionRule.expectMessage("500 Server Error");

        Client.Default mockClient = Mockito.mock(Client.Default.class);
        PowerMockito.whenNew(Client.Default.class).withAnyArguments().thenReturn(mockClient);

        // 使用CommonRetryer用于重试
        api = FeignBuilder.getDefaultFeignBuilder().retryer(CommonRetryer.create()).target(TestClient.class, URL);

        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(500)));
        api.getTest(DEST);
        // Verify，看retry调用是否为3次
        Mockito.verify(mockClient, Mockito.times(MAX_ATTEMPTS))
            .execute(Mockito.any(Request.class), Mockito.any(Request.Options.class));
    }

    private String getOsName() {
        return System.getProperty("os.name");
    }

    /**
     * 用例场景：使用FeignBuilder创建客户端，在超时时间内，rest请求可以正常完成。
     * 前置条件：服务器处理在2分钟内
     * 检查点： 无异常抛出
     */
    @Test
    public void test_client_read_within_2minutes_success() {
        // 用例时长约2分钟，仅在本地执行。
        assumeThat(getOsName(), is("Windows 10"));
        api = FeignBuilder.build(TestClient.class, CommonDecoder.decoder(), CommonDecoder::errorDecode, URL, null);
        stubFor(get("/test/all").willReturn(
            aResponse().withHeader("Content-Type", "text/plain").withFixedDelay(READ_TIMEOUT - 1)));
        api.getTest(DEST);
    }

    /**
     * 用例场景：使用FeignBuilder创建默认客户端，处理超过超时时间，rest请求抛出异常。
     * 前置条件：服务器处理时间超过2分钟
     * 检查点：是否抛出read timeout
     */
    @Test
    public void should_throw_timeout_exception_if_server_is_over2min_when_get_api() {
        // 用例时长约2分钟，仅在本地执行。
        assumeThat(getOsName(), is("Windows 10"));

        api = FeignBuilder.getDefaultRetryableBuilder()
            .retryer(Retryer.NEVER_RETRY)
            .client(new Client.Default(null, null))
            .target(TestClient.class, URL);
        stubFor(get("/test/all").willReturn(
            aResponse().withHeader("Content-Type", "text/plain").withFixedDelay(READ_TIMEOUT).withStatus(200)));
        api.getTest(DEST);
    }

    /**
     * 用例场景：使用FeignBuilder的不同函数创建默认客户端，get请求成功。
     * 前置条件：使用FeignBuilder的不同函数创建默认客户端。
     * 检查点：无异常抛出
     */
    @Test
    public void get_with_different_clients_success() throws Exception {
        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(200)));

        api = FeignBuilder.buildConfigWithDefaultConfig(TestClient.class, null);
        api.getTest(DEST);

        api = FeignBuilder.buildHttpsTarget(TestClient.class, null, false);
        api.getTest(DEST);

        api = FeignBuilder.buildHttpsTargetWithProxy(TestClient.class, null, false,  null);
        api.getTest(DEST);

        api = FeignBuilder.buildInternalHttpsTarget(TestClient.class, new Encoder.Default(),
            new Client.Default(null, null));
        api.getTest(DEST);
    }

    /**
     * 用例场景：使用FeignBuilder的不同函数创建默认客户端，get请求成功。
     * 前置条件：使用FeignBuilder的不同函数创建默认客户端。
     * 检查点：无异常抛出
     */
    @Test(expected = LegoCheckedException.class)
    public void get_with_different_clients() {
        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(200)));

        api = FeignBuilder.buildDefaultTargetClusterClient(TestClient.class, new Encoder.Default(), null);
        api.getTest(DEST);
    }


    /**
     * 测试场景：通过代理访问http
     * 前置条件：client设置好代理
     * 检查点：访问成功，没有异常
     */
    @Test
    public void test_visit_http_by_proxy_success() {
        String proxyIp = "127.0.0.1";
        Proxy proxy = RequestUriUtil.getProxy(proxyIp, PORT_NUM);
        api = FeignBuilder.buildProxy(TestClient.class, CommonDecoder.decoder(), CommonDecoder::errorDecode, proxy);
        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(200)));
        api.getTest(DEST);
    }

    /**
     * 测试场景：通过代理访问http
     * 前置条件：client设置好代理
     * 检查点：访问成功，没有异常
     */
    @Test
    public void test_visit_http_by_proxy_without_retry_success() {
        String proxyIp = "127.0.0.1";
        Proxy proxy = RequestUriUtil.getProxy(proxyIp, PORT_NUM);
        api = FeignBuilder.buildProxyWithoutRetry(TestClient.class, CommonDecoder.decoder(), CommonDecoder::errorDecode,
            proxy);
        stubFor(get("/test/all").willReturn(aResponse().withHeader("Content-Type", "text/plain").withStatus(200)));
        api.getTest(DEST);
    }
}
