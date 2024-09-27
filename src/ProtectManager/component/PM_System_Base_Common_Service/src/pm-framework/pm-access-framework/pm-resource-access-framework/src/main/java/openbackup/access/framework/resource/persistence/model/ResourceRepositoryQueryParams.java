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
package openbackup.access.framework.resource.persistence.model;

import lombok.Data;

import java.util.Map;

/**
 * 资源repository查询参数
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-23
 */
@Data
public class ResourceRepositoryQueryParams {
    // 查询条件是否忽略资源的拥有者
    private boolean shouldIgnoreOwner;

    private boolean isDesesitization;

    private int page;

    private int size;

    private Map<String, Object> conditions;

    private String[] orders;

    public ResourceRepositoryQueryParams() {
        shouldIgnoreOwner = false;
    }

    public ResourceRepositoryQueryParams(boolean shouldIgnoreOwner, int page, int size, Map<String, Object> conditions,
        String[] orders) {
        this.shouldIgnoreOwner = shouldIgnoreOwner;
        this.page = page;
        this.size = size;
        this.conditions = conditions;
        this.orders = orders;
    }
}
