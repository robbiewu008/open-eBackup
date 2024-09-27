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

import com.huawei.emeistor.console.contant.ConfigConstant;

import org.apache.commons.lang3.ArrayUtils;
import org.apache.commons.lang3.StringUtils;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * cookie 操作相关工具类
 *
 * @author t00482481
 * @since 2020-07-05
 */
public final class CookieUtils {
    private CookieUtils() {
    }

    /**
     * 从request中获取指定的cookie
     *
     * @param request request对象
     * @param cookieName cookieName
     * @return cookie内容
     */
    public static String get(HttpServletRequest request, String cookieName) {
        String res = StringUtils.EMPTY;
        Cookie[] cookies = request.getCookies();
        if (ArrayUtils.isNotEmpty(cookies)) {
            for (Cookie cookie : cookies) {
                if (cookie.getName().equals(cookieName)) {
                    res = cookie.getValue();
                }
            }
        }
        return res;
    }

    /**
     * 将响应中的cookie的过期时间设置为0，相当于清除cookie
     *
     * @param response http响应
     * @param sessionId sessionId
     */
    public static void clearCookie(HttpServletResponse response, String sessionId) {
        if (sessionId == null) {
            return;
        }
        Cookie cookie = new Cookie(ConfigConstant.SESSION, sessionId);
        cookie.setPath(ConfigConstant.COOKIE_PATH);
        cookie.setHttpOnly(true);
        cookie.setSecure(true);
        cookie.setMaxAge(0);
        response.addCookie(cookie);
    }
}
