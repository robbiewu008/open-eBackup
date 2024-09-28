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
package openbackup.access.framework.resource.validator.enums;

/**
 * JsonSchema中需获取的字段的关键字枚举
 *
 */
public enum SchemaKeywordsEnum {
    SECRET("secret"), // jsonschema文件中，关键字为secret的元素名
    EDITABLE("editable"); // jsonschema文件中，关键字为editable的元素名

    private String value;

    SchemaKeywordsEnum(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }
}
