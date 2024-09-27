package openbackup.oceanbase.common.dto;

import lombok.Data;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-04
 */
@Data
public class OBAgentInfo {
    /**
     * 主机agent节点的uuid
     */
    private String parentUuid;

    /**
     * 主机agent上OceanBase提供服务的IP
     */
    private String ip;

    /**
     * 主机agent上OceanBase提供服务的端口
     */
    private String port;

    /**
     * 节点类型，枚举: OBServer, OBClient
     */
    private String nodeType;

    /**
     * 节点状态
     */
    private String linkStatus;
}
