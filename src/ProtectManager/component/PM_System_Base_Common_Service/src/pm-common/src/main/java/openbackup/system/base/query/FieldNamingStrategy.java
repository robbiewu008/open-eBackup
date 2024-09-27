/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.query;

import java.lang.reflect.Field;

/**
 * Field Naming Strategy
 *
 * @author l00272247
 * @since 2021-06-03
 */
public interface FieldNamingStrategy {
    /**
     * translate
     *
     * @param type type
     * @param field field
     * @return result
     */
    String translate(Class<?> type, Field field);
}
