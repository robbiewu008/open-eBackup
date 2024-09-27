/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.db2.protection.access.constant;

/**
 * db2常量
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-05
 */
public class Db2Constants {
    /**
     * db2表空间集的最大规格
     */
    public static final int TABLESPACE_SPECIFICATION = 500;

    /**
     * db2编目节点ip的建值
     */
    public static final String CATALOG_IP_KEY = "catalogIp";

    /**
     * db2集群允许接入的最大规格数
     */
    public static final int DB2_CLUSTER_MAX_COUNT = 100;

    /**
     * DPF集群允许最大节点规格数
     */
    public static final int DPF_NODE_MAX_COUNT = 100;

    /**
     * powerHA集群允许最大节点规格数
     */
    public static final int POWER_HA_NODE_MAX_COUNT = 2;

    /**
     * HADR集群允许最大节点规格数
     */
    public static final int HADR_NODE_MAX_COUNT = 4;

    /**
     * db2节点上数据库的健值
     */
    public static final String NODE_DATABASE_KEY = "nodeDatabase";

    /**
     * db2 hadr角色的健值
     */
    public static final String HADR_ROLE_KEY = "HADR_ROLE";

    /**
     * db2 hadr本机ip的健值
     */
    public static final String HADR_LOCAL_HOST_KEY = "HADR_LOCAL_HOST";

    /**
     * db2 hadr远端ip的健值
     */
    public static final String HADR_REMOTE_HOST_KEY = "HADR_REMOTE_HOST";

    /**
     * 数据量大小的健值
     */
    public static final String DATA_SIZE_KEY = "dataSize";
}
