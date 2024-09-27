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