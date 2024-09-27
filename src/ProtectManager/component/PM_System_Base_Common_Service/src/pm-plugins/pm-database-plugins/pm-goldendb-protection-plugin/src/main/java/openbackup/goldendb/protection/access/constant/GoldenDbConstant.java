package openbackup.goldendb.protection.access.constant;

/**
 * 功能描述 Golden常量
 *
 * @author s30036254
 * @since 2023-02-14
 */
public class GoldenDbConstant {
    /**
     * 取json里面的GoldenDB
     */
    public static final String GOLDEN_CLUSTER = "GoldenDB";

    /**
     * 取json里面的clusterInfo
     */
    public static final String CLUSTER_INFO = "clusterInfo";

    /**
     * 取json里面的gtm
     */
    public static final String GTM = "gtm";

    /**
     * GoldenDB版本
     */
    public static final String VERSION = "version";

    /**
     * GoldenDB版本
     */
    public static final String SEPARATOR = ",";

    /**
     * 插件指定PM需要捕获的错误码
     */
    public static final long MISS_COMPONENT = 1577213479L;

    /**
     * 插件指定PM需要捕获的错误码
     */
    public static final long NODE_TYPE_MISMATCH = 1577209938L;

    /**
     * subType
     */
    public static final String SUBTYPE = "subType";

    /**
     * 父资源uuid
     */
    public static final String PARENT_UUID = "parentUuid";

    /**
     * 参数
     */
    public static final String PARAMETERS = "parameters";

    /**
     * 截取node长度
     */
    public static final int SUB_NODE_LENGTH = 4;

    /**
     * 开始时间
     */
    public static final String START_TIME = "start_time";

    /**
     * 降序
     */
    public static final String DESC_ORDER_TYPE = "desc";

    /**
     * 备份
     */
    public static final String BACKUP = "BACKUP";

    /**
     * 恢复
     */
    public static final String RESTORE = "RESTORE";

    /**
     * 备份label
     */
    public static final String BACKUP_LABEL = "common_backup_label";

    /**
     * 恢复label
     */
    public static final String RESTORE_LABEL = "common_restore_label";

    /**
     * agent ip 列表
     */
    public static final String AGENT_IP_LIST = "agentIpList";

    /**
     * 实例id
     */
    public static final String INSTANCE_ID = "id";

    /**
     * 支持的最低版本
     */
    public static final String LOWEST_VERSION = "6";

    /**
     * 管理节点类型
     */
    public static final String MANAGE_TYPE = "MANAGE";
}
