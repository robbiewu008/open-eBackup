/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.protection.controller.req;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * HDFS Login Params
 *
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
