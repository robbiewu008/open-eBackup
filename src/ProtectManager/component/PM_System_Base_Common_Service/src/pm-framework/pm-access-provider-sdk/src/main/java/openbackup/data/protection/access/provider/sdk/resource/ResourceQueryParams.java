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
package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Map;

/**
 * 资源查询context
 *
 * @author h30027154
 * @since 2022-07-08
 */
@Data
public class ResourceQueryParams {
    private boolean shouldDecrypt = false;

    private boolean shouldQueryDependency = false;

    private boolean isDesesitization = false;

    private boolean shouldLoadEnvironment = true;

    // 查询条件是否忽略资源的拥有者
    private boolean shouldIgnoreOwner = false;

    @JsonProperty("pageNo")
    private int page = 0;

    @JsonProperty("pageSize")
    private int size = 10;

    private Map<String, Object> conditions;

    private String[] orders = new String[0];
}
