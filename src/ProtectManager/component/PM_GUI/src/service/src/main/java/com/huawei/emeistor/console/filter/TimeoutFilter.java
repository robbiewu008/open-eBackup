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

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.CookieUtils;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.apache.hc.core5.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.nio.charset.Charset;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 配置Filter
 *
 */
@Slf4j
public class TimeoutFilter extends OncePerRequestFilter {
    @Autowired
    private TimeoutUtils timeoutUtils;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private SHA256Encryptor sha256Encryptor;

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
        throws IOException, ServletException {
        String sessionId = CookieUtils.get(request, ConfigConstant.SESSION);
        try {
            String encSessionId = sha256Encryptor.encryptionSessionId(sessionId);
            String requestUrl = request.getRequestURI();
            if (timeoutUtils.getWhiteList().contains(requestUrl)) {
                log.info("white list url: {} , delete session cookie", requestUrl);
                sessionService.deleteSessionFromReq(request);
            } else {
                if (StringUtils.isNotEmpty(encSessionId) && sessionService.checkTimeout(encSessionId)) {
                    response.setStatus(HttpStatus.SC_MOVED_TEMPORARILY);
                    return;
                }
            }
        } catch (LegoCheckedException e) {
            // 报错统一返回500错误码
            response.setStatus(HttpStatus.SC_SERVER_ERROR);
            log.error("LegoCheckedException delete session cookie , error message: {}", e.getMessage());
            // 清理Cookie和缓存
            sessionService.deleteSessionFromReq(request);
            // 封装body
            byte[] responseBody = buildResponseBody(e.getErrorCode(), e.getMessage());
            ServletOutputStream outputStream = response.getOutputStream();
            outputStream.write(responseBody);
            outputStream.flush();
            return;
        } catch (Exception e) {
            response.setStatus(HttpStatus.SC_INTERNAL_SERVER_ERROR);
            log.error("Exception delete session cookie error message: {}", e.getMessage());
            // 清理Cookie和缓存
            sessionService.deleteSessionFromReq(request);
            return;
        }

        filterChain.doFilter(request, response);
    }

    private byte[] buildResponseBody(long errorCode, String errorMessage) {
        return new StringBuilder().append("{\"errorCode\":\"")
            .append(errorCode)
            .append("\",\"errorMessage\":\"")
            .append(errorMessage)
            .append("\"}")
            .toString()
            .getBytes(Charset.defaultCharset());
    }
}
