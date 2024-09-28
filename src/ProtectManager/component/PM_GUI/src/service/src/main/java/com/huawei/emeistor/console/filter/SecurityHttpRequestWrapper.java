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
package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.contant.CommonConstant;
import com.huawei.emeistor.console.contant.ConfigConstant;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;

/**
 * URL标准化处理
 *
 */
@Slf4j
public class SecurityHttpRequestWrapper extends HttpServletRequestWrapper {
    /**
     * http的URL地址隐含了默认的端口80
     */
    private static final int DEFAULT_HTTP_PORT = 80;

    /**
     * https的URL地址隐含了默认的端口443
     */
    private static final int DEFAULT_HTTPS_PORT = 443;

    /**
     * URL前缀
     */
    private static final String URL_PREFIX = "://";

    /**
     * 第一次请求时缓存下来
     */
    private String normalizeUri = StringUtils.EMPTY;

    /**
     * 第一次请求时缓存下来
     */
    private String normalizeUrl = StringUtils.EMPTY;

    public SecurityHttpRequestWrapper(HttpServletRequest request) {
        super(request);
    }

    @Override
    public String getRequestURI() {
        if (StringUtils.isEmpty(normalizeUri) || StringUtils.equals(normalizeUri, ConfigConstant.CONSOLE_PATH)) {
            normalizeUri = getNormalizeRequestURI(getRequest());
        }
        return normalizeUri;
    }

    @Override
    public StringBuffer getRequestURL() {
        if (StringUtils.isEmpty(normalizeUri)) {
            normalizeUrl = getNormalizeRequestURL(getRequest());
        }
        return new StringBuffer(normalizeUrl);
    }

    /**
     * Get Normalization request URI that contain context path and URI
     *
     * @param servletRequest ServletRequest
     * @return 归一化后的uri
     */
    private String getNormalizeRequestURI(ServletRequest servletRequest) {
        return servletRequest.getServletContext().getContextPath() + getNormalizeRequestServletURI(servletRequest);
    }

    /**
     * Get Normalization request URL that contain scheme+servername+port and URI
     *
     * @param servletRequest ServletRequest
     * @return 归一化后的url
     */
    private String getNormalizeRequestURL(ServletRequest servletRequest) {
        StringBuilder builder = new StringBuilder();
        builder.append(servletRequest.getScheme()).append(URL_PREFIX).append(servletRequest.getServerName());
        if (servletRequest.getServerPort() != DEFAULT_HTTP_PORT
            && servletRequest.getServerPort() != DEFAULT_HTTPS_PORT) {
            builder.append(CommonConstant.COLON).append(servletRequest.getServerPort());
        }
        builder.append(getNormalizeRequestURI(servletRequest));
        return builder.toString();
    }

    /**
     * Get Normalization request URI
     *
     * @param servletRequest ServletRequest
     * @return 归一化后的uri（不包括context path）
     */
    private String getNormalizeRequestServletURI(ServletRequest servletRequest) {
        String uri = StringUtils.EMPTY;
        try {
            HttpServletRequest httpServletRequest = requestDownCast(servletRequest);
            uri = httpServletRequest.getServletPath();
            if (StringUtils.isNotEmpty(httpServletRequest.getPathInfo())) {
                uri = uri + httpServletRequest.getPathInfo();
            }
        } catch (ServletException e) {
            log.error("Fail to get normalize request uri, please input right http servlet request.");
        }
        return uri;
    }

    /**
     * ServletRequest向下转型为HttpServletRequest
     *
     * @param servletRequest ServletRequest
     * @return HttpServletRequest
     * @throws ServletException 转型失败抛出异常
     */
    private HttpServletRequest requestDownCast(ServletRequest servletRequest) throws ServletException {
        if (servletRequest instanceof HttpServletRequest) {
            return (HttpServletRequest) servletRequest;
        } else {
            throw new ServletException("ServletRequest downcast to HttpServletRequest failed!");
        }
    }
}
