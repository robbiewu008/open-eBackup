/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.host.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * asm auth info for target oracle host
 *
 * @author y30000858
 * @since 2020-09-21
 */
@Data
public class AsmAuthEntity {
    @JsonProperty("auth_type")
    private Integer authType;

    @JsonProperty("asm_insts")
    private List<String> asmInstant;

    @JsonProperty("username")
    private String userName;

    @JsonProperty("password")
    private String passWord;
}
