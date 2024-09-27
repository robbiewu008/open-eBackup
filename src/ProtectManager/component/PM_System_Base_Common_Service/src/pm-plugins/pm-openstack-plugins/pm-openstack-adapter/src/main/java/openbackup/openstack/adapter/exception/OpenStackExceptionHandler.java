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
package openbackup.openstack.adapter.exception;

import openbackup.system.base.common.exception.ErrorResponse;

import lombok.extern.slf4j.Slf4j;

import org.springframework.core.Ordered;
import org.springframework.core.annotation.Order;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

/**
 * OpenStack北向接口异常处理器
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
@Slf4j
@Order(Ordered.HIGHEST_PRECEDENCE)
@RestControllerAdvice("com.huawei.oceanprotect.openstack.adapter.controller")
public class OpenStackExceptionHandler {
    /**
     * 处理OpenStackException
     *
     * @param exception OpenStackException
     * @return 返回给请求的对象
     */
    @ExceptionHandler(OpenStackException.class)
    public final ResponseEntity<ErrorResponse> handleException(OpenStackException exception) {
        long errorCode = exception.getErrorCode();
        String message = exception.getMessage();

        ErrorResponse response = new ErrorResponse();
        response.setErrorCode(String.valueOf(errorCode));
        response.setErrorMessage(message);
        log.error("An error occurred when invoking the OpenStack interface", exception);
        return new ResponseEntity<>(response, HttpStatus.valueOf((int) errorCode));
    }
}
