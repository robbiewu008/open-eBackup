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

import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Optional;

import javax.net.SocketFactory;
import javax.net.ssl.SSLSocketFactory;

/**
 * 功能描述
 *
 */
@Slf4j
public class NetWorkUtil {
    /**
     * socket连接超时时间
     */
    public static final int SOCKET_TIMEOUT = 5000;

    /**
     * 根据域名获取全部IP
     *
     * @param domainName 域名
     * @param port 端口
     * @return 域名对应的IP列表
     */
    public static Optional<String> getReachableDomainIp(String domainName, int port) {
        try {
            InetAddress[] inetAddresses = InetAddress.getAllByName(domainName);
            for (InetAddress inetAddress : inetAddresses) {
                String ip = inetAddress.getHostAddress();
                if (isIpReachableBySocket(ip, port)) {
                    return Optional.of(ip);
                }
            }
        } catch (UnknownHostException ex) {
            log.error("Get domain: {} ip failed.", domainName);
        }
        return Optional.empty();
    }

    /**
     * 测试网络连通性，使用socket方式
     *
     * @param ip 目标端ip
     * @param port 目标端端口
     * @return 是否网络通畅
     */
    public static boolean isIpReachableBySocket(String ip, int port) {
        SocketFactory socketFactory = SSLSocketFactory.getDefault();
        boolean isReachable = false;
        try (Socket socket = socketFactory.createSocket()) {
            socket.connect(new InetSocketAddress(ip, port), SOCKET_TIMEOUT);
            isReachable = true;
        } catch (IOException e) {
            log.warn("connect target host {} {} error by socket .", ip, port);
        }
        return isReachable;
    }
}
