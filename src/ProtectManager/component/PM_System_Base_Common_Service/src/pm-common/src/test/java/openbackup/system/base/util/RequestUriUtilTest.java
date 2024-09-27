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
package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.config.DmaProxyProperties;
import openbackup.system.base.config.DmeProxyProperties;

import feign.Client;
import openbackup.system.base.util.RequestUriUtil;

import org.junit.Assert;
import org.junit.Test;

import java.net.Proxy;
import java.net.URI;

/**
 * RequestUriUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-26
 */
public class RequestUriUtilTest {
    private static final String TEST_IP = "127.0.0.1";
    private static final int TEST_PORT = 22;
    private static final Integer MAX_PORT_NUMBER = 65535;

    /**
     * 用例场景：获取URI
     * 前置条件：服务正常，IP和PORT正确
     * 检查点：返回一个预期URI
     */
    @Test
    public void get_request_uri_success() {
        URI requestUri = RequestUriUtil.getRequestUri(TEST_IP, TEST_PORT);
        Assert.assertNotNull(requestUri);
        Assert.assertEquals(TEST_IP, requestUri.getHost());
        Assert.assertEquals(TEST_PORT, requestUri.getPort());
    }

    /**
     * 用例场景：获取Proxy
     * 前置条件：服务正常，IP和PORT正确
     * 检查点：返回一个预期Proxy
     */
    @Test
    public void get_proxy_success() {
        Proxy proxy = RequestUriUtil.getProxy(TEST_IP, TEST_PORT);
        assertGetProxySuccess(proxy);
    }

    /**
     * 用例场景：获取dma Proxy
     * 前置条件：服务正常，IP和PORT正确
     * 检查点：返回一个预期Proxy
     */
    @Test
    public void get_dma_proxy_success() {
        DmaProxyProperties dmaProxyProperties = new DmaProxyProperties();
        dmaProxyProperties.setHost(TEST_IP);
        dmaProxyProperties.setPort(TEST_PORT);
        Proxy proxy = RequestUriUtil.getDmaProxy(dmaProxyProperties);
        assertGetProxySuccess(proxy);
    }

    /**
     * 用例场景：获取dme Proxy
     * 前置条件：服务正常，IP和PORT正确
     * 检查点：返回一个预期Proxy
     */
    @Test
    public void get_dme_proxy_success() {
        DmeProxyProperties dmeProxyProperties = new DmeProxyProperties();
        dmeProxyProperties.setIp(TEST_IP);
        dmeProxyProperties.setProxyPort(TEST_PORT);
        Proxy proxy = RequestUriUtil.getDmeProxy(dmeProxyProperties);
        assertGetProxySuccess(proxy);
    }

    /**
     * 用例场景：校验IP和PORT
     * 前置条件：服务正常，IP为空或PORT不正确
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_verify_ip_port_when_ip_is_empty_or_port_is_illegal() {
        Assert.assertThrows(LegoCheckedException.class, () -> RequestUriUtil.verifyIpAndPort(null, TEST_PORT));
        Assert.assertThrows(LegoCheckedException.class, () -> RequestUriUtil.verifyIpAndPort(TEST_IP, MAX_PORT_NUMBER + 1));
    }

    /**
     * 用例场景：校验IP和PORT
     * 前置条件：服务正常，IP和PORT正确
     * 检查点：校验通过
     */
    @Test
    public void verify_ip_port_success_when_ip_port_is_legal() {
        RequestUriUtil.verifyIpAndPort(TEST_IP, TEST_PORT);
    }

    /**
     * 用例场景：获取无验证的client成功
     * 前置条件：服务正常
     * 检查点：返回一个预期client
     */
    @Test
    public void get_no_verify_client_success() {
        Client client = RequestUriUtil.getNoVerifyClient();
        Assert.assertNotNull(client);
    }

    private void assertGetProxySuccess(Proxy proxy) {
        Assert.assertNotNull(proxy);
        Assert.assertEquals(Proxy.Type.HTTP, proxy.type());
        Assert.assertEquals("/" + TEST_IP + ":" + TEST_PORT, proxy.address().toString());
    }
}
