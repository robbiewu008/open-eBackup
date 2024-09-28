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
package openbackup.system.base.service.email;

import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSocket;

/**
 * 协议管理，主要提供通信协议的设置
 *
 */
@Slf4j
public class ProtocolManager {
    private ProtocolManager() {
    }

    /**
     * 根据指定的安全协议和加密套件重新设置SSLSocket使用的安全协议和加密套件
     * 1、若未指定安全协议则采用初始化SSLContext时使用的安全协议类型
     * 2、若未指定加密套件则使用默认安全协议对应的加密套件
     *
     * @param sslSocket SSLSocket
     * @param protocols 协议
     */
    public static void resetEnabledProtocolsAndCipherSuites(SSLSocket sslSocket, String[] protocols) {
        if (sslSocket == null) {
            log.error("sslSocket is null.");
            return;
        }

        String[] enabledProtocols = sslSocket.getEnabledProtocols();
        log.debug("sslSocket protocol: {},set protocols: {}.", Arrays.toString(enabledProtocols),
            Arrays.toString(protocols));

        if (protocols != null && protocols.length > 0) {
            sslSocket.setEnabledProtocols(protocols);
            enabledProtocols = protocols;
        }
        sslSocket.setEnabledCipherSuites(getCipherSuitesByProtocols(sortProtocolsByVersion(enabledProtocols)));
    }

    /**
     * 返回指定协议集的默认加密套件集
     *
     * @param protocolsParam 安全协议数组
     * @return String[] 默认加密套件集
     */
    private static String[] getCipherSuitesByProtocols(String[] protocolsParam) {
        Set<String> cipherSuites = new HashSet<>();
        for (String protocol : protocolsParam) {
            Set<String> suites = getCipherSuitesByProtocol(protocol);
            cipherSuites.addAll(suites);
        }

        return cipherSuites.toArray(new String[0]);
    }

    /**
     * 返回指定协议的默认加密套件集
     *
     * @param protocol 安全协议
     * @return Set<String>
     */
    private static Set<String> getCipherSuitesByProtocol(String protocol) {
        Set<String> cipherSuites = new HashSet<>();
        try {
            SSLContext sslcontext = SSLContext.getInstance(protocol);
            sslcontext.init(null, null, SecureRandom.getInstanceStrong());
            SSLParameters sslParam = sslcontext.getDefaultSSLParameters();
            if (sslParam == null) {
                log.error("sslParam is null.");
                return cipherSuites;
            }
            String[] defaultCipherSuites = sslParam.getCipherSuites();
            if (defaultCipherSuites == null) {
                log.error("defaultCipherSuites is null.");
                return cipherSuites;
            }
            for (String cipherSuite : defaultCipherSuites) {
                cipherSuites.add(cipherSuite);
            }
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            log.error("Getting cipher suites failed. Protocol: {}.", protocol, ExceptionUtil.getErrorMessage(e));
        }
        return cipherSuites;
    }

    /**
     * 返回根据安全协议类型和版本高低排序后的安全协议数组
     *
     * @param protocolsParam 安全协议数组
     * @return String[] 按安全协议类型和版本高低排序后的安全协议数组
     */
    private static String[] sortProtocolsByVersion(String[] protocolsParam) {
        List<String> protocolList = Arrays.asList(protocolsParam);
        protocolList.sort(new ProtocolComparator());

        return protocolList.toArray(new String[0]);
    }
}
