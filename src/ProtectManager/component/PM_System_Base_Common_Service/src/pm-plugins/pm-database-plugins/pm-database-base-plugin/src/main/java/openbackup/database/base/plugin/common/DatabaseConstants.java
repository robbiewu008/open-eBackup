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
package openbackup.database.base.plugin.common;

/**
 * 数据库相关常量类
 *
 */
public class DatabaseConstants {
    /**
     * SLA key
     */
    public static final String SLA_KEY = "backupTask_sla";

    /**
     * mysql 数据库dataDir：mariaDB不需要配置这个，所以恢复的时候是无法从配置文件里读取的，只能PM下发
     */
    public static final String DATA_DIR = "dataDir";

    /**
     * 父资源uuid
     */
    public static final String PARENT_UUID = "parentUuid";

    /**
     * 认证状态
     */
    public static final String AUTH_STATUS = "authStatus";

    /**
     * 已认证
     */
    public static final String AUTHENTICATED = "1";

    /**
     * 未认证
     */
    public static final String UN_AUTHENTICATED = "0";

    /**
     * 状态成功
     */
    public static final String SUCCESS = "SUCCESS";

    /**
     * 状态失败
     */
    public static final String FAIL = "FAIL";

    /**
     * 根资源uuid
     */
    public static final String ROOT_UUID = "rootUuid";

    /**
     * 可用性组依赖instance
     */
    public static final String INSTANCE = "instance";

    /**
     * 组
     */
    public static final String GROUP = "group";

    /**
     * 扩展信息
     */
    public static final String EXTEND_INFO = "extendInfo";

    /**
     * 备份任务类型
     */
    public static final String BACKUP = "BACKUP";

    /**
     * job任务排序字段
     */
    public static final String START_TIME = "start_time";

    /**
     * job任务结束时间字段
     */
    public static final String END_TIME = "end_time";

    /**
     * 上一次任务状态
     */
    public static final String PRE_BACKUP_JOB_STATUS = "preBackupJobStatus";

    /**
     * 实例端口
     */
    public static final String INSTANCE_PORT = "instancePort";

    /**
     * 编码格式
     */
    public static final String CHARSET = "charset";

    /**
     * 业务ip
     */
    public static final String SERVICE_IP = "serviceIp";

    /**
     * 主机ip
     */
    public static final String AGENTS_IP = "agentsIp";

    /**
     * 端口
     */
    public static final String PORT = "port";

    /**
     * 集群类型
     */
    public static final String CLUSTER_TYPE = "clusterType";

    /**
     * 资源uuid
     */
    public static final String UUID = "uuid";

    /**
     * 环境IP
     */
    public static final String END_POINT = "endpoint";

    /**
     * children key
     */
    public static final String CHILDREN = "children";

    /**
     * delete children key
     */
    public static final String DELETE_CHILDREN = "-children";

    /**
     * agents key
     */
    public static final String AGENTS = "agents";

    /**
     * 成功状态码
     */
    public static final int SUCCESS_CODE = 0;

    /**
     * 日志备份类型
     */
    public static final String LOG_BACKUP_TYPE = "logBackup";

    /**
     * 全量备份类型
     */
    public static final String FULL_BACKUP_TYPE = "fullBackup";

    /**
     * 差异备份类型
     */
    public static final String DIFF_BACKUP_DIFF = "diffBackup";

    /**
     * 虚拟IP
     */
    public static final String VIRTUAL_IP = "virtualIp";

    /**
     * 分隔符“,”
     */
    public static final String SPLIT_CHAR = ",";

    /**
     * 部署类型
     */
    public static final String DEPLOY_TYPE = "deployType";

    /**
     * 多节点任务
     */
    public static final String MULTI_POST_JOB = "multiPostJob";

    /**
     * 字动扫描配置文件路径
     */
    public static final String AUTO_SCAN_PATH = "functions.scan.auto-scan";

    /**
     * 资源类型
     */
    public static final String RESOURCE_TYPE = "type";

    /**
     * 资源字类型
     */
    public static final String SUB_TYPE = "subType";

    /**
     * 主机uuid
     */
    public static final String HOST_ID = "hostId";

    /**
     * 数据库id
     */
    public static final String DATABASE_ID = "databaseId";

    /**
     * 默认查询size
     */
    public static final int PAGE_SIZE = 100;

    /**
     * 所有节点信息key
     */
    public static final String ALL_NODES = "allNodes";

    /**
     * 主机名称
     */
    public static final String HOST_NAMES = "hostNames";

    /**
     * 资源名称
     */
    public static final String NAME = "name";

    /**
     * 服务状态
     */
    public static final String STATE = "state";

    /**
     * IP port 分隔符
     */
    public static final String IP_PORT_SPLIT_CHAR = ":";

    /**
     * 是否顶级子实例
     */
    public static final String IS_TOP_INSTANCE = "isTopInstance";

    /**
     * 区分子实例是单实例还是集群实例下的子实例，1代表这是单实例
     */
    public static final String TOP_INSTANCE = "1";

    /**
     * 节点角色
     */
    public static final String ROLE = "role";

    /**
     * 版本
     */
    public static final String VERSION = "version";

