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

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Optional;

import javax.net.SocketFactory;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

@RunWith(PowerMockRunner.class)
@PrepareForTest({NetWorkUtil.class, Socket.class, SocketFactory.class, SSLSocketFactory.class, InetAddress.class})
public class NetWorkUtilTest {
    private static final String DOMAIN_NAME = "example.com";

    private static final int PORT = 80;

    private static final String IP_ADDRESS = "192.168.1.1";

    /**
     * 用例场景：通过域名获取到可连接的IP
     * 前置条件：域名IP联通
     * 检查点：获取到可连接的IP
     */
    @Test
    public void testGetReachableDomainIp_whenInetAddressGetAllByName_thenReturnIpAddress() throws Exception {
        PowerMockito.mockStatic(InetAddress.class);
        InetAddress mock = PowerMockito.mock(InetAddress.class);
        PowerMockito.when(mock.getHostAddress()).thenReturn(IP_ADDRESS);
        InetAddress[] inetAddresses = {mock};
        PowerMockito.when(InetAddress.getAllByName(DOMAIN_NAME)).thenReturn(inetAddresses);
        mockSocketConnected();
        Optional<String> ip = NetWorkUtil.getReachableDomainIp(DOMAIN_NAME, PORT);
        Assert.assertTrue(ip.isPresent());
        Assert.assertEquals(IP_ADDRESS, ip.get());
    }

    /**
     * 用例场景：通过域名获取到可连接的IP失败
     * 前置条件：域名IP不联通
     * 检查点：获取到可连接的IP为空
     */
    @Test
    public void testGetReachableDomainIp_whenIsIpReachableBySocketReturnsFalse_thenReturnEmpty() throws Exception {
        PowerMockito.mockStatic(InetAddress.class);
        InetAddress mock = PowerMockito.mock(InetAddress.class);
        PowerMockito.when(mock.getHostAddress()).thenReturn(IP_ADDRESS);
        InetAddress[] inetAddresses = {mock};
        PowerMockito.when(InetAddress.getAllByName(DOMAIN_NAME)).thenReturn(inetAddresses);
        mockSocketConnectFailed();
        Optional<String> ip = NetWorkUtil.getReachableDomainIp(DOMAIN_NAME, PORT);
        Assert.assertFalse(ip.isPresent());
    }

    /**
     * 用例场景：通过Socket检测联通性失败
     * 前置条件：IP不联通
     * 检查点：Socket检测联通性失败,返回False
     */
    @Test
    public void testIsIpReachable_IOException() throws IOException {
        mockSocketConnectFailed();
        Assert.assertFalse(NetWorkUtil.isIpReachableBySocket("127.0.0.1", 80));
    }

    /**
     * 用例场景：通过Socket检测联通性成功
     * 前置条件：IP联通
     * 检查点：Socket检测联通性成功,返回True
     */
    @Test
    public void testIsIpReachable_Success() throws Exception {
        mockSocketConnected();
        Assert.assertTrue(NetWorkUtil.isIpReachableBySocket("127.0.0.1", 80));
    }

    private void mockSocketConnectFailed() throws IOException {
        PowerMockito.mockStatic(SSLSocketFactory.class);
        SSLSocketFactory factory = PowerMockito.mock(SSLSocketFactory.class);
        PowerMockito.when(SSLSocketFactory.getDefault()).thenReturn(factory);
        PowerMockito.when(factory.createSocket()).thenThrow(new IOException());
    }

    private void mockSocketConnected() throws IOException {
        PowerMockito.mockStatic(SSLSocketFactory.class);
        SSLSocketFactory factory = PowerMockito.mock(SSLSocketFactory.class);
        PowerMockito.when(SSLSocketFactory.getDefault()).thenReturn(factory);
        SSLSocket socket = PowerMockito.mock(SSLSocket.class);
        PowerMockito.when(factory.createSocket()).thenReturn(socket);
        PowerMockito.when(socket.isConnected()).thenReturn(true);
    }
}