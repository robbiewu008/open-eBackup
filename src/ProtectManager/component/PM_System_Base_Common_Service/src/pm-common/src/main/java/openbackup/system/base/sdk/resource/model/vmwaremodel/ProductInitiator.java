/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model.vmwaremodel;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * ProductInitiator 实体类
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-07-31
 */
@Getter
@Setter
public class ProductInitiator {
    @JsonProperty("esxi_name")
    private String esxiName;

    @JsonProperty("esxi_moref")
    private String esxiMoref;

    @JsonProperty("initiators")
    private Initiator initiators;
}
