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
package openbackup.system.base.common.rest;

import feign.Request;
import feign.RetryableException;

import java.util.Date;

/**
 * CommonRetryableException
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2024/1/17
 */
public class CommonRetryableException extends RetryableException {
    private Exception exception;

    public CommonRetryableException(int status, String message, Request.HttpMethod httpMethod, Throwable cause,
            Date retryAfter, Request request) {
        super(status, message, httpMethod, cause, retryAfter, request);
    }

    public CommonRetryableException(int status, String message, Request.HttpMethod httpMethod, Date retryAfter,
            Request request) {
        super(status, message, httpMethod, retryAfter, request);
    }

    @Override
    public Date retryAfter() {
        return super.retryAfter();
    }

    @Override
    public Request.HttpMethod method() {
        return super.method();
    }

    public Exception getException() {
        return exception;
    }

    public void setException(Exception exception) {
        this.exception = exception;
    }
}
