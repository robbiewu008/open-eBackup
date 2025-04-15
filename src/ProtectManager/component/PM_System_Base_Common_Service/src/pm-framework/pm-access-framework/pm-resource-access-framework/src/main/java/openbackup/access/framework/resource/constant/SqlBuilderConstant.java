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
package openbackup.access.framework.resource.constant;

/**
 * 动态sql常量类
 *
 */
public final class SqlBuilderConstant {
    /**
     * labelCondition参数
     */
    public static final String KEY_LABEL_CONDITION = "labelCondition";

    /**
     * labelList参数
     */
    public static final String KEY_LABEL_LIST = "labelList";

    /**
     * 标签过滤条件查询语句前缀
     */
    public static final String LABEL_LIST_CONDITION_SQL_PREFIX =
        "select tlb.resource_object_id from t_label_r_resource_object tlb " + "where tlb.label_id in (";

    /**
     * 标签过滤条件查询语句后缀
     */
    public static final String LABEL_LIST_CONDITION_SQL_SUFFIX =
        ") " + "group by tlb.resource_object_id " + "having count(distinct tlb.label_id) = ";

    /**
     * SINGLE_QUOTES
     */
    public static final String SINGLE_QUOTES = "'";

    /**
     * COMMA
     */
    public static final String COMMA = ",";

    /**
     * COLUMN_Q_UUID
     */
    public static final String COLUMN_Q_UUID = "q.uuid";

    private SqlBuilderConstant() {}
}
