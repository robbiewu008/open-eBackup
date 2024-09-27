/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

/**
 * The StorageAlarmRecordReq
 *
 * @author g30003063
 * @since 2022-08-02
 */
@Getter
@Setter
@EqualsAndHashCode
public class StorageAlarmRecordReq {
    @JsonProperty("eventId")
    private long alarmId;

    @JsonProperty("eventType")
    private int type;

    @JsonProperty("eventParams")
    private String params;

    // 告警对象字段为0，则DM会使用omrp上的默认资源对象
    @JsonProperty("eventObjType")
    private Integer sourceType = 0;

    @JsonProperty("eventObjId")
    private String sourceId = "";
}