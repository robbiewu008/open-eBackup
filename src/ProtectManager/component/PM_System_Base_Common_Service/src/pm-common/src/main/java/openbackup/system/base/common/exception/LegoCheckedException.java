/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.exception;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.IsmErrorCodeConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;

import org.slf4j.Logger;

import java.util.Arrays;

/**
 * 系统异常，用户可以直接介入修正异常信息的定义为LegoCheckedException
 *
 * @author w00448845
 * @version @version [CDM Integrated machine]
 * @since 2019-10-28
 */
public class LegoCheckedException extends RuntimeException {
    /**
     * I18N_PREFIX
     */
    protected static final String I18N_PREFIX = "i18n:";

    private static final long serialVersionUID = 9223257323548528435L;

    // 该List在转换异常方法(cast)中,声明如下的异常,可以获取底层的LegoCheckedException,即获取到声明异常.getCause()
    // The List declares the following exceptions in the cast method to get the underlying Lego Checked Exception,
    // which is to get the declared exception. getCause ()

    private long errorCode = CommonErrorCode.OPERATION_FAILED;

    private String[] parameters;

    private Object accessoryResult;

    private boolean isRetryable;

    /**
     * 默认构造函数
     *
     * @param message message
     * @param cause   cause
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
     * 默认构造函数
     *
     * @param errorCode errorCode
     */
    @Deprecated
    public LegoCheckedException(long errorCode) {
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     */
    @Deprecated
    public LegoCheckedException(long errorCode, String[] parameter) {
        this(errorCode);
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param cause     具体的异常
     */
    public LegoCheckedException(long errorCode, String[] parameter, Throwable cause) {
        super(cause.getMessage(), cause);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param message   message
     */
    public LegoCheckedException(long errorCode, String[] parameter, String message) {
        super(message);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param message message
     * @param cause cause
     */
    public LegoCheckedException(long errorCode, String[] parameter, String message, Throwable cause) {
        super(message, cause);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 待logger参数的异常构造函数<br/>
     * constructor with logger
     *
     * @param errorCode error code
     * @param message   message
     * @param logger    logger
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
     * @param throwable throwable
     */
    public LegoCheckedException(long errorCode, Throwable throwable) {
        super(throwable.getMessage(), throwable);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param info      info
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
     * @param throwable throwable
     */
    public LegoCheckedException(long errorCode, String info, Throwable throwable) {
        super(info, throwable);
        this.errorCode = errorCode;
    }

    public boolean isRetryable() {
        return isRetryable;
    }

    public void setRetryable(boolean isRetryable) {
        this.isRetryable = isRetryable;
    }

    public Object getAccessoryResult() {
        return accessoryResult;
    }

    public void setAccessoryResult(Object result) {
        this.accessoryResult = result;
    }

    public long getErrorCode() {
        return errorCode;
    }

    public String[] getParameters() {
        return parameters;
    }

    /**
     * 错误详细信息前缀，用于抛错时前台显示
     *
     * @return 错误详情
     */
    public String getErrorMessageKey() {
        return ErrorCodeConstant.ERROR_CODE_PREFIX + errorCode;
    }

    /**
     * rethrow method
     *
     * @param throwable throwable
     * @return LegoCheckedException
     */
    public static LegoCheckedException rethrow(Throwable throwable) {
        return rethrow(throwable, IsmErrorCodeConstant.SYSTEM_EXCEPTION);
    }

    /**
     * rethrow method
     *
     * @param throwable throwable
     * @param errorCode error code
     * @param params    params
     * @return LegoCheckedException
     */
    public static LegoCheckedException rethrow(Throwable throwable, long errorCode, String... params) {
        throw cast(throwable, errorCode, params);
    }

    /**
     * 异常转换
     * 用于抛出异常的错误转换，如果内部抛出错误码就使用内部抛出的错误码，如果没有抛出，就使用统一规定的错误码
     *
     * @param throwable  异常信息
     * @param errorCode 错误码
     * @param params    错误参数
     * @return 转换结果
     */
    public static LegoCheckedException cast(Throwable throwable, long errorCode, String... params) {
        LegoCheckedException exception = ExceptionUtil.lookFor(throwable, LegoCheckedException.class);
        if (exception != null) {
            return exception;
        }
        return new LegoCheckedException(errorCode, params, throwable);
    }

    /**
     * 异常转换
     * 用于抛出异常的错误转换，如果内部抛出错误码就使用内部抛出的错误码，如果没有抛出，就统一使用操作失败错误码
     *
     * @param throwable  异常信息
     * @return 转换结果
     */
    public static LegoCheckedException cast(Throwable throwable) {
        return cast(throwable, CommonErrorCode.OPERATION_FAILED);
    }

    /**
     * 国际化方式，形式类似于：i18n:lego.err.errorCode
     *
     * @return 国际化后的字符串
     */
    public String i18n() {
        JSONArray array = new JSONArray();
        array.add("lego.err." + errorCode);
        if (parameters != null) {
            array.addAll(Arrays.asList(parameters));
        }
        return I18N_PREFIX + array;
    }
}