/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.contant.ConfigConstant;

import org.springframework.http.HttpHeaders;

import java.util.Optional;

import javax.servlet.http.HttpServletRequest;

/**
 * 描述
 *
 * @author lwx544155
 * @version [OceanStor DJ V100R003C00, 2020年03月16日]
 * @see [相关类/方法]
 * @since [产品/模块版本]
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
