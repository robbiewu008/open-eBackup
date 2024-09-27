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
package com.huawei.emeistor.console.util;

import org.apache.commons.lang3.StringEscapeUtils;

import java.io.FileNotFoundException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.UndeclaredThrowableException;
import java.net.BindException;
import java.security.GeneralSecurityException;
import java.security.acl.NotOwnerException;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.ConcurrentModificationException;
import java.util.List;
import java.util.MissingResourceException;
import java.util.jar.JarException;

import javax.naming.InsufficientResourcesException;

/**
 * 异常工具类
 *
 * @author l90002863
 * @version [OceanStor ReplicationDirector V100R005C00, 2015-7-6]
 * @since 2019-11-01
 */
public final class ExceptionUtil {
    /**
     * 异常堆栈中类若为指定类路径下的类则输出堆栈信息
     */
    private static final String[] VALID_PACKAGE_PREFIX_NAMES = new String[] {"com.huawei"};

    /**
     * 敏感异常类型
     */
    private static final List<Class<? extends Throwable>> SPECIAL_EXCEPTION_CLASSES = Arrays.asList(
        SecurityException.class, GeneralSecurityException.class, ClassNotFoundException.class,
        FileNotFoundException.class, JarException.class, MissingResourceException.class, NotOwnerException.class,
        ConcurrentModificationException.class, InsufficientResourcesException.class, BindException.class,
        OutOfMemoryError.class, StackOverflowError.class, SQLException.class);

    private ExceptionUtil() {
    }

    /**
     * 判断异常是否为敏感异常
     *
     * @param exception 异常对象
     * @return boolean 是否为敏感异常
     */
    public static boolean isSpecialException(Throwable exception) {
        for (Class<? extends Throwable> clazz : SPECIAL_EXCEPTION_CLASSES) {
            if (clazz.isInstance(exception)) {
                return true;
            }
        }

        return false;
    }

    /**
     * 获取错误信息
     * 1、对于敏感异常不打印异常详情，仅输出异常名称
     * 2、对于所有堆栈路径仅输出个异常的第一条以及以指定类型类的路径
     * 例如：
     * java.io.FileNotFoundException: ***
     * at java.io.FileInputStream.open(Native Method)
     * at com.huawei.ism.Main.test(Main.java:33)
     *
     * @param exception 异常
     * @return String 错误信息
     */
    public static Exception getErrorMessage(Throwable exception) {
        StringBuilder strBuilder = new StringBuilder();
        if (exception == null) {
            return new Exception(strBuilder.toString());
        }

        strBuilder.append(System.lineSeparator());

        Throwable throwable = exception;
        int index = 0;
        do {
            if (index > 0) {
                strBuilder.append("Caused by: ");
            }

            String message = throwable.getMessage();
            if (isSpecialException(throwable)) {
                message = "***";
            }
            strBuilder.append(throwable.getClass().getName()).append(": ").append(StringEscapeUtils.escapeJava(message))
                    .append(System.lineSeparator());
            StackTraceElement[] elements = throwable.getStackTrace();
            retrieveStackTraces(elements, strBuilder);
            throwable = throwable.getCause();
            index++;
        } while (throwable != null);
        Exception newException = new Exception(strBuilder.toString());
        newException.setStackTrace(new StackTraceElement[0]);
        return newException;
    }

    /**
     * 提取异常堆栈信息
     *
     * @param elements   异常堆栈
     * @param strBuilder 用于保存提取的堆栈信息
     */
    private static void retrieveStackTraces(StackTraceElement[] elements, StringBuilder strBuilder) {
        int index = 0;
        for (StackTraceElement element : elements) {
            index++;
            String clsName = element.getClassName();
            if (index == 1 || isValidClass(clsName)) {
                strBuilder.append("   at   ").append(element).append(System.lineSeparator());
            }
        }
    }

    /**
     * 判断指定类的全路径名称是否为已指定有效的包名前缀开头
     *
     * @param clsName 类的全路径名
     * @return boolean 类的全路径名称是否为已指定有效的包名前缀开头
     */
    public static boolean isValidClass(String clsName) {
        for (String validClsName : VALID_PACKAGE_PREFIX_NAMES) {
            if (clsName.startsWith(validClsName)) {
                return true;
            }
        }

        return false;
    }

    /**
     * look for the special type error
     *
     * @param throwable throwable
     * @param type      the special type
     * @param <T>       template type
     * @return result
     */
    public static <T extends Throwable> T lookFor(Throwable throwable, Class<T> type) {
        return lookFor(throwable, type, null);
    }

    /**
     * look for the special type error
     *
     * @param throwable throwable
     * @param type      the special type
     * @param exception exception
     * @param <T>       template type
     * @return result
     */
    public static <T extends Throwable> T lookFor(Throwable throwable, Class<T> type, T exception) {
        Throwable error = throwable;
        do {
            if (error == null) {
                return exception;
            }
            if (type.isInstance(error)) {
                return type.cast(error);
            }
            if (error instanceof UndeclaredThrowableException) {
                T result = lookFor(((UndeclaredThrowableException) error).getUndeclaredThrowable(), type);
                if (result != null) {
                    return result;
                }
            }
            if (error instanceof InvocationTargetException) {
                T result = lookFor(((InvocationTargetException) error).getTargetException(), type);
                if (result != null) {
                    return result;
                }
            }
            Throwable cause = error.getCause();
            if (error == cause) {
                return exception;
            }
            error = cause;
        } while (true);
    }
}
