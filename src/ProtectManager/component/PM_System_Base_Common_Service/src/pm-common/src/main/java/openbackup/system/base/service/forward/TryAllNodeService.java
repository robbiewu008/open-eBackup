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
