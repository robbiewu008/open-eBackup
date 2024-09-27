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

import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.HashMap;
import java.util.stream.Collectors;

/**
 * I18n Converter
 *
 * @author l00272247
 * @since 2021-01-22
 */
@Component
public class I18nConverter implements DataConverter {
    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "i18n";
    }

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(Object::toString).map(I18nObject::new).collect(Collectors.toList());
    }

    /**
     * I18n Object
     */
    public static class I18nObject extends HashMap<String, I18nObject> implements StringProperties {
        private final String prefix;
        private final String suffix;

        /**
         * constructor
         *
         * @param suffix suffix
         */
        public I18nObject(String suffix) {
            this(null, suffix);
        }

        /**
         * constructor
         *
         * @param prefix prefix
         * @param suffix suffix
         */
        public I18nObject(String prefix, String suffix) {
            super();
            this.prefix = prefix;
            this.suffix = suffix;
        }

        /**
         * get i18n object
         *
         * @param key key
         * @return i18n object
         */
        @Override
        public I18nObject get(Object key) {
            String fullPrefix = prefix != null ? prefix + "_" + key : key.toString();
            return new I18nObject(fullPrefix, suffix);
        }

        /**
         * cast as i18n key string
         *
         * @return i18n key string
         */
        @Override
        public String toString() {
            String key = prefix != null ? prefix + "_" + suffix : suffix;
            return key + "_label";
        }
    }
}
