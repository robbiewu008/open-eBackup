/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * dataStore信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class DataStore {
    @JsonProperty("mo_id")
    private String moId;

    @JsonProperty("uuid")
    private String uuid;

    @JsonProperty("url")
    private String url;

    @JsonProperty("name")
    private String name;

    @JsonProperty("type")
    private String type;

    @JsonProperty("partitions")
    private List<String> partitions;
}
