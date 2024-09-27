/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.lang.reflect.AccessibleObject;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Locale;

/**
 * 反射工具
 *
 * @author l00272247
 * @version [V200R001C30, 2018-07-10]
 * @since 2019-11-01
 */
public final class ReflectUtil {
    private static final Logger logger = LoggerFactory.getLogger(ReflectUtil.class);

    private ReflectUtil() {
    }

    /**
     * 根据字段名称获取getter方法
     *
     * @param clazz     类对象
     * @param fieldName 字段名称
     * @param fieldType 字段类型
     * @return getter方法
     */
    public static Method getter(Class<?> clazz, String fieldName, Class<?> fieldType) {
        String methodName = "get" + fieldName.substring(0, 1).toUpperCase(Locale.US) + fieldName.substring(1);
        return method(clazz, methodName, fieldType);
    }

    /**
     * 根据字段名称获取setter方法
     *
     * @param clazz     类对象
     * @param fieldName 字段名称
     * @param fieldType 字段类型
     * @return setter方法
     */
    public static Method setter(Class<?> clazz, String fieldName, Class<?> fieldType) {
        String methodName = "set" + fieldName.substring(0, 1).toUpperCase(Locale.US) + fieldName.substring(1);
        return method(clazz, methodName, Void.class, fieldType);
    }

    /**
     * 根据字段名称获取方法
     *
     * @param clazz    类对象
     * @param name     字段名称
     * @param retType  字段类型
     * @param argTypes 参数
     * @return 方法
     */
    public static Method method(Class<?> clazz, String name, Class<?> retType, Class<?>... argTypes) {
        if (clazz == null) {
            return null;
        }
        try {
            Method method = clazz.getDeclaredMethod(name, argTypes);
            return retType.isAssignableFrom(method.getReturnType()) ? method : null;
        } catch (NoSuchMethodException | SecurityException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        }
        return method(clazz.getSuperclass(), name, retType, argTypes);
    }

    /**
     * 根据字段名称获取对象
     *
     * @param object 类对象
     * @param method 方法
     * @return 方法
     */
    public static Object get(Object object, Method method) {
        if (method == null) {
            return null;
        }
        boolean isNeedRestore = false;
        try {
            isNeedRestore = modifyAccessible(method);
            return method.invoke(object);
        } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        } finally {
            restoreAccessible(method, isNeedRestore);
        }
        return null;
    }

    /**
     * 根据字段名称获取字段
     *
     * @param object 类对象
     * @param field     字段
     * @return 方法
     */
    public static Object get(Object object, Field field) {
        if (field == null) {
            return null;
        }
        boolean isNeedRestore = false;
        try {
            isNeedRestore = modifyAccessible(field);
            return field.get(object);
        } catch (IllegalArgumentException | IllegalAccessException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        } finally {
            restoreAccessible(field, isNeedRestore);
        }
        return null;
    }

    /**
     * 根据字段名称设置字段
     *
     * @param object 类对象
     * @param method      方法
     * @param value  值
     * @return 成功
     */
    public static boolean set(Object object, Method method, Object value) {
        if (method == null) {
            return false;
        }
        boolean isNeedRestore = false;
        try {
            isNeedRestore = modifyAccessible(method);
            method.invoke(object, value);
            return true;
        } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        } finally {
            restoreAccessible(method, isNeedRestore);
        }
        return false;
    }

    /**
     * 根据字段名称设置字段
     *
     * @param object 类对象
     * @param field      属性
     * @param value  值
     * @return 成功
     */
    public static boolean set(Object object, Field field, Object value) {
        if (field == null) {
            return false;
        }
        boolean isNeedRestore = false;
        try {
            isNeedRestore = modifyAccessible(field);
            field.set(object, value);
            return true;
        } catch (IllegalAccessException | IllegalArgumentException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        } finally {
            restoreAccessible(field, isNeedRestore);
        }
        return false;
    }

    private static boolean modifyAccessible(AccessibleObject ao) {
        if (ao.isAccessible()) {
            return false;
        }
        ao.setAccessible(true);
        return true;
    }

    private static void restoreAccessible(AccessibleObject ao, boolean isNeedRestore) {
        if (isNeedRestore) {
            ao.setAccessible(false);
        }
    }

    /**
     * 获取属性
     *
     * @param clazz     类
     * @param name      名字
     * @param fieldType 属性type
     * @return 属性
     */
    public static Field field(Class<?> clazz, String name, Class<?> fieldType) {
        if (clazz == null) {
            return null;
        }
        try {
            Field field = clazz.getDeclaredField(name);
            return fieldType.isAssignableFrom(field.getType()) ? field : null;
        } catch (NoSuchFieldException | SecurityException e) {
            logger.error(e.getMessage(), ExceptionUtil.getErrorMessage(e));
        }
        return field(clazz.getSuperclass(), name, fieldType);
    }
}
