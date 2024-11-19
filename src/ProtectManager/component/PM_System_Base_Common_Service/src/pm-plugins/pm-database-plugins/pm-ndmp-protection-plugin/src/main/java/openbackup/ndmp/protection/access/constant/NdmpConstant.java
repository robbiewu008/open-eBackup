/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.ndmp.protection.access.constant;

/**
 * NDMP 常数类
 *
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
     * 租户名称
     */
    public static final String TENANT_NAME = "tenantName";


    /**
     * 完整名称
     */
    public static final String FULL_NAME = "fullName";

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

    /**
     * 备份网络平面ip
     */
    public static final String MULTI_IP = "multiIp";

    /**
     * 是否是文件系统标识
     */
    public static final String IS_FILE_SYSTEM = "isFs";

    /**
     * NDMP目录标识
     */
    public static final String DIR = "0";

    /**
     * 注册的扩展参数，资源上最大任务并发数，1-20, 默认为8
     */
    public static final String MAX_CONCURRENT_JOB_NUMBER = "maxConcurrentJobNumber";

    /**
     * 目标位置
     */
    public static final String TARGET_ENV = "targetEnv";

    /**
     * uuid
     */
    public static final String UUID = "uuid";

    /**
     * ndmp类型
     */
    public static final String NDMP_TYPE = "ndmpType";

    /**
     * type
     */
    public static final String TYPE = "type";
}
