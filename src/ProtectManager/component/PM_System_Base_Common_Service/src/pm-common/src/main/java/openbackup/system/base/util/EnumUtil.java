/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.util;

import java.util.Objects;
import java.util.function.Function;

/**
 * Enum Util
 *
 * @author l00272247
 * @since 2020-10-14
 */
public class EnumUtil {
    /**
     * get enum by value
     *
     * @param clazz  clazz
     * @param getter getter
     * @param value  value
     * @param <T>    T
     * @param <V>    V
     * @return enum
     */
    public static <T extends Enum, V> T get(Class<T> clazz, Function<T, V> getter, V value) {
        return get(clazz, getter, value, true);
    }

    /**
     * get enum by value
     *
     * @param clazz  clazz
     * @param getter getter
     * @param value  value
     * @param isStrict strict
     * @param <T>    T
     * @param <V>    V
     * @return enum
     */
    public static <T extends Enum, V> T get(Class<T> clazz, Function<T, V> getter, V value, boolean isStrict) {
        return get(clazz, getter, value, isStrict, false);
    }

    /**
     * get enum by value
     *
     * @param clazz  clazz
     * @param getter getter
     * @param value  value
     * @param isStrict isStrict mode
     * @param isSilent silent mode
     * @param <T>    T
     * @param <V>    V
     * @return enum
     */
    public static <T extends Enum, V> T get(
            Class<T> clazz, Function<T, V> getter, V value, boolean isStrict, boolean isSilent) {
        T[] constants = clazz.getEnumConstants();
        for (T constant : constants) {
            V object = getter.apply(constant);
            boolean isEquals;
            if (isStrict) {
                isEquals = Objects.equals(object, value);
            } else if (object != null && value != null) {
                isEquals = object.toString().equalsIgnoreCase(value.toString());
            } else {
                isEquals = object == value;
            }
            if (isEquals) {
                return constant;
            }
        }
        if (isSilent) {
            return null;
        }
        throw new IllegalArgumentException("No enum constant " + clazz.getCanonicalName() + "." + value);
    }
}
