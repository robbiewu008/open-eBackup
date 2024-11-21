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