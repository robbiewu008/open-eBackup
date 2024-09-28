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
package openbackup.data.protection.access.provider.sdk.verify;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;

import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * 副本校验任务对象
 *
 **/
public class CopyVerifyTask extends BaseTask {
    // 高级备份参数，key/value键值对存放
    private Map<String, String> advanceParams;

    public Map<String, String> getAdvanceParams() {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, String> advanceParams) {
        this.advanceParams = advanceParams;
    }

    /**
     * 副本校验任务是否是子任务
     *
     * @return true-子任务；false-非子任务
     */
    public boolean isSubTask() {
        if (StringUtils.isAnyBlank(getRequestId(), getTaskId())) {
            return false;
        }
        return !StringUtils.equals(getRequestId(), getTaskId());
    }

    /**
     * 添加高级参数
     *
     * @param key 参数名称
     * @param value 参数值
     */
    public void addParameter(String key, String value) {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        this.advanceParams.put(key, value);
    }

    /**
     * 添加高级参数
     *
     * @param param 待新增的高级参数map
     */
    public void addParameters(Map<String, String> param) {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        this.advanceParams.putAll(param);
    }
}
