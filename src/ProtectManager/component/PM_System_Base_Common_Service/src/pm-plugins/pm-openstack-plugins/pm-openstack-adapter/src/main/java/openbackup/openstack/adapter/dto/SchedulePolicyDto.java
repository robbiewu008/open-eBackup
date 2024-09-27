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

import org.hibernate.validator.constraints.Range;

import java.util.List;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

/**
 * 调度策略内容DTO
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class SchedulePolicyDto {
    /**
     * 开始时间，若为空则从创建任务时开始（当前时间）
     */
    private String startDate;

    /**
     * 结束时间，若为空则一直调度
     */
    private String stopDate;

    /**
     * 调度间隔周期
     */
    @Min(1)
    @Max(30)
    private Integer intervalDays;

    /**
     * 每周几执行，1-7代表周一到周日
     */
    private List<@Range(min = 1, max = 7) Integer> daysOfWeek;

    /**
     * 在一天内某时执行，取值范围0-23
     */
    @Min(0)
    @Max(23)
    @NotNull
    private Integer executeTime;
}
