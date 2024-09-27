/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

/**
 * 共享路径恢复任务请求参数
 *
 * @author w00574036
 * @since 2024-04-19
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountTaskReq {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 克隆共享保留时间/小时
     */
    private int fileSystemKeepTime;

    /**
     * 任务状态，success、failed
     */
    private String status;

    /**
     * 任务对应存储设备ID
     */
    private String deviceId;
}
