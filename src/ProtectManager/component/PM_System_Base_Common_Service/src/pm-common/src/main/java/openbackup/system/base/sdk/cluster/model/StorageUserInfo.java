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
package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Storage user info
 *
 * @author p30001902
 * @since 2020-12-11
 */
@Data
public class StorageUserInfo {
    @JsonProperty("userId")
    @JsonAlias("ID")
    private String userId;

    @JsonProperty("roleId")
    @JsonAlias("ROLEID")
    private String roleId;

    @JsonProperty("createTime")
    @JsonAlias("CREATETIME")
    private String createTime;

    // user type
    private ClusterEnum.DoradoCreateUserType userType;

    private String esn;
}
