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
package openbackup.exchange.protection.access.constant;

/**
 * Exchange 常量
 *
 */
public class ExchangeConstant {
    /**
     * 常数项 0
     */
    public static final int INT_ZERO = 0;

    /**
     * 查询每页大小
     */
    public static final int QUERY_SIZE = 100;

    /**
     * Exchange agents
     */
    public static final String EXCHANGE_AGENTS = "agents";

    /**
     * Exchange 单机
     */
    public static final String EXCHANGE_SINGLE = "0";

    /**
     * Exchange 集群
     */
    public static final String EXCHANGE_GROUP = "1";

    /**
     * 备份类型为全量备份
     */
    public static final String FULL_BACKUP = "fullBackup";

    /**
     * PM下发给agent的扩展字段，用于判断日志备份是否需要转全量
     */
    public static final String IS_CHECK_BACKUP_JOB_TYPE = "isCheckBackupJobType";

    /**
     * 日志备份类型
     */
    public static final String LOG_BACKUP_TYPE = "logBackup";

    /**
     * 细粒度恢复表名
     */
    public static final String DATABASE_NAME = "databaseName";

    /**
     * 环境名称正则
     */
    public static final String NAME_FORMAT = "^[\\u4E00-\\u9FA5A-Za-z_][\\u4E00-\\u9FA5A-Za-z0-9-_]*$";

    /**
     * rootUuid
     */
    public static final String ROOT_UUID = "rootUuid";

    /**
     * rootUuid
     */
    public static final String SUB_TYPE = "subType";

    /**
     * rootUuid
     */
    public static final String EXT_DATABASE_NAME = "DatabaseName";

    /**
     * PAGE_SIZE
     */
    public static final int PAGE_SIZE = 500;

    /**
     * 注册的扩展参数，资源上最大任务并发数，1-20
     */
    public static final String MAX_CONCURRENT_JOB_NUMBER = "maxConcurrentJobNumber";

    /**
     * 最小任务数，1
     */
    public static final int MIN_JOB_NUMBER = 1;

    /**
     * 最大任务数，10
     */
    public static final int MAX_JOB_NUMBER = 10;

    /**
     * Exchange注册资源上限值
     */
    public static final int EXCHANGE_MAX_COUNT = 20000;

    /**
     * nodes对应key
     */
    public static final String NODES = "nodes";

    /**
     * uuid和实例端口分隔符
     */
    public static final String UUID_INSTANCE_PORT_SPLIT_CHAR = "_";

    /**
     * 注册Exchange数据库可用性组时，由于成员服务器已经被注册，操作失败
     */
    public static final long CLUSTER_NODE_IS_REGISTERED = 1577213576L;
}
