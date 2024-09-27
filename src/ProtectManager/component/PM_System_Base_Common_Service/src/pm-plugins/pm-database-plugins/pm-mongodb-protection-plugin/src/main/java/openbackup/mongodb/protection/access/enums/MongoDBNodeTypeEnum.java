package openbackup.mongodb.protection.access.enums;

import java.util.HashMap;
import java.util.Map;

/**
 * MongoDB类型集合
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public enum MongoDBNodeTypeEnum {
    /**
     * 单机实例节点
     */
    SINGLE("single"),

    /**
     * 分片集群route节点
     */
    MONGOS("mongos"),

    /**
     * 复制集群节点
     */
    REPLICATION("replication"),
    /**
     * 分片集群分片节点
     */
    SHARD("shard"),

    /**
     * 分片集群配置节点
     */
    CONFIG("config");

    private static final Map<String, MongoDBNodeTypeEnum> MONGODB_NODE_MAP = new HashMap<>(
        MongoDBNodeTypeEnum.values().length);

    static {
        for (MongoDBNodeTypeEnum each : MongoDBNodeTypeEnum.values()) {
            MONGODB_NODE_MAP.put(each.getType(), each);
        }
    }

    private final String type;

    MongoDBNodeTypeEnum(String type) {
        this.type = type;
    }

    /**
     * 根据集群类型获取到对应的部署类型
     *
     * @param type 集群节点类型
     * @return 部署类型
     */
    public static MongoDBNodeTypeEnum getMongoDBNodeType(String type) {
        return MONGODB_NODE_MAP.get(type);
    }

    public String getType() {
        return type;
    }
}
