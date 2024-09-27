/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.constant;

/**
 * NDMP 常数类
 *
 * @author t30021437
 * @since 2023-05-06
 */
public class NdmpConstant {
    /**
     * 允许接入的NDMP集群规格上限
     */
    public static final int GAUSSDB_CLUSTER_MAX_COUNT = 8;

    /**
     * 常数项 0
     */
    public static final int INT_ZERO = 0;

    /**
     * 查询每页大小
     */
    public static final int QUERY_SIZE = 100;

    /**
     * NDMPagents
     */
    public static final String AGENTS = "agents";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_STATE = "cluster_state";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_FILE_SETS = "fileSets";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServer 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_SERVER = "ndmpServer";

    /**
     * NDMP的使用状态 扩展信息 中 ndmpServer 状态的 Key 名称
     */
    public static final String STATUS_OPEN = "1";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServer 状态的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_SERVER_STATUS_OPEN = "1";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServer 状态的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_SERVER_STATUS_CLOSE = "0";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServerGap 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_GAP = "ndmpServerGap";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServerUserName 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_NAME = "ndmpServerUserName";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServerIp 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_IP = "ndmpServerIp";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_VERSION = "version";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 ndmpServerPwd 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_USER_PASS = "ndmpServerPwd";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String VERSION = "version";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 port 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_NDMP_PORT = "port";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 speedStatistics 的 Key 名称
     */
    public static final String SPEED_STATISTICS = "speedStatistics";

    /**
     * NDMP的使用状态 扩展信息 extendInfo 中 esn 的 Key 名称
     */
    public static final String REPOSITORIES_KEY_ENS = "esn";

    /**
     * repository role 角色 master
     */
    public static final int MASTER_ROLE = 0;

    /**
     * repository role 角色 master
     */
    public static final String NDMP_SRC = "ndmpSrc";

    /**
     * repository role 角色 master
     */
    public static final String NDMP_DST = "ndmpDst";

    /**
     * repository role 角色 master
     */
    public static final String AUTH_KEY = "authKey";

    /**
     * repository role 角色 master
     */
    public static final String AUTH_PASS = "authPwd";

    /**
     * checkApplication 的返回正确信息
     */
    public static final String SUCCESS = "0";

    /**
     * DORADO_FORMAT Dorado设备
     */
    public static final String DORADO_FORMAT = "DoradoV6";

    /**
     * NET_APP NETApp设备
     */
    public static final String NET_AP = "NetApp";

    /**
     * STORAGE_TYPE NDMP接入设备类型标识
     * 0 Dorado
     * 1 NETAPP
     */
    public static final String STORAGE_TYPE = "storageType";

    /**
     * COPY_FORMAT 副本格式
     */
    public static final String COPY_FORMAT = "copyFormat";
}
