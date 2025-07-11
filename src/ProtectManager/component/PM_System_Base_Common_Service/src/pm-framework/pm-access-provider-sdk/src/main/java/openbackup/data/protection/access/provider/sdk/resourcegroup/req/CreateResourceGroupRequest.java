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
package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.hibernate.validator.constraints.Length;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 创建资源组请求体
 *
 */

@Getter
@Setter
public class CreateResourceGroupRequest {
    @Length(min = 1, max = 64)
    @NotNull
    @Pattern(regexp = RegexpConstants.NAME_STR_NOT_START_WITH_NUM, message = "resource group name is invalid")
    private String name;

    @Length(max = 1024)
    @NotNull
    private String path;

    @Length(max = 64)
    @NotNull
    private String sourceType;

    @Length(max = 64)
    @NotNull
    private String sourceSubType;

    private List<String> resourceIds;

    @Length(min = 1, max = 64)
    @NotNull
    private String scopeResourceId;

    @Length(min = 1, max = 64)
    private String groupType;

    private String extendStr;
}