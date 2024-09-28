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

import lombok.Data;

/**
 * 子资源查询过滤条件对象组合
 *
 */
@Data
public class BrowseEnvironmentResourceConditions {
    // 受保护环境ID
    private String envId;

    // 代理id
    private String agentId;

    // 父资源ID
    private String parentId;

    // 资源类型
    private String resourceType;

    // 资源子类型
    private String resourceSubType;

    // 起始页
    private int pageNo;

    // 每页大小
    private int pageSize;

    // 模糊匹配参数
    private String conditions;
}
