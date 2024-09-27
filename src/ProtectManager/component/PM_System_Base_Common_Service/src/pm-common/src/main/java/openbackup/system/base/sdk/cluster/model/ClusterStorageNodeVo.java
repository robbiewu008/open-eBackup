package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

/**
 * Storage nodes vo
 *
 * @author p30001902
 * @version [版本号, 需求编号_需求名称]
 * @since 2020-06-08
 */
@Data
public class ClusterStorageNodeVo {
    private String nodeId;

    private String nodeName;

    private Integer nodeRole;

    private String managementIPv4;

    private String managementIPv6;

    private Integer status;

    // 数据备份引擎IP
    private String backupEngineIp;

    // 数据利用引擎IP
    private String deeEngineIp;

    // 数据归档引擎IP
    private String archiveEngineIp;

    // 复制网络ip
    private String copyEngineIp;

    // 健康状态
    private String healthStatus;
}
