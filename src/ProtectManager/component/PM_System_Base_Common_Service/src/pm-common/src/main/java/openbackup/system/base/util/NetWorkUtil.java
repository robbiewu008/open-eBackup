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
import openbackup.system.base.common.cmd.Command;
import openbackup.system.base.common.constants.HcsConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

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
     * 根据域名获取全部IP解析(无论是否连通)
     *
     * @param domainName 域名
     * @return 域名对应的IP列表
     */
    public static List<String> getDomainIp(String domainName) {
        try {
            InetAddress[] inetAddresses = InetAddress.getAllByName(domainName);
            if (VerifyUtil.isEmpty(inetAddresses)) {
                log.warn("get empty ip record by domainName:{}", domainName);
                return Collections.emptyList();
            }
            return Arrays.stream(inetAddresses).map(InetAddress::getHostAddress).collect(Collectors.toList());
        } catch (UnknownHostException ex) {
            log.error("Get domain: {} ip failed.", domainName);
        }
        return Collections.emptyList();
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

    /**
     * 添加域名映射
     *
     * @param ip 待映射的ip
     * @param host 待映射的host
     * @return 添加域名映射是否成功
     */
    public static boolean addHostMapping(String ip, String host) {
        log.info("Start to add host mapping ip:{} host:{}.", ip, host);
        if (StringUtils.isEmpty(ip) || StringUtils.isEmpty(host)) {
            log.error("error input ip:{} or host:{}, will not add to host map!", ip, host);
            return false;
        }
        String entryToCheck = ip + " " + host;
        try (BufferedReader reader = new BufferedReader(new FileReader(HcsConstant.HOSTS_FILE_PATH))) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.contains(entryToCheck)) {
                    return true;
                }
            }
        } catch (IOException e) {
            log.error("Read ip:{} host:{} from hosts fail.", ip, host, ExceptionUtil.getErrorMessage(e));
        }
        String[] command = {"sudo", "python3", HcsConstant.PYTHON_SCRIPT_PATH, ip, host};
        if (getExitCode(command) != HcsConstant.SUCCESS_CODE) {
            log.error("Write ip:{} host:{} to hosts fail.", ip, host);
            return false;
        }
        log.info("Write ip:{} host:{} to hosts success.", ip, host);
        return true;
    }

    /**
     * 刪除域名映射
     *
     * @param ip 待刪除的ip
     * @return 刪除域名映射是否成功
     */
    public static boolean deleteHostMapping(String ip) {
        log.info("Start to delete host mapping ip:{}.", ip);
        try (BufferedReader reader = new BufferedReader(new FileReader(HcsConstant.HOSTS_FILE_PATH))) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (deleteHostMapping(ip, line)) {
                    return true;
                }
            }
        } catch (IOException e) {
            log.error("Read ip:{} from hosts fail.", ip, ExceptionUtil.getErrorMessage(e));
        }
        return true;
    }

    private static boolean deleteHostMapping(String ip, String line) {
        if (line.contains(ip)) {
            String[] command = {"sudo", "python3", HcsConstant.PYTHON_DELETE_HOST_SCRIPT_PATH, ip};
            if (getExitCode(command) != HcsConstant.SUCCESS_CODE) {
                log.error("Delete ip:{} from hosts fail.", ip);
                return false;
            }
            log.info("Delete ip:{} from hosts success.", ip);
            return true;
        }
        return false;
    }

    private static int getExitCode(String[] command) {
        // 等待命令执行完成
        int exitCode = Command.run(command);
        // 输出HTTP状态码和命令的退出状态
        log.info("Exit Code: " + exitCode);
        return exitCode;
    }
}
