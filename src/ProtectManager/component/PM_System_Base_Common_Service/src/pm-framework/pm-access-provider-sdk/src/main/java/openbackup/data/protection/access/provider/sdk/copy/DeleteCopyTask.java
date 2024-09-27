/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 删除副本任务实体类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-15
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
