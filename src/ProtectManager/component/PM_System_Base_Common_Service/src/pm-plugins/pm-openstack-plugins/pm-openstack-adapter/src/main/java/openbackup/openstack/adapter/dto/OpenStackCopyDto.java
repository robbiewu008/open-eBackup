package openbackup.openstack.adapter.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

/**
 * 云核OpenStack副本Dto
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class OpenStackCopyDto {
    /**
     * 备份副本id
     */
    private String id;

    /**
     * 所属备份任务id，转为保护对象id
     */
    private String backupJobId;

    /**
     * 副本生成时间
     */
    private String generateTime;

    /**
     * 副本生成序列号（在所属备份任务中第几个生成的，从1开始）
     */
    private int generateId;

    /**
     * 副本占用空间大小，单位GB
     */
    private int size;

    /**
     * 是否为备份任务生成的最新副本
     */
    @JsonProperty("is_latest")
    private boolean isLatest;
}
