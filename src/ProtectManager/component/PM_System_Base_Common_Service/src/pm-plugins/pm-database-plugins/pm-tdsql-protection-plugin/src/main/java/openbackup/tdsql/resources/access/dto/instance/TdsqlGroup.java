package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

/**
 * 分布式实例数据
 *
 * @author z00445440
 * @since 2023-11-14
 */
@Data
public class TdsqlGroup {
    /**
     * id
     */
    private String id;

    /**
     * name
     */
    private String name;

    /**
     * type分布式/非分布式
     */
    private String type;

    /**
     * 关联的集群
     */
    private String cluster;

    /**
     * 实例分片和数据节点信息
     */
    private GroupInfo group;
}
