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
package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 存储池参数
 *
 */
@Data
@NoArgsConstructor
public class StoragePoolParm {
    private static final int ENDING_UP_THRESHOLD = 95;

    /**
     * 容量告警阈值
     */
    @JsonProperty("USERCONSUMEDCAPACITYTHRESHOLD")
    private int userConsumedCapacityThreshold;

    /**
     * 容量严重不足告警阈值
     */
    @JsonProperty("MAJORTHRESHOLD")
    private int majorThreshold;

    /**
     * 即将耗尽容量阈值
     */
    @JsonProperty("ENDINGUPTHRESHOLD")
    private int endingUpThreshold = ENDING_UP_THRESHOLD;

    public StoragePoolParm(int userConsumedCapacityThreshold) {
        this.userConsumedCapacityThreshold = userConsumedCapacityThreshold;
    }
}


