package openbackup.goldendb.protection.access.dto.cluster;

import lombok.Data;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-14
 */
@Data
public class Node {
    /**
     * 节点类型
     */
    private String nodeType;

    /**
     * agentId
     */
    private String parentUuid;

    /**
     * 用户
     */
    private String osUser;
}
