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
package openbackup.dameng.protection.access.constant;

/**
 * Dameng常量
 *
 */
public class DamengConstant {
    /**
     * Dameng注册资源上限值
     */
    public static final int DAMENG_CLUSTER_MAX_COUNT = 2000;

    /**
     * 实例状态的key
     */
    public static final String INSTANCESTATUS = "instanceStatus";

    /**
     * 节点角色类型的key 1->主 2->备
     */
    public static final String ROLE = "role";

    /**
     * 日志备份类型
     */
    public static final String LOG_BACKUP_TYPE = "logBackup";

    /**
     * 主节点的key
     */
    public static final String PRIMARY = "1";

    /**
     * uuid和实例端口分隔符
     */
    public static final String UUID_INSTANCE_PORT_SPLIT_CHAR = "_";

    /**
     * nodes对应key
     */
    public static final String NODES = "nodes";

    /**
     * 认证类型
     */
    public static final String AUTH_TYPE = "authType";

    /**
     * 数据库路径
     */
    public static final String DB_PATH = "dbPath";

    /**
     * 数据库监听端口
     */
    public static final String DB_PORT = "dbPort";

    /**
     * dm.ini文件路径
     */
    public static final String DM_INI_PATH = "dminiPath";

    /**
     * 数据库名称
     */
    public static final String DB_NAME = "dbName";

    /**
     * 集群的组Id
     */
    public static final String GROUP_ID = "groupId";

    /**
     * dameng版本
     */
    public static final String VERSION = "version";

    /**
     * dameng大版本，DTS2023122504169处理此问题单特殊场景
     */
    public static final String BIG_VERSION = "bigVersion";

    /**
     * 扩展信息
     */
    public static final String EXTEND_INFO = "extendInfo";

    /**
     * 恢复的目标对象key
     */
    public static final String TARGET_LOCATION_KEY = "targetLocation";

    /**
     * 多节点执行
     */
    public static final String MULTI_POST_JOB = "multiPostJob";

    /**
     * 端口号最大值
     */
    public static final int DAMENG_NODE_MAX_PORT = 65535;

    /**
     * 数据库密码最大长度
     */
    public static final int DATABASE_PWD_MAX_LENGTH = 48;
}
