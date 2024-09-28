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
package openbackup.system.base.query;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * page query condition enum
 *
 */
public enum PageQueryOperator {
    /**
     * 等于
     */
    EQ("=="),

    /**
     * 不等于
     */
    NE("!="),

    /**
     * 大于
     */
    GT(">"),

    /**
     * 大于等于
     */
    GE(">="),

    /**
     * 小于
     */
    LT("<"),

    /**
     * 小于等于
     */
    LE("<="),

    /**
     * 匹配 like "%值“
     */
    LIKE_LEFT("~="),

    /**
     * 匹配 like "值%
     */
    LIKE_RIGHT("=~"),

    /**
     * like
     */
    LIKE("~~"),

    /**
     * in
     */
    IN("in"),

    NOT_IN("not in");

    private final String value;

    PageQueryOperator(String value) {
        this.value = value;
    }

    @JsonValue
    public String getValue() {
        return value;
    }

    /**
     * get page query operation type enum
     *
     * @param str str
     * @return page query operation enum
     */
    @JsonCreator
    public static PageQueryOperator get(String str) {
        return EnumUtil.get(PageQueryOperator.class, PageQueryOperator::getValue, str);
    }
}
