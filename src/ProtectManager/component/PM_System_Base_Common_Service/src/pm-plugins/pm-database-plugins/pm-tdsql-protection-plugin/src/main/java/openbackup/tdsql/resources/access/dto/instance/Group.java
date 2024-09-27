package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

import java.util.List;

/**
 * group信息
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Data
public class Group {
    /**
     * setId 分片id 非分布式只有一个setID，非分布式有多个setID，即有多个节点
     */
    private String setId;

    /**
     * 节点
     */
    private List<DataNode> dataNodes;
}
