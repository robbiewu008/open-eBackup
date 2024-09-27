/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 本地存储告警查询返回对象
 *
 * @author y30000858
 * @since 2021-01-12
 */
@Data
public class StorageAlarmResponse {
    private int alarmStatus;

    private int type;

    private long recoverTime;

    private long clearTime;

    private long startTime;

    private int level;

    private long sequence;

    private String sourceID;

    private String sourceType;

    @JsonProperty("strEventID")
    private String strEventId;

    private String alarmObjType;

    private String location;

    private String name;

    private String suggestion;

    private String description;

    private String detail;

    // 事件参数
    private String eventParam;
}
