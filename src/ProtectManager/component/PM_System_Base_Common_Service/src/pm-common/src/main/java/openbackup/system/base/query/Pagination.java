/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00272247
 * @param <T> template type
 * @since 2021-06-01
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
