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

import openbackup.openstack.adapter.enums.OpenStackJobType;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * 云核OpenStack恢复任务Dto
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class OpenStackRestoreJobDto {
    /**
     * 恢复任务id
     */
    private String id;

    /**
     * 恢复任务名称
     */
    @NotEmpty
    private String name;

    /**
     * 恢复任务描述
     */
    @NotEmpty
    private String description;

    /**
     * 恢复对象实例
     */
    @NotNull
    private OpenStackJobType type;

    /**
     * 恢复对象实例id，虚拟机id或卷id
     */
    @NotEmpty
    private String instanceId;

    /**
     * 待恢复的备份副本id
     */
    @NotEmpty
    private String copyId;

    /**
     * 任务执行结果
     */
    private String result;

    /**
     * 任务状态
     */
    private String status;
}
