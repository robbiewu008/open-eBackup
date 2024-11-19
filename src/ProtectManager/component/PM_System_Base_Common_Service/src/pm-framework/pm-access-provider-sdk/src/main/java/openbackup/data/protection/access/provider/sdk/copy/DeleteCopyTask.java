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
package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 删除副本任务实体类
 *
 */
@Data
public class DeleteCopyTask {
    private String taskId;

    private String requestId;

    private String copyId;

    private TaskEnvironment protectEnv;

    private TaskResource protectObject;


    private List<StorageRepository> repositories;

    @JsonProperty("forceDelete")
    private Boolean isForceDeleted;

    @JsonProperty("isDeleteData")
    private Boolean isDeleteData;

    private List<Endpoint> agents;

    private Map<String, String> advanceParams;

    /**
     * 数据布局
     */
    private BaseDataLayout dataLayout;

    /**
     * 获取dataLayout，如果为null则new一个实例
     *
     * @return BaseDataLayout实例
     */
    public BaseDataLayout getDataLayout() {
        if (this.dataLayout == null) {
            this.dataLayout = new BaseDataLayout();
        }
        return this.dataLayout;
    }

    public void setDataLayout(BaseDataLayout dataLayout) {
        this.dataLayout = dataLayout;
    }

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
