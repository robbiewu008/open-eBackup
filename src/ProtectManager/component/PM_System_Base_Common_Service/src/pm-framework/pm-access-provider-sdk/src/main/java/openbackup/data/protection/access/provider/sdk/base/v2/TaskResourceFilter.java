/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.base.v2;

import java.util.List;

/**
 * ResourceFilter
 *
 * @description: 资源过滤器，用于DME接口参数
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public class TaskResourceFilter {
    /**
     * 根据什么进行过滤，取值：Name-名称；ID-标识；Formate-文件的格式；ModifyTime-修改时间；CreateTime-创建时间
     */
    private String filterBy;

    /**
     * 过滤的资源类型，取值：VM-虚拟机；Disk-磁盘；File-文件；Dir-目录
     */
    private String type;

    /**
     * 过滤规则：如模糊匹配、全匹配、正则表达式等。Fuzzy-模糊匹配；Exact-精确匹配；Regex-正则表达式
     */
    private String rule;

    /**
     * 过滤类型：INCLUDE-包含；EXCLUDE-排除
     */
    private String mode;

    /**
     * 过滤的具体内容
     */
    private List<String> values;

    public String getFilterBy() {
        return filterBy;
    }

    public void setFilterBy(String filterBy) {
        this.filterBy = filterBy;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getRule() {
        return rule;
    }

    public void setRule(String rule) {
        this.rule = rule;
    }

    public String getMode() {
        return mode;
    }

    public void setMode(String mode) {
        this.mode = mode;
    }

    public List<String> getValues() {
        return values;
    }

    public void setValues(List<String> values) {
        this.values = values;
    }
}
