/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller.response;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * KerberosIDResp
 *
 * @author m00576658
 * @since 2021-08-17
 */
@Data
public class KerberosIDResp {
    @JsonProperty("kerberos_id")
    private String kerberosId;
}
