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

import java.util.List;

/**
 * 根据key分组查询资源列表
 *
 * @author t30028453
 * @version [X8000 1.2.1]
 * @since 2022-05-20
 */
@Data
public class ProtectedResourceGroupResult {
    /**
     * 扩展属性 key 名称
     */
    private String key;

    /**
     * 扩展属性 value 值
     */
    private String value;

    /**
     * 资源集合
     */
    private List<ProtectedResource> resources;
}
