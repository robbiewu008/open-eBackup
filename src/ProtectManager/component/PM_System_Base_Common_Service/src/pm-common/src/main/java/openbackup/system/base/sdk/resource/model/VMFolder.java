/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VMFolder信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class VMFolder {
    @JsonProperty("mo_id")
    private String moId;

    private String name;
}
