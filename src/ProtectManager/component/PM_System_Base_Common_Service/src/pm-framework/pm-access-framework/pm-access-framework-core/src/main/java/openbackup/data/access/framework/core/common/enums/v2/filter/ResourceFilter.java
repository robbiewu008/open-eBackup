/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums.v2.filter;

import java.util.List;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * ResourceFilter
 *
 * @description: 资源过滤器，用于定义各种资源的过滤规则及过滤条件，用于跟框架层传递数据
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
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
