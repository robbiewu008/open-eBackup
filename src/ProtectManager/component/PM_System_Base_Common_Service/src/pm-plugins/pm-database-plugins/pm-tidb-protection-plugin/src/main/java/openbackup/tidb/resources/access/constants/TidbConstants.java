package openbackup.tidb.resources.access.constants;

/**
 * Tidb 常量
 *
 * @author w00426202
 * @since 2023-07-08
 */
public class TidbConstants {
    /**
     * 资源上限
     */
    public static final int TIDB_RESOURCE_MAX_COUNT = 2000;

    /**
     * 操作类型
     */
    public static final String ACTION_TYPE = "action_type";

    /**
     * 健康检查
     */
    public static final String CHECK_CLUSTER = "check_cluster";

    /**
     * 检查数据库
     */
    public static final String CHECK_DB = "check_db";

    /**
     * 检查表
     */
    public static final String CHECK_TABLE = "check_table";

    /**
     * conditions
     */
    public static final String CONDITIONS = "conditions";

    /**
     * 集群名字
     */
    public static final String CLUSTER_NAME = "clusterName";

    /**
     * 数据库名称
     */
    public static final String DATABASE_NAME = "databaseName";

    /**
     * 表名称
     */
    public static final String TABLE_NAME = "tableName";

    /**
     * 表名称 list
     */
    public static final String TABLE_NAME_LIST = "tableNameList";

    /**
     * 集群列表
     */
    public static final String CLUSTER_INFO_LIST = "clusterInfoList";

    /**
     * 资源父id
     */
    public static final String PARENT_UUID = "parentUuid";

    /**
     * 操作类型
     * 0:注册/1:修改
     */
    public static final String SAVE_TYPE_UPDATE = "1";

    /**
     * 保存类型
     */
    public static final String SAVE_TYPE = "save_type";

    /**
     * tiupUuid
     */
    public static final String TIUP_UUID = "tiupUuid";

    /**
     * tiupUuid
     */
    public static final String TIUP_PATH = "tiupPath";

    /**
     * logBackupPath
     */
    public static final String LOG_BACKUP_PATH = "logBackupPath";

    /**
     * 日志副本时间点恢复的时候时间点参数
     */
    public static final String RESTORE_TIME_STAMP_KEY = "restoreTimestamp";

    /**
     * 资源表 column PARENT_NAME
     */
    public static final String PARENT_NAME = "parent_name";

    /**
     * action type  list_table
     */
    public static final String ACTION_LIST_TABLE = "list_table";

    /**
     * query table result , table list
     */
    public static final String TABLE_LIST = "table_list";

    /**
     * table num constant
     */
    public static final String TABLE_NUM = "table_num";

    /**
     * 日志备份支持6.2及以上
     */
    public static final String LOG_BACK_UP_VERSION = "6.2.0";

    /**
     * role
     */
    public static final String ROLE = "role";

    /**
     * id
     */
    public static final String ID = "id";

    /**
     * host
     */
    public static final String HOST = "host";

    /**
     * status
     */
    public static final String STATUS = "status";

    /**
     * hostManagerIp
     */
    public static final String HOST_MANAGER_IP = "hostManagerIp";

    /**
     * check_tiup actiontype
     */
    public static final String ACTION_TYPE_CHECK_TIUP = "check_tiup";

    /**
     * hostManagerResourceUuid
     */
    public static final String HOST_MANAGER_RESOURCE_UUID = "hostManagerResourceUuid";

    /**
     * 表注册最大数量
     */
    public static final int TABLE_REGISTER_LIMIT_NUM = 256;

    /**
     * 错误码,请求成功
     */
    public static final String SUCCESS_ERROR_CODE = "0";

    private TidbConstants() {
    }
}
