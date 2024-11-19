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
import openbackup.system.base.common.utils.JSONObject;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * 创建资源组保护请求体
 *
 */

@Getter
@Setter
public class CreateResourceGroupProtectedObjectRequest {
    @Length(min = 1, max = 64)
    @NotNull
    private String resourceGroupId;

    @Length(min = 1, max = 36)
    @NotNull
    private String slaId;

    @Length(min = 1, max = 36)
    private String postAction;

    private JSONObject extParams;
}