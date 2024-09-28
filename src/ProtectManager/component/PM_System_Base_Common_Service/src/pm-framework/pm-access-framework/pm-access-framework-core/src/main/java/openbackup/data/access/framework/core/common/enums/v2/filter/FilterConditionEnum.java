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
package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterByEnum
 *
 * @description: 资源过滤条件枚举类，用于跟框架层传递数据
 **/
public enum FilterConditionEnum {
    NAME("Name"),
    ID("ID"),
    FORMAT("Format"),
    MODIFY_TIME("ModifyTime"),
    CREATE_TIME("CreateTime"),
    ACCESS_TIME("AccessTime");

    private String condition;

    FilterConditionEnum(String condition) {
        this.condition = condition;
    }

    public String getCondition() {
        return condition;
    }
}
