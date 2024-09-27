/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * The StorageAlarmObj
 *
 * @author g30003063
 * @since 2022/8/11
 */
@Getter
@Setter
public class StorageAlarmObj {
    @JsonProperty("ID")
    private String id;

    @JsonProperty("CMO_ALARM_OBJ_TYPE")
    private String objType;

    @JsonProperty("CMO_ALARM_OBJ_NAME")
    private String objName;
}
