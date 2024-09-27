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
package openbackup.system.base.common.exception;

/**
 * 系统异常，所有不需要用户直接介入的异常都定义为LegoUncheckedException
 *
 * @author s90004407
 * @version [Lego V100R002C10, 2014-12-17]
 * @since 2019-10-31
 */
public class LegoUncheckedException extends RuntimeException { // ExceptionLogDecorator
    private static final long serialVersionUID = 5625178995893882625L;

    private long errorCode;

    /**
     * 默认构造函数
     *
     * @param message message
     * @param cause   cause
     */
    public LegoUncheckedException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * 默认构造函数
     *
     * @param message message
     */
    public LegoUncheckedException(String message) {
        super(message);
    }

    /**
     * 默认构造函数
     *
     * @param errorCode message
     */
    public LegoUncheckedException(long errorCode) {
        this.errorCode = errorCode;
    }

    /**
     * 构造函数
     *
     * @param errorCode 错误码
     * @param message 错误信息
     */
    public LegoUncheckedException(long errorCode, String message) {
        super(message);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode message
     * @param cause     cause
     */
    public LegoUncheckedException(long errorCode, Throwable cause) {
        super(cause);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param cause message
     */
    public LegoUncheckedException(Throwable cause) {
        super(cause);
    }

    public long getErrorCode() {
        return errorCode;
    }
}
