/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package com.huawei.emeistor.console.exception;

import com.huawei.emeistor.console.contant.CommonErrorCode;

import com.fasterxml.jackson.core.JsonLocation;
import com.fasterxml.jackson.core.JsonParseException;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.http.HttpStatus;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;

import java.net.SocketTimeoutException;

/**
 * 功能描述: WebExceptionHandlerTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-2-28
 */
public class WebExceptionHandlerTest {
    private final WebExceptionHandler handler = new WebExceptionHandler();

    @Test
    public void testHttpClientErrorExceptionHandler() {
        HttpClientErrorException clientErrorException = new HttpClientErrorException(HttpStatus.BAD_REQUEST);
        Assert.assertNotNull(handler.httpClientErrorExceptionHandler(clientErrorException));

        HttpServerErrorException serverErrorException = new HttpServerErrorException(HttpStatus.SERVICE_UNAVAILABLE);
        Assert.assertNotNull(handler.httpServerErrorExceptionHandler(serverErrorException));

        JsonParseException parseException = new JsonParseException("test", JsonLocation.NA);
        Assert.assertNotNull(handler.jsonParseException(parseException));

        SocketTimeoutException socketTimeoutException = new SocketTimeoutException();
        Assert.assertNotNull(handler.asyncRequestTimeoutExceptionHandler(socketTimeoutException));
    }
}