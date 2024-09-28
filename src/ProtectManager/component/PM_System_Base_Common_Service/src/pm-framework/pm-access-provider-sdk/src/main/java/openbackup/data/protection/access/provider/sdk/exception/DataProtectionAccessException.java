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
package openbackup.data.protection.access.provider.sdk.exception;

/**
 * 数据保护接入异常
 *
 */
public class DataProtectionAccessException extends RuntimeException {
    private long errorCode;
    private String[] parameters;

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     */
    @Deprecated
    public DataProtectionAccessException(long errorCode, String[] parameter) {
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param cause     具体的异常
     */
    public DataProtectionAccessException(long errorCode, String[] parameter, Throwable cause) {
        super(cause.getMessage(), cause);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 构造函数
     *
     * @param errorCode 错误码
     * @param parameter 参数
     * @param message 错误消息
     */
    public DataProtectionAccessException(long errorCode, String[] parameter, String message) {
        super(message);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    public long getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(long errorCode) {
        this.errorCode = errorCode;
    }

    public String[] getParameters() {
        return parameters;
    }

    public void setParameters(String[] parameters) {
        this.parameters = parameters;
    }

    /**
     * raise exception
     */
    public void raise() {
        throw this;
    }
}
