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
package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 复制原副本保留时间
 *
 * @since 2023-03-06
 */
@Data
public class ReplicationOriginCopyDuration {
    @JsonProperty("retention_type")
    private int retentionType;

    @JsonProperty("retention_duration")
    private int retentionDuration;

    @JsonProperty("duration_unit")
    private String durationUnit;
}
