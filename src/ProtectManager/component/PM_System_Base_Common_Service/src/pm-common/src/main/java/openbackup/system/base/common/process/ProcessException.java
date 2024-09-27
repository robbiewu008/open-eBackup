/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.process;

import java.util.Locale;

/**
 * 处理描述
 *
 * @author w00493811
 * @since 2021-08-10
 */
public class ProcessException extends Exception {
    /**
     * 默认构造函数
     *
     * @param message 信息
     */
    public ProcessException(String message) {
        super(message);
    }

    /**
     * 默认构造函数
     *
     * @param message 信息
     * @param parameters 参数
     */
    public ProcessException(String message, Object... parameters) {
        super(String.format(Locale.ENGLISH, message, parameters));
    }

    /**
     * 默认构造函数
     *
     * @param cause 异常
     * @param message 信息
     * @param parameters 参数
     */
    public ProcessException(Throwable cause, String message, Object... parameters) {
        super(String.format(Locale.ENGLISH, message, parameters), cause, false, false);
    }
}