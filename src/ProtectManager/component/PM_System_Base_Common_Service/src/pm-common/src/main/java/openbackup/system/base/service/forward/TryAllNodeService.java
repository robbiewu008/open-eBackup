/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

package openbackup.system.base.service.forward;

import org.springframework.http.ResponseEntity;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 转发请求到具体一控，失败后尝试其他控制器，直到所有控制器都失败或者至少有一个控制器成功
 *
 * @author y30046482
 * @since 2023-12-23
 */
public interface TryAllNodeService {
    /**
     * 转发请求到具体一控，失败后尝试其他控制器，直到所有控制器都失败或者至少有一个控制器成功
     *
     * @param httpRequest http请求
     * @param httpServletResponse httpServletResponse
     * @return responseEntity
     */
    default ResponseEntity<Object> processTryAllNodeRequest(HttpServletRequest httpRequest,
        HttpServletResponse httpServletResponse) {
        return processTryAllNodeRequest(httpRequest, httpServletResponse, null, false, null);
    }

    /**
     * 转发请求到具体一控，失败后尝试其他控制器，直到所有控制器都失败或者至少有一个控制器成功
     *
     * @param httpRequest http请求
     * @param httpServletResponse httpServletResponse
     * @param requestBody Object
     * @param isOverwriteBody boolean
     * @param forwardCache ForwardCache
     * @return responseEntity
     */
    ResponseEntity<Object> processTryAllNodeRequest(HttpServletRequest httpRequest,
        HttpServletResponse httpServletResponse, Object requestBody,
        boolean isOverwriteBody, ForwardCache forwardCache);
}
