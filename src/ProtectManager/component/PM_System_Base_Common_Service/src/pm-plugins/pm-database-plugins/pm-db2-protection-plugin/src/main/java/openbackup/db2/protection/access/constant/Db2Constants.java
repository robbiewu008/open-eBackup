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
package openbackup.db2.protection.access.constant;

/**
 * db2常量
 *
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

    /**
     * 前端下发给PM的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String AUTO_FULL_BACKUP = "autoFullBackup";

    /**
     * PM下发给agent的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String IS_CHECK_BACKUP_JOB_TYPE = "isCheckBackupJobType";

    /**
     * END_TIME
     */
    public static final String END_TIME = "endTime";

    /**
     * BEGIN_TIME
     */
    public static final String BEGIN_TIME = "beginTime";

    /**
     * AGENT_ID
     */
    public static final String UUID = "uuid";

    /**
     * extendInfo
     */
    public static final String EXTENDINFO = "extendInfo";

    /**
     * clusterType
     */
    public static final String CLUSTERTYPE = "clusterType";

    /**
     * deleteLog
     */
    public static final String DELETELOG = "deleteLog";

    /**
     * false
     */
    public static final String FALSE = "false";

    /**
     * databaseName
     */
    public static final String DATABASENAME = "databaseName";

    /**
     * userName
     */
    public static final String USERNAME = "userName";

    /**
     * cachePath
     */
    public static final String CACHEPATH = "cachePath";

    /**
     * sinceTimeStamp
     */
    public static final String SINCETIMESTAMP = "sinceTimeStamp";
}
