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
package com.huawei.oceanprotect.system.base.dto.pacific;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * 每个节点的名称和端口绑定信息
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-19
 */
@Getter
@Setter
public class NodeInfoDto {
    // 节点名称
    private String name;

    // 节点管理ip
    private String manageIp;

    // 节点绑定的端口的绑定类型信息
    private Map<String, String> portBondMap;
}
