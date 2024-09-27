package openbackup.saphana.protection.access.constant;

/**
 * SAP HANA常量
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-09
 */
public class SapHanaConstants {
    /**
     * SAP HANA实例允许接入的最大规格数
     */
    public static final int SAP_HANA_INSTANCE_MAX_COUNT = 64;

    /**
     * SAP HANA检查数据库状态线程池大小
     */
    public static final int THREAD_POOL_NUM = 20;

    /**
     * SAP HANA检查数据库状态线程超时（单位：秒）
     */
    public static final long CHECK_DB_TIMEOUT = 300L;

    /**
     * SAP HANA检查数据库状态线程池工作队列大小
     */
    public static final int WORK_QUEUE_SIZE = 200;

    /**
     * SAP HANA系统数据库类型
     */
    public static final String SYSTEM_DB_TYPE = "SystemDatabase";

    /**
     * SAP HANA租户数据库类型
     */
    public static final String TENANT_DB_TYPE = "TenantDatabase";

    /**
     * SAP HANA System ID key
     */
    public static final String SYSTEM_ID = "systemId";

    /**
     * SAP HANA系统数据库端口key
     */
    public static final String SYSTEM_DB_PORT = "systemDbPort";

    /**
     * SAP HANA hdbUserStore认证Key
     */
    public static final String HDB_USER_STORE_KEY = "hdbUserStoreKey";

    /**
     * SAP HANA开启日志备份
     */
    public static final String ENABLE_LOG_BACKUP = "enableLogBackup";

    /**
     * SAP HANA日志备份扩展信息key
     */
    public static final String LOG_BACKUP_EXT_INFO = "logBackupExtInfo";

    /**
     * SAP HANA定时日志备份时间间隔时间key
     */
    public static final String LOG_BACKUP_INTERVAL = "logBackupInterval";

    /**
     * SAP HANA定时日志备份时间间隔时间单位key
     */
    public static final String LOG_BACKUP_INTERVAL_UNIT = "logBackupIntervalUnit";

    /**
     * SAP HANA日志备份路径key
     */
    public static final String LOG_BACKUP_PATH = "logBackupPath";

    /**
     * SAP HANA日志仓持续挂载key
     */
    public static final String PERSISTENT_MOUNT = "persistentMount";

    /**
     * SAP HANA数据库类型key
     */
    public static final String SAP_HANA_DB_TYPE = "sapHanaDbType";

    /**
     * SAP HANA数据库类型key
     */
    public static final String SAP_HANA_DB_DEPLOY_TYPE = "sapHanaDbDeployType";

    /**
     * SAP HANA实例所有节点列表key
     */
    public static final String NODES = "nodes";

    /**
     * SAP HANA实例landscape id key
     */
    public static final String LANDSCAPE_ID = "landscapeId";

    /**
     * SAP HANA数据库认证信息扩展参数中实例认证信息key
     */
    public static final String INSTANCE_AUTH = "instanceAuth";

    /**
     * SAP HANA数据库扩展参数中实例的系统数据库端口key
     */
    public static final String INSTANCE_SYSTEM_DB_PORT = "instanceSystemDbPort";

    /**
     * SAP HANA操作类型key
     */
    public static final String OPERATION_TYPE = "operationType";

    /**
     * SAP HANA操作类型：连通测试
     */
    public static final String TEST_CONNECT_OPERATION_TYPE = "TestConnectivity";

    /**
     * SAP HANA操作类型：修改
     */
    public static final String MODIFY_OPERATION_TYPE = "modify";

    /**
     * 通用数据库类型的展示名称key
     */
    public static final String GENERAL_DB_TYPE_DISPLAY_KEY = "databaseTypeDisplay";

    /**
     * 通用数据库类型SAP HANA
     */
    public static final String GENERAL_DB_TYPE_SAP_HANA = "SAP HANA";

    /**
     * 通用数据库扩展参数--自定义参数customParams的key
     */
    public static final String GENERAL_DB_EXT_CUSTOM_PARAMS = "customParams";

    /**
     * 通用数据库扩展参数--关联主机ID的key
     */
    public static final String GENERAL_DB_EXT_RELATED_HOST_IDS = "relatedHostIds";
}
