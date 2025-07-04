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
package openbackup.data.protection.access.provider.sdk.base.v2;

import java.util.List;

/**
 * ResourceFilter
 *
 * @description: 资源过滤器，用于DME接口参数
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
