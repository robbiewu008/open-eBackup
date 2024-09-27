package openbackup.openstack.adapter.dto;

import openbackup.openstack.adapter.enums.ScheduleRetentionType;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

/**
 * 备份副本保留策略DTO
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ScheduleRetentionDto {
    /**
     * 保留策略类型
     */
    @NotNull
    private ScheduleRetentionType type;

    /**
     * 保留最近count次备份的副本或保留最近count天产生的备份副本,0为永久保留
     */
    @Min(0)
    @Max(300)
    @NotNull
    private Integer count;
}