    /**
     * 版本
     */
    public static final String ORACLE_VERSION = "database_version";

    /**
     * 分片节点数
     */
    public static final String NODE_COUNT = "nodeCount";

    /**
     * 是否主节点key
     */
    public static final String IS_MASTER = "is_master";

    /**
     * 数据目录
     */
    public static final String DATA_DIRECTORY = "dataDirectory";

    /**
     * 连接状态
     */
    public static final String LINK_STATUS_KEY = "linkStatus";

    /**
     * 副本格式为目录
     */
    public static final int DIRECTORY = 1;

    /**
     * 多文件系统的value 默认值为ture
     */
    public static final String MULTI_FILE_SYSTEM_VALUE_ENABLE = "true";

    /**
     * 多文件系统的value 默认值为ture
     */
    public static final String EXTEND_INFO_KEY_IS_LOCKED = "isLocked";

    /**
     * 副本生成方式
     */
    public static final String COPY_GENERATED_BY = "generated_by";

    /**
     * 副本保护对象版本key
     */
    public static final String COPY_PROTECT_OBJECT_VERSION_KEY = "copyProtectObjectVersion";

    /**
     * 恢复目标位置类型key
     */
    public static final String TARGET_LOCATION_KEY = "targetLocation";

    /**
     * 数据库重命名
     */
    public static final String DATABASE_NEW_NAME = "newName";

    /**
     * 不开启worm key名称
     */
    public static final String FORBID_WORM_FILE_SYSTEM = "forbidWormFileSystem";

    /**
     * 扩展信息 extendInfo 中 kerberosId 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_KERBEROS_ID = "kerberosId";

    /**
     * 扩展信息 extendInfo 中 sslEnable 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_SSL_ENABLE = "sslEnable";

    /**
     * 状态
     */
    public static final String STATUS = "status";

    /**
     * Kerberos的 keytab 认证方式
     */
    public static final String KERBEROS_CONFIG_MODEL = "configModel";

    /**
     * Kerberos的 keytab 认证方式
     */
    public static final String KERBEROS_MODEL_KEYTAB = "keytab_model";

    /**
     * Kerberos的 keytab 认证方式
     */
    public static final String PASSWORD_MODEL = "password_model";

    /**
     * krb5.conf文件名
     */
    public static final String KERBEROS_KRB5_CONF = "krb5Conf";

    /**
     * keytab文件名
     */
    public static final String KERBEROS_KEYTAB_FILE = "keytab";

    /**
     * kerberos认证密码key
     */
    public static final String KERBEROS_SECRET_KEY = "secret";

    /**
     * 框架的 environment 扩展信息 extendInfo 中 principal 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_PRINCIPAL = "principal";

    /**
     * 集群类型
     */
    public static final String CLUSTER_TARGET = "cluster";

    /**
     * 集群节点类型
     */
    public static final String NODE_TARGET = "node";

    /**
     * 部署的操作系统
     */
    public static final String DEPLOY_OS_KEY = "deployOperatingSystem";

    /**
     * 数据库位数
     */
    public static final String DATABASE_BITS_KEY = "databaseBits";

    /**
     * 表空间健值
     */
    public static final String TABLESPACE_KEY = "table";

    /**
     * 实例的id健值
     */
    public static final String INSTANCE_UUID_KEY = "instanceUuid";

    /**
     * 集群实例节点数健值
     */
    public static final String NODE_NUMS_KEY = "nodeNums";

    /**
     * 备份类型的健值
     */
    public static final String BACKUP_TYPE_KEY = "backupType";

    /**
     * 时间点恢复的时候时间点参数key
     */
    public static final String RESTORE_TIME_STAMP_KEY = "restoreTimestamp";

    /**
     * 副本备份类型的健值
     */
    public static final String COPY_BACKUP_TYPE_KEY = "backup_type";

    /**
     * 副本备份类型的健值
     */
    public static final String COPY_PARENT_NAME_KEY = "parent_name";

    /**
     * 副本备份时间的健值
     */
    public static final String COPY_BACKUP_TIME_KEY = "backupTime";

    /**
     * 日志副本开始时间的健值
     */
    public static final String LOG_COPY_BEGIN_TIME_KEY = "beginTime";

    /**
     * 数据库模式key
     */
    public static final String DB_MODE_KEY = "databaseMode";

    /**
     * VPC信息key
     */
    public static final String VPC_INFO_KEY = "vpc_info";

    /**
     * 使用过的存储资源
     */
    public static final String USED_STORAGE_KEY = "used_storages";

    /**
     * 使用过的存储资源认证信息
     */
    public static final String USED_STORAGE_PWD_KEY = "used_storages_pwd";

    /**
     * 存储资源认证信息
     */
    public static final String STORAGE_PWD_KEY = "storagesPwd";

    /**
     * 存储资源信息
     */
    public static final String STORAGES = "storages";

    /**
     * 恢复时，副本是否需要可写，除 DWS 之外，所有数据库应用都设置为 True。该字段默认为 False，字段不存在也为 False。
     */
    public static final String IS_COPY_RESTORE_NEED_WRITABLE = "isCopyRestoreNeedWritable";
}