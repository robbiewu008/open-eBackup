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
package openbackup.oceanbase.common.dto;

import lombok.Data;

/**
 * 功能描述
 *
 */
@Data
public class OBTenantInfo {
    /**
     * 租户名称
     */
    private String name;

    /**
     * 节点状态
     */
    private String linkStatus;

    public OBTenantInfo() {
        super();
    }

    public OBTenantInfo(String name) {
        this.name = name;
    }
}
