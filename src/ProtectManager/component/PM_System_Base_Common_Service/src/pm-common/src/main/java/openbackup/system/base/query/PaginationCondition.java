/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.query;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import java.util.Collection;
import java.util.Collections;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-06-04
 */
public class PaginationCondition {
    private final JSONObject conditions;

    public PaginationCondition(JSONObject conditions) {
        this.conditions = conditions;
    }

    /**
     * get val
     *
     * @param key kye
     * @return val
     */
    public Object get(String key) {
        return conditions.get(key);
    }

    /**
     * get pagination condition
     *
     * @param field field
     * @param operation page query condition enum
     * @param value val
     * @param <T> template type T
     * @return pagination condition
     */
    public <T> PaginationCondition with(String field, PageQueryOperator operation, T value) {
        return with(field, operation.getValue(), value, false);
    }

    /**
     * get pagination conditon
     *
     * @param field field
     * @param operation str
     * @param value val
     * @param <T> template type T
     * @return pagination conditiion
     */
    public <T> PaginationCondition with(String field, String operation, T value) {
        return with(field, operation, value, false);
    }

    /**
     * get pagination condition
     *
     * @param field field
     * @param operation str
     * @return pagination condition
     */
    public PaginationCondition with(String field, String operation) {
        return with(field, operation, get(field), true);
    }

    /**
     * get pagination condition
     *
     * @param field field
     * @param operation page query condition enum
     * @return pagination condition
     */
    public PaginationCondition with(String field, PageQueryOperator operation) {
        return with(field, operation.getValue(), get(field), true);
    }

    /**
     * get pagination condition
     *
     * @param field field
     * @param operation str
     * @param value val
     * @param isOverwrite boolean
     * @param <T> template type T
     * @return pagination condition
     */
    private <T> PaginationCondition with(String field, String operation, T value, boolean isOverwrite) {
        if (value == null) {
            return this;
        }
        JSONArray condition = conditions.getJSONArray(field);
        if (isOverwrite || condition == null) {
            condition = new JSONArray();
            conditions.set(field, condition);
        }
        condition.add(Collections.singletonList(operation));
        if (value instanceof Collection) {
            condition.addAll((Collection) value);
        } else {
            condition.add(value);
        }
        return this;
    }
}
