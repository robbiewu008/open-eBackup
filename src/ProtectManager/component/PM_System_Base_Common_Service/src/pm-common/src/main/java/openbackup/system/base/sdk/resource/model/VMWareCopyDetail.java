/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VMWare info in copy properties
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VMWareCopyDetail {
    @JsonProperty("vmware_metadata")
    VMWareMetadata vmWareMetadata;
}
