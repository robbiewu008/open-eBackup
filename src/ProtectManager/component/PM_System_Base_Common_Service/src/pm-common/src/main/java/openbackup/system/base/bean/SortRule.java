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
package openbackup.system.base.bean;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * 排序规则
 *
 */
@Data
public class SortRule {
    /**
     * 排序的字段
     */
    private String field;

    /**
     * 顺序
     */
    private List<?> order = new ArrayList<>();

    /**
     * 是否倒序
     */
    private boolean isReversed = false;

    public SortRule(String field, List<?> order, boolean isReversed) {
        this.field = field;
        this.order = order;
        this.isReversed = isReversed;
    }

    public SortRule(String field) {
        this(field, new ArrayList<>(), false);
    }

    public SortRule(String field, boolean isReversed) {
        this(field, new ArrayList<>(), isReversed);
    }

    public SortRule(String field, List<?> order) {
        this(field, order, false);
    }
}
