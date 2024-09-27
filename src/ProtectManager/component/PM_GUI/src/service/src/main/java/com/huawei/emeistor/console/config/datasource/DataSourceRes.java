/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package com.huawei.emeistor.console.config.datasource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 基础设施返回值
 *
 * @author y30000858
 * @since 2021-01-22
 */
@Data
public class DataSourceRes {
    @JsonProperty("data")
    private JSONArray data;

    @JsonProperty("error")
    private StorageError error;
}
