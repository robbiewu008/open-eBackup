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
package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * 保留时间数据类型
 *
 **/
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class RetentionBo {
    /**
     * retention type
     */
    private Integer retentionType;

    /**
     * retention duration
     */
    private Integer retentionDuration;

    /**
     * duration unit
     */
    private String durationUnit;

    /**
     * worm retention duration
     */
    private Integer wormRetentionDuration;

    /**
     * worm duration unit
     */
    private String wormDurationUnit;

    /**
     * 副本保留个数
     */
    private Integer retentionQuantity;
}
