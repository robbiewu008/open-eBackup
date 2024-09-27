/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.dto;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * OpenStack批量创建备份任务请求体
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-13
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
@AllArgsConstructor
@NoArgsConstructor
public class BatchCreateBackupJobDto {
    /**
     * {@link OpenStackBackupJobDto} 创建备份请求体列表
     */
    @Size(min = 1, max = 50)
    @NotNull
    @Valid
    List<OpenStackBackupJobDto> backupJobs;
}
