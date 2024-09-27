/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.index.v2;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import java.util.List;

/**
 * 副本索引参数对象，索引时所需要的参数由该对象承载
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-29
 */
public class CopyIndexTask {
    // 索引请求ID
    private String requestId;

    // 建索引触发方式:AUTO-调度自动建,MANUAL-手动建
    private String triggerMode;

    // 副本信息
    private CreateIndexCopyInfo copyInfo;

    // 索引代理列表
    private List<Endpoint> agents;

    private TaskEnvironment protectEnv;

    private TaskResource protectObject;

    private StorageRepository storageRepository;

    public void setStorageRepository(StorageRepository storageRepository) {
        this.storageRepository = storageRepository;
    }

    public StorageRepository getStorageRepository() {
        return storageRepository;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getTriggerMode() {
        return triggerMode;
    }

    public void setTriggerMode(String triggerMode) {
        this.triggerMode = triggerMode;
    }

    public CreateIndexCopyInfo getCopyInfo() {
        return copyInfo;
    }

    public void setCopyInfo(CreateIndexCopyInfo copyInfo) {
        this.copyInfo = copyInfo;
    }

    public List<Endpoint> getAgents() {
        return agents;
    }

    public void setAgents(List<Endpoint> agents) {
        this.agents = agents;
    }

    public TaskEnvironment getProtectEnv() {
        return protectEnv;
    }

    public void setProtectEnv(TaskEnvironment protectEnv) {
        this.protectEnv = protectEnv;
    }

    public TaskResource getProtectObject() {
        return protectObject;
    }

    public void setProtectObject(TaskResource protectObject) {
        this.protectObject = protectObject;
    }
}
