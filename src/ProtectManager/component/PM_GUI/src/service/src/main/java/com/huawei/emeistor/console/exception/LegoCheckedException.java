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

import lombok.Getter;
import lombok.Setter;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;

import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutionException;

/**
 * 系统异常，用户可以直接介入修正异常信息的定义为LegoCheckedException 如：用户登录时用户名/密码错误
 *
 * @since 2019-10-31
 */
@Getter
@Setter
public class LegoCheckedException extends RuntimeException {
    /**
     * ExceptionLogDecorator I18N_PREFIX
     */
    protected static final String I18N_PREFIX = "i18n:";

    private static final Logger logger = LoggerFactory.getLogger(LegoCheckedException.class);

    private static final long serialVersionUID = 9223257323548528435L;

    // 该List在转换异常方法(cast)中,声明如下的异常,可以获取底层的LegoCheckedException,即获取到声明异常.getCause()
    // The List declares the following exceptions in the cast method to get the underlying Lego Checked Exception,
    // which is to get the declared exception. getCause ()
    private static final List<Class<? extends Throwable>> EXCEPTIONS = Arrays.asList(ExecutionException.class,
        InvocationTargetException.class);

    private long errorCode;

    private String[] parameters;

    private Object accessoryResult;

    private HttpStatus httpStatus;

    private String emeiStorErrCode;

    /**
     * 默认构造函数
     *
     * @param message message
     * @param cause cause
     */
    public LegoCheckedException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * 默认构造函数
     *
     * @param message message
     */
    public LegoCheckedException(String message) {
        super(message);
    }

    /**
     * LegoChecked构造体
     *
     * @param httpStatus 错误状态
     * @param errorCode 错误码
     * @param info 错误信息
     */
    public LegoCheckedException(HttpStatus httpStatus, String errorCode, String info) {
        super(info);
        this.emeiStorErrCode = errorCode;
        this.httpStatus = httpStatus;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     */
    public LegoCheckedException(long errorCode) {
        super("Code: " + errorCode);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     */
    public LegoCheckedException(long errorCode, String[] parameter) {
        super("Code: " + errorCode);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param cause 具体的异常
     */
    public LegoCheckedException(long errorCode, String[] parameter, Throwable cause) {
        super("Code: " + errorCode, cause);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param message message
     */
    public LegoCheckedException(long errorCode, String[] parameter, String message) {
        super(message);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 待logger参数的异常构造函数<br/>
     * constructor with logger
     *
     * @param errorCode error code
     * @param message message
     * @param logger logger
     * @param parameter parameters
     */
    public LegoCheckedException(long errorCode, String message, Logger logger, String... parameter) {
        this(errorCode, parameter, message);
        logger.error(message, this);
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param ex ex
     */
    public LegoCheckedException(long errorCode, Throwable ex) {
        super("Code: " + errorCode, ex);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param info info
     */
    public LegoCheckedException(long errorCode, String info) {
        super(info);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param info info
     * @param ex ex
     */
    public LegoCheckedException(long errorCode, String info, Throwable ex) {
        super(info, ex);
        this.errorCode = errorCode;
    }

    /**
     * 异常转换
     * 用于抛出异常的错误转换，如果内部抛出错误码就使用内部抛出的错误码，如果没有抛出，就统一使用操作失败错误码
     *
     * @param throwable 异常信息
     * @return 转换结果
     */
    public static LegoCheckedException cast(Throwable throwable) {
        return new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, throwable);
    }
}
