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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 本地存储告警查询返回对象
 *
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
