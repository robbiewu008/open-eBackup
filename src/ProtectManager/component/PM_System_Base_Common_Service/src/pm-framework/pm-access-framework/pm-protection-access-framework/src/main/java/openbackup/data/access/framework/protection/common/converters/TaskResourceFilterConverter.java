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
package openbackup.data.access.framework.protection.common.converters;

import openbackup.data.access.framework.core.common.enums.v2.filter.ResourceFilter;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResourceFilter;

/**
 * TaskResourceFilter对象转换器，用于跟controller层对象转换
 *
 **/
public class TaskResourceFilterConverter {
    /**
     * 转换为TaskResourceFilter对象
     *
     * @param filter controller层资源过滤器对象
     * @return TaskResourceFilter对象
     */
    public static TaskResourceFilter covertToTaskResourceFilter(ResourceFilter filter) {
        TaskResourceFilter taskResourceFilter = new TaskResourceFilter();
        taskResourceFilter.setFilterBy(filter.getFilterBy().getCondition());
        taskResourceFilter.setMode(filter.getMode().getMode());
        taskResourceFilter.setRule(filter.getRule().getRule());
        taskResourceFilter.setType(filter.getType().getType());
        taskResourceFilter.setValues(filter.getValues());
        return taskResourceFilter;
    }
}
