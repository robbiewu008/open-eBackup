package openbackup.clickhouse.plugin.constant;

import java.util.regex.Pattern;

/**
 * ClickHouse常量类
 *
 * @author q00464130
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-27
 */
public class ClickHouseConstant {
    /**
     * 扩展信息 extendInfo 中 业务IP 的 Key 名称
     */
    public static final String IP = "ip";

    /**
     * agent返回的集群信息里面的ip
     */
    public static final String HOST_ADDRESS = "host_address";

    /**
     * 扩展信息 extendInfo 中 端口 的 Key 名称
     */
    public static final String PORT = "port";

    /**
     * UUID
     */
    public static final String UUID = "uuid";

    /**
     * 库名
     */
    public static final String DB_NAME = "db_name";

    /**
     * 版本号
     */
    public static final String VERSION = "version";

    /**
     * 表引擎
     */
    public static final String TABLE_ENGINE = "table_engine";

    /**
     * 类型
     */
    public static final String TYPE = "type";

    /**
     * 子类型
     */
    public static final String SUB_TYPE = "subType";

    /**
     * 客户端安装路径
     */
    public static final String CLIENT_PATH = "clientPath";

    /**
     * name最大长度
     */
    public static final int NMAE_MAX_LENGTH = 64;

    /**
     * name最大长度
     */
    public static final int NMAE_MIN_LENGTH = 3;

    /**
     * 最小长度——8
     */
    public static final int PWD_MIN_LENGTH = 8;

    /**
     * Linux合法路径的正则表达式
     */
    public static final Pattern LINUX_PATH_PATTERN = Pattern.compile(
        "(/([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255}/)*([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255})|/)");

    /**
     * 查询集群详情，枚举：version/database/table
     */
    public static final String QUERY_DETAIL_QUERY_TYPE = "queryType";

    /**
     * 查询集群详情，queryType=table时，需要制定查哪个库
     */
    public static final String QUERY_DETAIL_QUERY_DATABASE = "queryDatabase";

    /**
     * 查询集群详情，queryType查询数据库信息的value
     */
    public static final String QUERY_TYPE_VALUE_DATABASE = "database";

    /**
     * 查询集群详情，queryType查询数据库信息的value
     */
    public static final String QUERY_TYPE_VALUE_VERSION = "version";

    /**
     * 查询集群详情，queryType查询数据库信息的value
     */
    public static final String QUERY_TYPE_VALUE_TABLE = "table";

    /**
     * 集群的type
     */
    public static final String CLUSTER_TYPE = "Cluster";

    /**
     * 数据库的type
     */
    public static final String DATABASE_TYPE = "Database";

    /**
     * 表集的type
     */
    public static final String TABLE_SET_TYPE = "TableSet";

    /**
     * 表的Type
     */
    public static final String TABLE_TYPE = "Table";

    /**
     * 节点的type
     */
    public static final String NODE_TYPE = "Node";

    /**
     * 版本号，第一个.之前，至少是20
     */
    public static final int CLICK_HOUSE_VERSION_FIRST = 20;

    /**
     * 父资源UUID
     */
    public static final String PARENT_UUID = "parentUuid";

    /**
     * 查询资源信息，分页大小
     */
    public static final int QUERY_RESOURCE_PAGE_SIZE = 100;

    /**
     * 多文件系统key名称
     */
    public static final String ADVANCE_PARAMS_KEY_MULTI_FILE_SYSTEM = "multiFileSystem";

    /**
     * 多节点执行
     */
    public static final String ADVANCE_PARAMS_KEY_MULTI_POST_JOB = "multiPostJob";

    /**
     * 备份时，传给DME参数中的backupObject的key
     */
    public static final String BACKUP_OBJECT_KEY = "backupObject";

    /**
     * 表集备份时，传给DME参数中的backupObject的value
     */
    public static final String BACKUP_OBJECT_TABLE_SET_TYPE = "tableSet";

    /**
     * 数据库备份时，传给DME参数中的backupObject的value
     */
    public static final String BACKUP_OBJECT_DATABASE_TYPE = "database";

    /**
     * 备份时，传给DME参数中的backupTables的key
     */
    public static final String BACKUP_TABLES_KEY = "backupTables";

    /**
     * 备份时，传给DME参数中的backupDatabases的key
     */
    public static final String BACKUP_DATABASE_KEY = "backupDatabase";

    /**
     * 节点扩展字段里面存放agent id
     */
    public static final String AGENT_ID = "agentId";

    /**
     * 恢复时，原始集群的agentId
     */
    public static final String ORIGINAL_AGENT_ID = "originalAgentId";

    /**
     * 备份时，backupTables中name的key
     */
    public static final String BACKUP_TABLES_NAME_KEY = "name";

    /**
     * 备份时，backupTables中engine的key
     */
    public static final String BACKUP_TABLES_ENGINE_KEY = "engine";

    /**
     * 备份时，backupTables中backupData的key
     */
    public static final String BACKUP_TABLES_BACKUP_DATA_KEY = "backupData";

    /**
     * 备份时，backupTables中dbName的key
     */
    public static final String BACKUP_TABLES_DB_NAME_KEY = "dbName";

    /**
     * 备份时，表需要备份，backupTables中backupData的值
     */
    public static final String BACKUP_DATA_BACKUP = "1";

    /**
     * MergeTree引擎
     */
    public static final String MERGE_TREE_ENGINE = "MergeTree";

    /**
     * ReplicatedMergeTree引擎
     */
    public static final String REPLICATED_MERGE_TREE_ENGINE = "ReplicatedMergeTree";

    /**
     * 备份时，表需不要备份，backupTables中backupData的值
     */
    public static final String BACKUP_DATA_NO_BACKUP = "0";

    /**
     * 备份时，表不存在，backupTables中backupData的值
     */
    public static final String BACKUP_DATA_TABLE_DOES_NOT_EXIST = "-1";

    /**
     * 分片的副本数量
     */
    public static final String REPLICA_NUM = "replica_num";

    /**
     * 集群中的分片数
     */
    public static final String SHARD_NUM = "shard_num";

    /**
     * 写数据时该分片的相对权重
     */
    public static final String SHARD_WEIGHT = "shard_weight";

    /**
     * 原因：集群节点信息错误或者网络异常。
     * 建议：1、请检查集群节点信息填写正确。
     * 2、请确保数据保护代理主机和ClickHouse网络连接正常。
     */
    public static final long CLICK_HOUSE_CONNECT_FAILED = 1577209993L;

    private ClickHouseConstant() {
    }
}