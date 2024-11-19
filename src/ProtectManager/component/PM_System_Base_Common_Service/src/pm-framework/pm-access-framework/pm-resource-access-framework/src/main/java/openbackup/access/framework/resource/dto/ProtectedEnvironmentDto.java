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
package openbackup.access.framework.resource.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;
import openbackup.data.protection.access.provider.sdk.base.Authentication;

import org.hibernate.validator.constraints.Length;

/**
 * 受保护环境DTO对象
 *
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class ProtectedEnvironmentDto extends BaseProtectedEnvironmentDto {
    // 资源类型（主类）
    @Length(max = 64)
    private String type;

    // 资源子类
    @Length(max = 64)
    private String subType;

    // 版本
    @Length(max = 64)
    private String version;

    // 创建时间
    @Length(max = 256)
    private String createdTime;

    // 用户id
    @Length(max = 255)
    @JsonProperty("userid")
    private String userId;

    // 是否开启多组户共享
    private Boolean isShared;

    // 1，注册安装，2，更新
    private String registerType;

    // 认证信息
    private Authentication auth;
}
