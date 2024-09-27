package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

/**
 * Dme服务化备份集群对象属性
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-29
 */
@Setter
@Getter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class DmeAzInfo {
    /**
     * 备份集群id
     */
    private String clusterId;

    /**
     * 端口号
     */
    private String port;

    /**
     * 可用分区名称
     */
    private String azName;

    /**
     * 可用分区id
     */
    private String azId;

    /**
     * 备份集群IP地址
     */
    private String ip;

    /**
     * 备份集群esn
     */
    private String sn;
}
