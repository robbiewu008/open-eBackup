package openbackup.informix.protection.access.constant;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Informix常量类
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-09
 */
public class InformixConstant {
    /**
     * 实例状态的key
     */
    public static final String INSTANCESTATUS = "instanceStatus";

    /**
     * 日志备份开关
     */
    public static final String LOG_BACKUP = "logBackup";

    /**
     * 日志备份开关 -> 关
     */
    public static final String LOG_BACKUP_OFF = "0";

    /**
     * 日志备份路径
     */
    public static final String LOG_BACKUP_PATH = "logBackupPath";

    /**
     * 日志备份路径校验正则
     */
    public static final String LOG_BACKUP_PATH_PATTERN = "^(/[^/]{1,255})+$|^/$";

    /**
     * informix数据库版本
     */
    public static final String APPLICATION_VERSION = "applicationVersion";

    /**
     * SERVER_NUM
     */
    public static final String SERVER_NUM = "serverNum";

    /**
     * master node status
     */
    public static final String MASTER_NODE_STATUS = "On-Line (Prim)";

    /**
     * master node status : Quiescent
     */
    public static final String MASTER_STATUS_QUIESCENT = "Quiescent (Prim)";

    /**
     * second node status
     */
    public static final String SECOND_NODE_STATUS = "Read-Only (Sec)";

    /**
     * Updatable Sec status
     */
    public static final String UPDATABLE_SEC_STATUS = "Updatable (Sec)";

    /**
     * PAIRED_SERVER
     */
    public static final String PAIRED_SERVER = "pairedServer";

    /**
     * LOCAL_SERVER
     */
    public static final String LOCAL_SERVER = "localServer";

    /**
     * AGENT_IP_LIST
     */
    public static final String AGENT_IP_LIST = "agentIpList";

    /**
     * PAIRED_SERVER_IP
     */
    public static final String PAIRED_SERVER_IP = "pairedServerIp";

    /**
     * ONCONFIG_PATH
     */
    public static final String ONCONFIG_PATH = "onconfigPath";

    /**
     * SQLHOSTS_PATH
     */
    public static final String SQLHOSTS_PATH = "sqlhostsPath";

    /**
     * SUB_TYPE
     */
    public static final String SUB_TYPE = "sub_type";

    /**
     * PERSISTENT_MOUNT
     */
    public static final String PERSISTENT_MOUNT = "persistentMount";

    /**
     * MANUAL_MOUNT
     */
    public static final String MANUAL_MOUNT = "manualMount";

    /**
     * HOST_ID
     */
    public static final String HOST_ID = "hostId";

    /**
     * CLUSTER_NAME
     */
    public static final String CLUSTER_NAME = "clusterName";

    /**
     * comma 逗号
     */
    public static final String COMMA = ",";

    /**
     * ROOT_DBS_PATH
     */
    public static final String ROOT_DBS_PATH = "rootdbsPath";

    /**
     * DATA_SIZE
     */
    public static final String DATA_SIZE = "dataSize";

    /**
     * MASTER_STATUS_LIST
     */
    public static final List<String> MASTER_STATUS_LIST = Collections.unmodifiableList(
            Arrays.asList("On-Line (Prim)", "Quiescent (Prim)"));

    /**
     * 传给UBC，标识资源是否存在
     */
    public static final String RESOURCE_EXISTS = "resourceExists";
}
