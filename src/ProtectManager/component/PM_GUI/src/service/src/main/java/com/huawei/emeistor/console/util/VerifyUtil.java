/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import java.util.Collection;
import java.util.Map;

/**
 * 本类用于对常见的集合、String、Map、一维数组、二维数组等进行校验
 *
 * @author w90002860
 * @version V100R001C00
 * @since 2020-05-25
 */
public final class VerifyUtil {
    private VerifyUtil() {
    }

    /**
     * 判断Collection子类元素是否为空集合
     *
     * @param collection 集合对象
     * @return boolean [true：空，false：非空]
     */
    public static boolean isEmpty(Collection<?> collection) {
        return (collection == null) || (collection.isEmpty());
    }

    /**
     * 判断对象是否为空
     *
     * @param obj 对象
     * @return boolean [true：空，false：非空]
     */
    public static boolean isEmpty(Object obj) {
        return (obj == null) || (obj instanceof String && isEmpty((String) obj));
    }

    /**
     * 判断Map对象是否为空
     *
     * @param map Map对象
     * @return boolean [true：空，false：非空]
     */
    public static boolean isEmpty(Map<?, ?> map) {
        return (map == null) || (map.isEmpty());
    }

    /**
     * 判断String对象是否为null、空字符串、空格串
     *
     * @param string 字符串
     * @return boolean [true：空，false：非空]
     */
    public static boolean isEmpty(String string) {
        return (string == null) || (string.trim().isEmpty());
    }

    /**
     * 是否是数组对象
     *
     * @param value 值
     * @return 是否是数组对象
     */
    public static boolean isArrayObject(Object value) {
        if (value == null) {
            return false;
        }
        return value instanceof Collection || value.getClass().isArray();
    }
}