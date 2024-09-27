/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * vmware runtime host info
 *
 * @author h30003246
 * @since 2020-12-22
 */
@Data
public class VMWareHost {
    @JsonProperty("mo_id")
    String moId;

    String name;

    String uuid;

    String version;
}
