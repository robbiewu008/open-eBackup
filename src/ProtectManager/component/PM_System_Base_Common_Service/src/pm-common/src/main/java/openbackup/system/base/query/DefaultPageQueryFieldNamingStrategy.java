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
package openbackup.system.base.query;

import openbackup.system.base.common.utils.ExceptionUtil;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.extern.slf4j.Slf4j;

import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.stereotype.Component;

import java.lang.reflect.Field;

/**
 * Page Query Field Naming Snake Case Strategy
 *
 * @author l00272247
 * @since 2021-06-03
 */
@Component(Pagination.DEFAULT_FIELD_NAMING_STRATEGY)
@Slf4j
public class DefaultPageQueryFieldNamingStrategy implements FieldNamingStrategy {
    /**
     * translate
     *
     * @param type type
     * @param field field
     * @return result
     */
    @Override
    public String translate(Class<?> type, Field field) {
        JsonProperty property = field.getAnnotation(JsonProperty.class);
        if (property != null) {
            return property.value();
        }
        PropertyNamingStrategy.PropertyNamingStrategyBase strategy = getPropertyNamingStrategy(type);
        if (strategy == null) {
            return field.getName();
        } else {
            return strategy.translate(field.getName());
        }
    }

    /**
     * get Property Naming Strategy
     *
     * @param type type
     * @return result
     */
    protected PropertyNamingStrategy.PropertyNamingStrategyBase getPropertyNamingStrategy(Class<?> type) {
        JsonNaming jsonNaming = AnnotatedElementUtils.findMergedAnnotation(type, JsonNaming.class);
        if (jsonNaming == null) {
            return null;
        }
        Class<? extends PropertyNamingStrategy> clazz = jsonNaming.value();
        if (!PropertyNamingStrategy.PropertyNamingStrategyBase.class.isAssignableFrom(clazz)) {
            log.error("class: {} is not PropertyNamingStrategyBase.", clazz.getName());
            return null;
        }
        Class<? extends PropertyNamingStrategy.PropertyNamingStrategyBase> base =
            (Class<? extends PropertyNamingStrategy.PropertyNamingStrategyBase>) clazz;
        try {
            return base.newInstance();
        } catch (InstantiationException | IllegalAccessException e) {
            log.error("can not create instance for class: {}", clazz.getName(), ExceptionUtil.getErrorMessage(e));
            return null;
        }
    }
}
