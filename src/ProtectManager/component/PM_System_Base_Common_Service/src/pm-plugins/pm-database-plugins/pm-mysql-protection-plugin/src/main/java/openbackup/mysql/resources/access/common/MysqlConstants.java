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
package openbackup.mysql.resources.access.common;

/**
 * 数据库相关常量类
 *
 */
public class MysqlConstants {
    /**
     * 数据库名称
     */
    public static final String MYSQL = "MySQL";

    /**
     * PXC集群类过滤key
     */
    public static final String MYSQL_PXC = "MySQL_PXC";

    /**
     * AP集群集群类过滤key
     */
    public static final String MYSQL_AP = "MySQL_AP";

    /**
     * eapp集群类型过滤key
     */
    public static final String MYSQL_EAPP = "MySQL_EAPP";

    /**
     * PXC集群
     */
    public static final String PXC = "PXC";

    /**
     * AP集群
     */
    public static final String AP = "AP";

    /**
     * eapp集群
     */
    public static final String EAPP = "EAPP";

    /**
     * 操作系统
     */
    public static final String DEPLOY_OPERATING_SYSTEM = "deployOperatingSystem";

    /**
     * 恢复时重命名数据库
     */
    public static final String NEW_DATABASE_NAME = "newDatabaseName";

    /**
     * Mysql数据库name长度限制
     */
    public static final int DATABASE_NAME_LIMIT_LENGTH = 64;

    /**
     * Mysql启动服务的名称
     */
    public static final String MYSQL_SERVICE_NAME = "serviceName";

    /**
     * Mysql系统启动服务的名称
     */
    public static final String MYSQL_SYSTEM_SERVICE_TYPE = "systemServiceType";

    /**
     * 系统服务systemctl
     */
    public static final String SYSTEM_CTL = "systemctl";

    /**
     * Mysql的logbin.index文件路径
     */
    public static final String LOG_BIN_INDEX_PATH = "logBinIndexPath";

    /**
     * 针对封装后的版本，需要指定连接mysql服务的具体IPMysql的instance ip
     */
    public static final String INSTANCE_IP = "instanceIp";

    /**
     * master节点ip列表
     */
    public static final String MASTER_LIST = "master_list";

    /**
     * 主节点信息
     */
    public static final String MASTER_INFO = "master_info";

    /**
     * 错误码
     */
    public static final String ERROR_CODE = "error_code";

    /**
     * 本机ip列表
     */
    public static final String CURRENT_IP_LIST = "current_ip_list";

    /**
     * 即时挂载前置脚本
     */
    public static final String PRE_SCRIPT = "preScript";

    /**
     * 即时挂载后置脚本
     */
    public static final String POST_SCRIPT = "postScript";

    /**
     * 即时失败后置脚本
     */
    public static final String FAIL_POST_SCRIPT = "failPostScript";

    /**
     * 自定义my.cnf路径
     */
    public static final String MY_CNF_PATH = "myCnfPath";

    /**
     * eapp最大节点数
     */
    public static final int MAX_NODES_COUNT_FOR_EAPP = 4;
}
