/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.common;

/**
 * SQL Server常量
 *
 * @author xwx1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-23
 */
public class SqlServerConstants {
    /**
     * 资源名称分隔符
     */
    public static final String RESOURCE_NAME_SPLIT = "/";

    /**
     * 可用性组数据库列表
     */
    public static final String DATABASE = "database";

    /**
     * 可用性组实例列表
     */
    public static final String INSTANCE = "instance";

    /**
     * 可用性组名称
     */
    public static final String AG_NAME = "agName";

    /**
     * 可用性组uuid
     */
    public static final String AG_ID = "agId";

    /**
     * 集群主节点
     */
    public static final String MASTER_NODE = "masterNode";

    /**
     * 集群最小节点数
     */
    public static final int CLUSTER_INSTANCE_MIN_NODE_NUM = 1;

    /**
     * 集群最大节点数
     */
    public static final int CLUSTER_INSTANCE_MAX_NODE_NUM = 2;

    /**
     * SQL Server集群下最大资源数量
     */
    public static final int MAX_RESOURCE_COUNT = 10000;

    /**
     * SQL Server环境Windows目录正则表达式
     */
    public static final String WINDOWS_PATH_REGEX = "^[a-zA-Z]:(\\\\[\\w\\u4e00-\\u9fa5\\s]+)+";

    /**
     * SQL Server环境数据库名称正则表达式:支持数字，字母，中文，字符 _ - $，限制长度1到128之间
     */
    public static final String DATABASE_NAME_REGEX = "^[0-9a-zA-Z\\u4e00-\\u9fa5_$-]{1,128}$";

    /**
     * SQL Server恢复目标路径的key
     */
    public static final String KEY_RESTORE_NEW_LOCATION_PATH = "newDatabasePath";

    /**
     * SQL Server恢复目标重命名的key
     */
    public static final String KEY_RESTORE_NEW_LOCATION_NAME = "newDatabaseName";

    /**
     * SQL Server数据库恢复重命名不支持的名称“master”
     */
    public static final String MASTER_DATABASE = "master";

    /**
     * SQL Server数据库恢复重命名不支持的名称“model”
     */
    public static final String MODEL_DATABASE = "model";

    /**
     * SQL Server数据库恢复重命名不支持的名称“msdb”
     */
    public static final String MSDB_DATABASE = "msdb";

    /**
     * SQL Server数据库恢复重命名不支持的名称“tempdb”
     */
    public static final String TEMPDB_DATABASE = "tempdb";

    private SqlServerConstants() {
    }
}
