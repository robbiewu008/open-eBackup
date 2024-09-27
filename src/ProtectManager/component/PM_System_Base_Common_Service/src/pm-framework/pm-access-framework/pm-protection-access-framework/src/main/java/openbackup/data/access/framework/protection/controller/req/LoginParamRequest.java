/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.req;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * HDFS Login Params
 *
 * @author m00576658
 * @since 2020/08/05
 */
@Data
public class LoginParamRequest {
    @JsonProperty("env_id")
    private String envId;

    private String username;

    private String password;

    private String endpoint;

    private String type;

    @JsonProperty("sub_type")
    private String subType;

    private Integer port;

    @JsonProperty("login_model")
    private String loginModel;

    @JsonProperty("kerberos_id")
    private String kerberosId;

    @JsonProperty("ext_parameters")
    private JSONObject extParameters;
}
