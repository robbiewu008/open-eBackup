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
package openbackup.system.base.util;

import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.lambda.Lambda;

import com.google.gson.Gson;

import org.springframework.beans.BeanUtils;

import java.beans.PropertyDescriptor;
import java.util.List;
import java.util.function.Supplier;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Bean Tools
 *
 */
public class BeanTools {
    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param factory factory
     * @param <E> template type E
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, Supplier<T> factory) {
        return copy(source, factory, new String[0]);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param factory factory
     * @param properties properties
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, Supplier<T> factory, String[] properties) {
        return copy(source, factory.get(), properties);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param factory factory
     * @param isInclude include flag
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, Supplier<T> factory, boolean isInclude) {
        return copy(source, factory, isInclude, new String[0]);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param factory factory
     * @param isInclude include flag
     * @param properties ignore properties
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, Supplier<T> factory, boolean isInclude, String[] properties) {
        return copy(source, factory.get(), isInclude, properties);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param target target
     * @param <E> template type E
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, T target) {
        return copy(source, target, new String[0]);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param target target
     * @param properties properties
     * @param <E> template type E
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, T target, String[] properties) {
        return copy(source, target, true, properties);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param target target
     * @param isInclude include flag
     * @param <E> template type E
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, T target, boolean isInclude) {
        return copy(source, target, isInclude, new String[0]);
    }

    /**
     * copy properties of source object to target object
     *
     * @param source source object
     * @param target target
     * @param isInclude include flag
     * @param properties properties
     * @param <E> template type E
     * @param <T> template type T
     * @return result object
     */
    public static <E, T> T copy(E source, T target, boolean isInclude, String[] properties) {
        if (source == null || target == null) {
            return null;
        }
        if (properties == null || properties.length == 0) {
            BeanUtils.copyProperties(source, target);
            return target;
        }
        List<String> fields = CollectionUtils.nonNullList(properties);
        List<String> ignores;
        if (isInclude) {
            ignores =
                    Stream.of(BeanUtils.getPropertyDescriptors(source.getClass()))
                            .map(PropertyDescriptor::getName)
                            .filter(name -> !fields.contains(name))
                            .collect(Collectors.toList());
        } else {
            ignores = fields;
        }
        BeanUtils.copyProperties(source, target, ignores.toArray(new String[0]));
        return target;
    }

    /**
     * clone method
     *
     * @param object object
     * @param <T> template type T
     * @return clone object
     */
    public static <T> T clone(T object) {
        return clone(object, new String[0]);
    }

    /**
     * clone method
     *
     * @param object object
     * @param fields fields
     * @param <T> template type T
     * @return clone object
     */
    public static <T> T clone(T object, String[] fields) {
        return copy(object, () -> createObjectWithSameType(object), fields);
    }

    /**
     * deep clone method
     *
     * @param object object
     * @param <T> template type T
     * @return clone object
     */
    public static <T> T deepClone(T object) {
        Gson gson = new Gson();
        String jsonObject = gson.toJson(object);
        return (T) gson.fromJson(jsonObject, object.getClass());
    }

    private static <T> T createObjectWithSameType(T object) {
        if (object == null) {
            return null;
        }
        try {
            @SuppressWarnings("unchecked")
            Class<T> clazz = (Class<T>) object.getClass();
            return clazz.newInstance();
        } catch (InstantiationException | IllegalAccessException e) {
            throw new IllegalArgumentException("create instance failed", e);
        }
    }

    /**
     * 属性保持函数，在代码逻辑执行完成后，对属性进行恢复。
     *
     * @param supplier supplier
     * @param object object
     * @param <E> template type E
     * @param <T> template type T
     * @return result
     */
    public static <E, T> T hold(Supplier<T> supplier, E object) {
        return hold(supplier, object, new String[0]);
    }

    /**
     * 属性保持函数，在代码逻辑执行完成后，对属性进行恢复。
     *
     * @param supplier supplier
     * @param object object
     * @param fields fields
     * @param <E> template type E
     * @param <T> template type T
     * @return result
     */
    public static <E, T> T hold(Supplier<T> supplier, E object, String[] fields) {
        Object backup = clone(object, fields);
        try {
            return supplier.get();
        } finally {
            copy(backup, object, fields);
        }
    }

    /**
     * 属性保持函数，在代码逻辑执行完成后，对属性进行恢复。
     *
     * @param runnable runnable
     * @param object object
     * @param <E> template type E
     */
    public static <E> void hold(Runnable runnable, E object) {
        hold(runnable, object, new String[0]);
    }

    /**
     * 属性保持函数，在代码逻辑执行完成后，对属性进行恢复。
     *
     * @param runnable runnable
     * @param object object
     * @param fields fields
     * @param <E> template type E
     */
    public static <E> void hold(Runnable runnable, E object, String[] fields) {
        hold(Lambda.supplier(runnable), object, fields);
    }
}
