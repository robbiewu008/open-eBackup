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
package openbackup.data.access.framework.core.config;

import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.ErrorResponse;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;
import org.springframework.core.Ordered;
import org.springframework.core.annotation.Order;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * DataProtectionAccessException处理器，针对改异常进行拦截处理
 *
 * @author j00364432
 * @version [OceanProtect 1.1.0]
 * @since 2021-11-24
 */
@Slf4j
@RestControllerAdvice
@Order(Ordered.LOWEST_PRECEDENCE - 1)
public class DataProtectionAccessExceptionAdvice {
    private static final Set<Long> UNKNOWN_ERROR_CODES = new HashSet<>(Collections.singletonList(-1L));

    @Autowired
    @Qualifier("errorMessageSource")
    private MessageSource messageSource;

    /**
     * 处理错误码exception
     *
     * @param ex 异常信息
     * @return 返回给请求的对象
     */
    @ExceptionHandler(value = DataProtectionAccessException.class)
    @ResponseBody
    public final ResponseEntity<ErrorResponse> businessException(DataProtectionAccessException ex) {
        log.error("Data protection access exception occurs!");
        long code = ex.getErrorCode();
        long errorCode = UNKNOWN_ERROR_CODES.contains(code) ? CommonErrorCode.OPERATION_FAILED : code;
        String errorMessage;
        try {
            errorMessage = messageSource.getMessage(errorCode + "", ex.getParameters(), null);
        } catch (NoSuchMessageException noSuchMessageException) {
            errorMessage = ex.getMessage();
        }
        ErrorResponse errorResp = new ErrorResponse(errorCode + "", errorMessage, ex.getParameters(), false);
        HttpStatus status = HttpStatus.INTERNAL_SERVER_ERROR;
        if (errorCode == CommonErrorCode.ERR_PARAM) {
            status = HttpStatus.BAD_REQUEST;
        }
        if (errorCode == CommonErrorCode.ACCESS_DENIED) {
            status = HttpStatus.FORBIDDEN;
        }
        return new ResponseEntity<>(errorResp, status);
    }
}