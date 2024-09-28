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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Stringify Converter
 *
 */
@Component
@Slf4j
public class StringifyConverter implements DataConverter {
    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "string";
    }

    /**
     * List<String>的数据转化
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(StringifyObject::new).collect(Collectors.toList());
    }

    /**
     * I18n Object
     */
    public static class StringifyObject extends HashMap<String, StringifyObject> implements StringProperties {
        private final Object object;

        /**
         * constructor
         *
         * @param object object
         */
        public StringifyObject(Object object) {
            this.object = object;
        }

        private static StringifyObject create(Object object) {
            if (object instanceof StringifyObject) {
                return (StringifyObject) object;
            }
            return new StringifyObject(object);
        }

        /**
         * get i18n object
         *
         * @param key key
         * @return i18n object
         */
        @Override
        public StringifyObject get(Object key) {
            if (object == null) {
                return this;
            }
            if (object instanceof Collection) {
                Collection<?> collection = (Collection<?>) object;
                List<StringifyObject> items = collection.stream()
                    .map(StringifyObject::create)
                    .map(item -> item.get(key))
                    .collect(Collectors.toList());
                return new StringifyObject(items);
            }
            Class<?> clazz = object.getClass();
            Method method;
            try {
                method = clazz.getMethod(key.toString());
            } catch (NoSuchMethodException e) {
                log.trace("no found the method: {}", key, ExceptionUtil.getErrorMessage(e));
                return this;
            }
            boolean isAccessible = method.isAccessible();
            Object result;
            try {
                method.setAccessible(true);
                result = method.invoke(object);
            } catch (IllegalAccessException | InvocationTargetException e) {
                log.trace("the method: {} is not invokable", key, ExceptionUtil.getErrorMessage(e));
                return this;
            } finally {
                method.setAccessible(isAccessible);
            }
            return new StringifyObject(result);
        }

        /**
         * cast as i18n key string
         *
         * @return i18n key string
         */
        @Override
        public String toString() {
            if (object == null) {
                return null;
            }
            if (object instanceof Collection) {
                Collection<?> collection = (Collection<?>) object;
                return collection.stream().map(String::valueOf).collect(Collectors.joining(" "));
            }
            return object.toString();
        }
    }
}
