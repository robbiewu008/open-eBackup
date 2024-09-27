package openbackup.goldendb.protection.access.dto.instance;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 全局事务节点
 *
 * @author s30036254
 * @since 2023-02-14
 */
@NoArgsConstructor
@Data
public class Gtm {
    /**
     * nodeType 节点类型
     */
    private String nodeType;

    /**
     * parentUuid agentId
     */
    private String parentUuid;

    /**
     * osUser 用户
     */
    private String osUser;

    /**
     * agent名称（用于显示）
     */
    private String parentName;

    /**
     * gtmId
     */
    private String gtmId;

    /**
     * 端口
     */
    private String port;

    /**
     * 主节点标志
     */
    private String masterFlag;

    /**
     * gtm ip
     */
    private String gtmIp;
}
