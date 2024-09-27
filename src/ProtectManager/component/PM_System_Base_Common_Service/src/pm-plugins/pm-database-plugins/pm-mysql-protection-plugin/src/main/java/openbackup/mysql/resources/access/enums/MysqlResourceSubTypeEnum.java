package openbackup.mysql.resources.access.enums;

import openbackup.system.base.common.utils.VerifyUtil;

/**
 * MySQL子资源sub type
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/5/31
 */
public enum MysqlResourceSubTypeEnum {
    /**
     * MySQL集群
     */
    MYSQL_CLUSTER("MySQL-cluster"),

    /**
     * MySQL实例
     */
    MYSQL_SINGLE_INSTANCE("MySQL-instance"),

    /**
     * MySQL集群实例
     */
    MYSQL_CLUSTER_INSTANCE("MySQL-clusterInstance"),

    /**
     * MySQL数据库
     */
    MYSQL_DATABASE("MySQL-database");

    private final String type;

    /**
     * MySQL子资源sub type
     *
     * @param type 类型
     */
    MysqlResourceSubTypeEnum(String type) {
        this.type = type;
    }

    /**
     * getter
     *
     * @return 类型
     */
    public String getType() {
        return type;
    }

    /**
     * 判断目标资源的subType是否属于MySQL的一种资源
     *
     * @param subType 任务的subType
     * @return 是否属于MySQL资源的一种
     */
    public static boolean isBelongToMysql(String subType) {
        if (VerifyUtil.isEmpty(subType)) {
            return false;
        }
        MysqlResourceSubTypeEnum[] mysqlResourceSubTypeEnums = MysqlResourceSubTypeEnum.values();
        for (MysqlResourceSubTypeEnum mysqlResourceSubTypeEnum : mysqlResourceSubTypeEnums) {
            if (subType.equals(mysqlResourceSubTypeEnum.getType())) {
                return true;
            }
        }
        return false;
    }
}
