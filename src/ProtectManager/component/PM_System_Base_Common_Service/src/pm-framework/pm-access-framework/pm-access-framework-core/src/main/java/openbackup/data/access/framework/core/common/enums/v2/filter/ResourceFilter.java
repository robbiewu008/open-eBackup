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

import java.util.List;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * ResourceFilter
 *
 * @description: 资源过滤器，用于定义各种资源的过滤规则及过滤条件，用于跟框架层传递数据
 **/
public class ResourceFilter {
    /**
     * 根据什么进行过滤
     */
    @NotNull
    private FilterConditionEnum filterBy;

    /**
     * 过滤的资源类型
     */
    @NotNull
    private FilterTypeEnum type;

    /**
     * 过滤规则
     */
    @NotNull
    private FilterRuleEnum rule;

    /**
     * 过滤模式
     */
    @NotNull
    private FilterModeEnum mode;

    /**
     * 过滤的内容
     */
    @NotEmpty
    private List<String> values;

    public FilterConditionEnum getFilterBy() {
        return filterBy;
    }

    public void setFilterBy(FilterConditionEnum filterBy) {
        this.filterBy = filterBy;
    }

    public FilterTypeEnum getType() {
        return type;
    }

    public void setType(FilterTypeEnum type) {
        this.type = type;
    }

    public FilterRuleEnum getRule() {
        return rule;
    }

    public void setRule(FilterRuleEnum rule) {
        this.rule = rule;
    }

    public FilterModeEnum getMode() {
        return mode;
    }

    public void setMode(FilterModeEnum mode) {
        this.mode = mode;
    }

    public List<String> getValues() {
        return values;
    }

    public void setValues(List<String> values) {
        this.values = values;
    }
}
