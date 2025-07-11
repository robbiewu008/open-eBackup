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
package openbackup.openstack.adapter.dto;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;
import openbackup.openstack.adapter.enums.JobScheduleType;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;

/**
 * 备份任务调度策略DTO
 *
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class JobScheduleDto {
    /**
     * 调度类型
     */
    @NotNull
    private JobScheduleType type;

    /**
     * 调度策略内容
     */
    @NotNull
    @Valid
    private SchedulePolicyDto policy;

    /**
     * 备份副本保留策略
     */
    @NotNull
    @Valid
    private ScheduleRetentionDto retention;
}
