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

import openbackup.system.base.common.validator.constants.RegexpConstants;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.common.constants.IsmNumberConstant;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 指定管理集群时，需要的相关本地集群参数
 *
 */
@Data
public class LocalClusterRequest {
    // 本地集群名称
    @NotNull
    @NotEmpty(message = "The clusterName cannot be empty. ")
    @NotBlank(message = "The clusterName cannot be blank. ")
    @Length(max = IsmNumberConstant.THIRTY_TWO, min = IsmNumberConstant.FOUR, message = "The length of cluster name is 4-32 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "cluster name is invalid")
    private String localClusterName;

    // 本地集群-系统管理员的用户名
    @NotNull(message = "The username cannot be null. ")
    @NotEmpty(message = "The username cannot be empty. ")
    @NotBlank(message = "The username cannot be blank. ")
    @Length(min = IsmNumberConstant.FIVE, max = IsmNumberConstant.SIXTY_FOUR)
    private String username;

    // 本地集群-系统管理员的密码
    @NotNull(message = "The password cannot be null. ")
    @Length(min = IsmNumberConstant.EIGHT, max = IsmNumberConstant.SIXTY_FOUR)
    private String password;

    // 指定管理集群操作，是否同步到远端【true：则会在目标集群也指定此本地集群为管理集群；false：则不在目标集群操作】
    @NotNull
    @JsonProperty("syncToRemote")
    private Boolean shouldSyncToRemote = true;
}
