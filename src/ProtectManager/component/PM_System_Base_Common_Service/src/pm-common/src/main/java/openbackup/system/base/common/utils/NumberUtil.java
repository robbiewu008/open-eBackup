/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 数字工具类
 *
 * @author w90002860
 * @version V100R001C00
 * @since 2019-11-01
 */
public final class NumberUtil {
    private static final Logger logger = LoggerFactory.getLogger(NumberUtil.class);

    private NumberUtil() {}

    /**
     * 定义转换时的进制
     *
     * @since 2019-11-01
     */
    public enum RADIX {
        /**
         * 10进制
         */
        TEN(10);

        private final int value;

        RADIX(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * Object类型转换为Double类型，如果类型为空，返回0
     *
     * @param obj 对象
     * @return Double
     */
    public static Double parseDouble(Object obj) {
        double data = 0.0D;
        if (obj != null) {
            try {
                data = Double.parseDouble(obj.toString());
            } catch (NumberFormatException e) {
                logger.error("Converting failed, return default value: {}", data);
                data = 0.0D;
            }
        }
        return data;
    }

    /**
     * 转换对象成Long
     *
     * @param object       待转换的对象
     * @param defaultValue 转换失败时使用此默认值
     * @return Long
     */
    public static Long convertToLong(Object object, long defaultValue) {
        if (object == null) {
            return defaultValue;
        }
        long result;
        try {
            result = Long.parseLong(String.valueOf(object));
        } catch (NumberFormatException e) {
            logger.error("Converting failed, return default value: {}", defaultValue);
            result = defaultValue;
        }

        return result;
    }

    /**
     * 转换对象成Integer
     *
     * @param object       object
     * @param radix        指定转换的进制
     * @param defaultValue 转换失败时使用此默认值
     * @return Integer Integer
     */
    public static Integer convertToInteger(Object object, RADIX radix, int defaultValue) {
        if (object == null) {
            return defaultValue;
        }

        int result;
        try {
            result = Integer.valueOf(String.valueOf(object), radix.getValue());
        } catch (NumberFormatException e) {
            logger.error("Converting failed, return default value: {}", defaultValue);
            result = defaultValue;
        }

        return result;
    }

    /**
     * 转换对象成Long
     *
     * @param object        待转换的对象
     * @param failException 转换失败时抛出的指定异常
     * @return Long 转换之后的值
     */
    public static Long convertToLong(Object object, EmeiStorDefaultExceptionHandler failException) {
        if (object == null) {
            throw failException;
        }

        try {
            return Long.valueOf(String.valueOf(object));
        } catch (NumberFormatException e) {
            throw failException;
        }
    }

    /**
     * 转换对象成Integer
     *
     * @param object        object
     * @param radix         指定转换的进制
     * @param failException 转换失败时抛出的指定异常
     * @return Integer 转换之后的值
     */
    public static Integer convertToInteger(Object object, RADIX radix, EmeiStorDefaultExceptionHandler failException) {
        if (object == null) {
            throw failException;
        }

        try {
            return Integer.valueOf(String.valueOf(object), radix.getValue());
        } catch (NumberFormatException e) {
            throw failException;
        }
    }

    /**
     * 转换对象成Integer
     *
     * @param object        object
     * @param failException 转换失败时抛出的指定异常
     * @return Integer 转换之后的值
     */
    public static Integer convertToInteger(Object object, EmeiStorDefaultExceptionHandler failException) {
        return convertToInteger(object, RADIX.TEN, failException);
    }

    /**
     * 转换对象成Double
     *
     * @param object        object
     * @param failException 转换失败时抛出的指定异常
     * @return Double 转换之后的值
     */
    public static Double convertToDouble(Object object, EmeiStorDefaultExceptionHandler failException) {
        if (object == null) {
            throw failException;
        }

        try {
            return Double.valueOf(String.valueOf(object));
        } catch (NumberFormatException e) {
            throw failException;
        }
    }

    /**
     * 转换对象成Float
     *
     * @param object        object
     * @param failException 转换失败时抛出的指定异常
     * @return Float 转换之后的值
     */
    public static Float convertToFloat(Object object, EmeiStorDefaultExceptionHandler failException) {
        if (object == null) {
            throw failException;
        }

        try {
            return Float.valueOf(String.valueOf(object));
        } catch (NumberFormatException e) {
            throw failException;
        }
    }

    /**
     * 转换对象成Long
     * <p>
     * 如果转换失败抛出参数错误异常
     *
     * @param object 待转换的对象
     * @return Long 转换之后的值
     */
    public static Long convertToLong(Object object) {
        return convertToLong(object, new EmeiStorDefaultExceptionHandler(null));
    }

    /**
     * 转换对象成Integer
     * <p>
     * 如果转换失败抛出参数错误异常
     *
     * @param object object
     * @return Integer 转换之后的值
     */
    public static Integer convertToInteger(Object object) {
        return convertToInteger(object, new EmeiStorDefaultExceptionHandler(null));
    }
}
