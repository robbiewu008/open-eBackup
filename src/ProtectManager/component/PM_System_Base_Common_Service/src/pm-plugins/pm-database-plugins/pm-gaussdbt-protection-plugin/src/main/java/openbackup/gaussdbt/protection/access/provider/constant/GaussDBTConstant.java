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
package openbackup.gaussdbt.protection.access.provider.constant;

/**
 * GaussDBT常量
 *
 */
public class GaussDBTConstant {
    /**
     * GaussDBT注册资源上限值
     */
    public static final int GAUSSDBT_CLUSTER_MAX_COUNT = 2000;

    /**
     * nodes对应key
     */
    public static final String NODES_KEY = "nodes";

    /**
     * nodes status对应key
     */
    public static final String NODE_STATUS_KEY = "status";

    /**
     * cluster_state对应key
     */
    public static final String CLUSTER_STATE_KEY = "clusterState";

    /**
     * cluster_version对应key
     */
    public static final String CLUSTER_VERSION_KEY = "clusterVersion";

    /**
     * cluster_info对应key
     */
    public static final String CLUSTER_INFO_KEY = "clusterInfo";

    /**
     * 恢复的目标对象key
     */
    public static final String TARGET_LOCATION_KEY = "targetLocation";

    /**
     * 多文件系统key
     */
    public static final String MULTI_FILE_SYSTEM_KEY = "multiFileSystem";

    /**
     * 日志副本时间点恢复的时候时间点参数key
     */
    public static final String RESTORE_TIME_STAMP_KEY = "restoreTimestamp";

    /**
     * 挂载类型key：0-挂载非全路径 1-挂载全路径
     */
    public static final String MOUNT_TYPE_KEY = "mountType";

    /**
     * 字符串false
     */
    public static final String FALSE = "false";

    /**
     * 数据库安装运行的操作系统用户key
     */
    public static final String INSTALL_USER_KEY = "installUser";

    /**
     * GaussDBT产品发行类型key
     */
    public static final String RELEASE_TYPE_KEY = "releaseType";

    /**
     * PM下发给agent的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String IS_CHECK_BACKUP_JOB_TYPE = "isCheckBackupJobType";

    /**
     * 前端下发给PM的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String AUTO_FULL_BACKUP = "autoFullBackup";

    /**
     * 前端下发给PM的扩展字段，用于设置备份、恢复时的并发数
     */
    public static final String PARALLEL_PROCESS = "parallel_process";

    private GaussDBTConstant() {
    }
}
