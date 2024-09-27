package openbackup.goldendb.protection.access.dto.cluster;

import lombok.Data;

import java.util.List;

/**
 * 功能描述 GoldenDb集群注册
 *
 * @author s30036254
 * @since 2023-02-14
 */
@Data
public class GoldenCluster {
    /**
     * 节点
     */
    private List<Node> nodes;
}
