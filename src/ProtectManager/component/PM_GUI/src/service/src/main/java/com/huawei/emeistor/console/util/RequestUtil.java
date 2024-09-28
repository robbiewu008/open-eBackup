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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;

import org.apache.commons.lang3.ArrayUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.stereotype.Component;

import java.util.Optional;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

/**
 * Http Request 工具类，封装一些Request的公共工具方法
 *
 */
@Component
public final class RequestUtil {
    @Autowired
    private HttpServletRequest request;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    private RequestUtil() {
    }

    /**
     * 获取发起请求的IP地址
     *
     * @param request HTTP请求
     * @return String 发起请求的IP地址
     */
    @ExterAttack
    public static String getClientIpAddress(HttpServletRequest request) {
        String address = request.getHeader(ConfigConstant.REQUEST_IP);
        if (StringUtils.isNotBlank(address) && !"unknown".equalsIgnoreCase(address)) {
            int index = address.indexOf(",");
            if (index > 0) {
                address = address.substring(0, index);
            }
        } else {
            address = request.getRemoteAddr();
        }
        return address;
    }

    /**
     * 获取带Token的请求头
     *
     * @return 带Token的请求头信息
     */
    public HttpHeaders getForwardHeaderAndValidCsrf() {
        Cookie[] cookies = request.getCookies();
        String session = StringUtils.EMPTY;
        if (ArrayUtils.isNotEmpty(cookies)) {
            for (Cookie cookie : cookies) {
                if (StringUtils.equals(ConfigConstant.SESSION, cookie.getName())) {
                    session = cookie.getValue();
                    break;
                }
            }
        }
        HttpHeaders header = new HttpHeaders();
        if (!StringUtils.isEmpty(request.getHeader(ConfigConstant.HCS_FLAG))) {
            header.add(ConfigConstant.HCS_AUTH_TOKEN, request.getHeader(ConfigConstant.TOKEN));
        }
        if (StringUtils.isNotBlank(request.getHeader(ConfigConstant.MEMBER_ESN))) {
            header.add(ConfigConstant.MEMBER_ESN, request.getHeader(ConfigConstant.MEMBER_ESN));
        }
        String requestIp = getClientIpAddress(request);
        String requestId = request.getHeader(ConfigConstant.REQUEST_ID);
        if (!StringUtils.isEmpty(request.getHeader(ConfigConstant.DME_AUTH_TOKEN))) {
            header.add(ConfigConstant.DME_AUTH_TOKEN, request.getHeader(ConfigConstant.DME_AUTH_TOKEN));
        }
        if (!StringUtils.isEmpty(request.getHeader(ConfigConstant.DME_AZ))) {
            header.add(ConfigConstant.DME_AZ, request.getHeader(ConfigConstant.DME_AZ));
        }
        header.add(ConfigConstant.REQUEST_ID, NormalizerUtil.normalizeForString(requestId));
        header.add(ConfigConstant.REQUEST_IP, NormalizerUtil.normalizeForString(requestIp));
        String clusterType = request.getHeader(ConfigConstant.CLUSTER_TYPE) == null
                ? ConfigConstant.LOCAL_CLUSTER_TYPE
                : request.getHeader(ConfigConstant.CLUSTER_TYPE);
        header.add(ConfigConstant.CLUSTER_TYPE, clusterType);
        header.add(ConfigConstant.CLUSTER_ID, request.getHeader(ConfigConstant.CLUSTER_ID));
        header.add(ConfigConstant.MANAGE_IP, request.getHeader(ConfigConstant.HOST));
        if (StringUtils.isNotEmpty(session)) {
            Optional.ofNullable(sessionService.getSessionInfo(session).getToken()).ifPresent(
                    token -> header.add(ConfigConstant.TOKEN, encryptorRestClient.decrypt(token).getPlaintext()));
        }
        return header;
    }

    /**
     * 从请求中获取SessionInfo
     *
     * @return SessionInfo
     */
    public SessionInfo getSessionInfo() {
        Cookie[] cookies = request.getCookies();
        String session = StringUtils.EMPTY;
        if (ArrayUtils.isNotEmpty(cookies)) {
            for (Cookie cookie : cookies) {
                if (StringUtils.equals(ConfigConstant.SESSION, cookie.getName())) {
                    session = cookie.getValue();
                    break;
                }
            }
        }

        return StringUtils.isEmpty(session) ? new SessionInfo() : sessionService.getSessionInfo(session);
    }
}
