/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 安全卡信息
 *
 * @author j00619968
 * @since 2024-01-23
 */
@Getter
@Setter
public class SecureCardInfo {
    /**
     * 健康状态
     */
    @JsonIgnore
    public static final String HEALTH_STATUS_OK = "1";

    /**
     * 运行状态
     */
    @JsonIgnore
    public static final String RUNNING_STATUS_OK = "2";

    /**
     * 控制器ID，例如"0A"
     */
    @JsonProperty("controllerID")
    String controllerID;

    /**
     * 健康状态，0:未知;1:正常;2:故障
     */
    @JsonProperty("healthStatus")
    String healthStatus;

    /**
     * 硬件ID
     */
    @JsonProperty("id")
    String id;

    /**
     * 硬件运行状态，0:未知;1:正常;2:运行;12:正在上电;13:已下电;27:在线;28:离线;103:上电失败
     */
    @JsonProperty("runningStatus")
    String runningStatus;
}
