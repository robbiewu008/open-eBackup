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

import org.springframework.http.HttpHeaders;

import java.util.Optional;

import javax.servlet.http.HttpServletRequest;

/**
 * 描述
 *
 * @see [相关类/方法]
 */
public class RequestUtils {
    private RequestUtils() {
    }

    /**
     * 校验CSRF，获取转发的header
     *
     * @param request HttpRequest
     * @return HttpHeaders
     */
    public static HttpHeaders getForwardHeaderAndValidCsrf(HttpServletRequest request) {
        HttpHeaders header = new HttpHeaders();
        header.add(ConfigConstant.REQUEST_ID, request.getHeader(ConfigConstant.REQUEST_ID));
        Optional.ofNullable(request.getSession())
            .map(httpSession -> httpSession.getAttribute(ConfigConstant.SESSION_TOKEN))
            .map(Object::toString)
            .ifPresent(token -> header.add(ConfigConstant.TOKEN, token));
        return header;
    }
}
