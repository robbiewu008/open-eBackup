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

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;

import java.util.List;
import java.util.function.Consumer;

/**
 * Pagination
 *
 * @param <T> template type
 */
@Data
public class Pagination<T> {
    /**
     * default_field_naming_strategy
     */
    public static final String DEFAULT_FIELD_NAMING_STRATEGY = "defaultPageQueryFieldNamingStrategy";

    private final int page;
    private final int size;
    private final T conditions;
    private final List<String> orders;
    private String fieldNamingStrategy;

    /**
     * constructor
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     */
    public Pagination(int page, int size, T conditions, List<String> orders) {
        this(page, size, conditions, orders, DEFAULT_FIELD_NAMING_STRATEGY);
    }

    /**
     * constructor
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @param fieldNamingStrategy field naming strategy
     */
    public Pagination(int page, int size, T conditions, List<String> orders, String fieldNamingStrategy) {
        this.page = page;
        this.size = size;
        this.conditions = conditions;
        this.orders = orders;
        this.fieldNamingStrategy = fieldNamingStrategy;
    }

    /**
     * conditions
     *
     * @return conditions
     */
    public JSONObject conditions() {
        return JSONObject.fromObject(conditions, JSONObject.RAW_OBJ_MAPPER);
    }

    /**
     * duplicate
     *
     * @param consumers consumer
     * @return pagination
     */
    public final Pagination<JSONObject> duplicate(List<Consumer<PaginationCondition>> consumers) {
        JSONObject json = conditions();
        PaginationCondition condition = new PaginationCondition(json);
        for (Consumer<PaginationCondition> consumer : consumers) {
            consumer.accept(condition);
        }
        return new Pagination<>(page, size, json, orders, this.fieldNamingStrategy);
    }

    @JsonIgnore
    public String getFieldNamingStrategy() {
        return fieldNamingStrategy;
    }

    public void setFieldNamingStrategy(String fieldNamingStrategy) {
        this.fieldNamingStrategy = fieldNamingStrategy;
    }
}
