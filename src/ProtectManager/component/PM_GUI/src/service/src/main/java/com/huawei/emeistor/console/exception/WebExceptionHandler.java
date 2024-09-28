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
package com.huawei.emeistor.console.exception;

import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ErrorResponse;

import com.fasterxml.jackson.core.JsonParseException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.MessageSource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;

import java.net.SocketTimeoutException;

/**
 * 描述
 *
 * @see [相关类/方法]
 */
@Slf4j
@RestControllerAdvice
public class WebExceptionHandler {
    @Autowired
    @Qualifier("errorMessageSource")
    private MessageSource messageSource;

    /**
     * 功能描述
     *
     * @param ex HttpClientErrorException
     * @return ResponseEntity
     */
    @ExceptionHandler(HttpClientErrorException.class)
    public ResponseEntity<String> httpClientErrorExceptionHandler(HttpClientErrorException ex) {
        log.error("ERROR. code: {}, msg: {}", ex.getStatusCode(), ex.getMessage());
        return createResponseEntity(ex.getResponseBodyAsString(), ex.getResponseHeaders(), ex.getStatusCode());
    }

    /**
     * 功能描述
     *
     * @param ex HttpServerErrorException
     * @return ResponseEntity
     */
    @ExceptionHandler(HttpServerErrorException.class)
    public ResponseEntity<String> httpServerErrorExceptionHandler(HttpServerErrorException ex) {
        log.error("ERROR. code: {}, msg: {}", ex.getStatusCode(), ex.getMessage());
        return createResponseEntity(ex.getResponseBodyAsString(), ex.getResponseHeaders(), ex.getStatusCode());
    }

    /**
     * 处理错误码exception
     *
     * @param ex 异常信息
     * @return 返回给请求的对象
     */
    @ExceptionHandler(value = LegoCheckedException.class)
    @ResponseBody
    public final ResponseEntity<ErrorResponse> businessException(LegoCheckedException ex) {
        long errorCode = ex.getErrorCode();
        String errorMessage = messageSource.getMessage(errorCode + "", ex.getParameters(), null);
        ErrorResponse errorResp = new ErrorResponse(errorCode + "", errorMessage, ex.getParameters());
        return new ResponseEntity<>(errorResp, HttpStatus.INTERNAL_SERVER_ERROR);
    }

    /**
     * 处理请求Json格式错误JsonParseException
     *
     * @param ex JsonParseException
     * @return response
     */
    @ExceptionHandler(JsonParseException.class)
    @ResponseBody
    public ResponseEntity<ErrorResponse> jsonParseException(JsonParseException ex) {
        log.error("json parse error, message: {}", ex.getMessage());
        ErrorResponse response = new ErrorResponse();
        response.setErrorCode(CommonErrorCode.ERR_PARAM + "");
        response.setErrorMessage("Json parse error!");
        return new ResponseEntity<>(response, HttpStatus.BAD_REQUEST);
    }

    /**
     * 处理超时异常
     *
     * @param ex request timeout exception
     * @return 返回json格式的异常错误信息
     */
    @ExceptionHandler(value = SocketTimeoutException.class)
    @ResponseBody
    public ResponseEntity<ErrorResponse> asyncRequestTimeoutExceptionHandler(SocketTimeoutException ex) {
        ErrorResponse response = new ErrorResponse();
        log.error("request timed out: {}", ex.getMessage());
        response.setErrorCode(CommonErrorCode.REQUEST_TIMEOUT + "");
        response.setErrorMessage("The request timed out.");
        return new ResponseEntity<>(response, HttpStatus.INTERNAL_SERVER_ERROR);
    }

    private ResponseEntity<String> createResponseEntity(String body, HttpHeaders httpHeaders, HttpStatus httpStatus) {
        HttpHeaders responseHeaders = new HttpHeaders();
        if (httpHeaders != null) {
            responseHeaders.setContentType(httpHeaders.getContentType());
        }
        return new ResponseEntity<>(body, responseHeaders, httpStatus);
    }
}
