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
package openbackup.system.base.sdk.user.enums;

import lombok.Getter;

/**
 * 功能描述
 *
 */
@Getter
public enum OperationTypeEnum {
    /**
     * 创建
     */
    CREATE("create"),
    /**
     * 删除
     */
    DELETE("delete"),
    /**
     * 修改
     */
    MODIFY("modify"),
    /**
     * 查询详情
     */
    QUERY("query");

    private final String value;

    OperationTypeEnum(String value) {
        this.value = value;
    }
}
