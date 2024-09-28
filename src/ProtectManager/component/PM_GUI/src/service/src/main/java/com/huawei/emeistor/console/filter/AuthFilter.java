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

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ErrorResponse;
import com.huawei.emeistor.console.util.RequestUtil;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.hc.core5.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.List;
import java.util.regex.Pattern;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 登录白名单过滤器
 *
 */
@Slf4j
public class AuthFilter extends OncePerRequestFilter {
    @Value("#{'${security.auth.white.list.uri}'.split(',')}")
    private List<String> authWhiteListUri;

    @Value("#{'${security.auth.white.list.static}'.split(',')}")
    private List<String> authWhiteListStatic;

    @Value("#{'${security.auth.white.list.static.regex}'.split(',')}")
    private List<String> authWhiteListStaticRegex;

    @Value("#{'${whitebox.path}'.split(',')}")
    private List<String> whiteBox;

    @Autowired
    private RequestUtil requestUtil;

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
        throws ServletException, IOException {
        String uri = request.getRequestURI();
        if (whiteBox.contains(uri)) {
            response.setStatus(HttpStatus.SC_NOT_FOUND);
        } else if (authWhiteListUri.contains(uri) || authWhiteListStatic.contains(uri) || isMatchStaticRegex(uri)
            || isAuth()) {
            filterChain.doFilter(request, response);
        } else {
            // 重定向到登录接口，前端无法捕获到重定向，给前端返回用户会话状态异常，前端进行跳转。
            log.warn("You need login first, uri: {}", uri);
            response.setStatus(HttpStatus.SC_SERVER_ERROR);
            ServletOutputStream outputStream = response.getOutputStream();
            outputStream.write(buildResponseBody());
            outputStream.flush();
        }
    }

    /**
     * 是否有token
     *
     * @return boolean
     */
    private boolean isAuth() {
        SessionInfo sessionInfo = requestUtil.getSessionInfo();
        return sessionInfo != null && StringUtils.isNotEmpty(sessionInfo.getToken());
    }

    /**
     * uri是否和增加随机数的静态页面正则表达式匹配
     *
     * @param uri 请求uri
     * @return boolean
     */
    private boolean isMatchStaticRegex(String uri) {
        return authWhiteListStaticRegex.stream().anyMatch(regex -> Pattern.compile(regex).matcher(uri).matches());
    }

    private byte[] buildResponseBody() {
        ErrorResponse errorResponse = new ErrorResponse();
        errorResponse.setErrorCode(CommonErrorCode.USER_STATUS_ABNORMAL + "");
        errorResponse.setErrorMessage("User's status is abnormal.");
        String responseBodyJson = JSONObject.toJSONString(errorResponse);
        return responseBodyJson.getBytes(Charset.defaultCharset());
    }
}
