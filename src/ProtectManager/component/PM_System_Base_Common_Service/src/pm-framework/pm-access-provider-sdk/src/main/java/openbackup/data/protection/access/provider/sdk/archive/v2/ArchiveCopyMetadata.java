/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.archive.v2;

/**
 * 更新归档副本元数据请求对象
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/21
 **/
public class ArchiveCopyMetadata {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 副本元数据，副本对象的json串
     */
    private String pmMetadata;

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }

    public String getPmMetadata() {
        return pmMetadata;
    }

    public void setPmMetadata(String pmMetadata) {
        this.pmMetadata = pmMetadata;
    }
}
