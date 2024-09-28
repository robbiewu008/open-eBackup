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
package openbackup.system.base.common.constants;

import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;

import com.google.common.collect.ImmutableList;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * constants
 *
 */
public class Constants {
    /**
     * CURRENT_OPERATE_USER_ID
     */
    public static final String CURRENT_OPERATE_USER_ID = "current_operate_user_id";

    /**
     * auth token.
     */
    public static final String AUTH_TOKEN = "X-Auth-Token";

    /**
     * hcs iam auth token.
     */
    public static final String HCS_IAM_TOKEN = "HCS-X-AUTH-TOKEN";

    /**
     * 预置在hcs中的账号名称
     */
    public static final String HCS_PRESET_ACCOUNT_NAME = "op_service_oceanprotect";

    /**
     * Op在cmdb云服务中的indexName
     */
    public static final String OP_IN_HCS_CMDB_INDEX_NAME = "OceanProtect";

    /**
     * agent主动上报卸载时标记
     */
    public static final String MANUAL_UNINSTALLATION = "manualUninstallation";

    /**
     * 安装agent
     */
    public static final String INSTALL_TYPE_REGISTER = "1";

    /**
     * 生成公私钥，签名使用密码
     */
    public static final String SIGN_PASS = "pass";

    /**
     * 传输路径VxLAN
     */
    public static final String VX_LAN = "1";

    /**
     * 传输路径VLAN
     */
    public static final String VLAN = "0";

    /**
     * dpa name space
     */
    public static final String DPA_NAMESPACE = "dpa";

    /**
     * network-conf config map
     */
    public static final String CM_NETWORK_CONF = "network-conf";

    /**
     * backup_net_plane
     */
    public static final String BACKUP_NET_PLANE = "backup_net_plane";

    /**
     * archive_net_plane
     */
    public static final String ARCHIVE_NET_PLANE = "archive_net_plane";

    /**
     * replication_net_plane
     */
    public static final String REPLICATION_NET_PLANE = "replication_net_plane";

    /**
     * 存在network-conf中的ips
     */
    public static final String IPS_KEY = "logic_ip_list";

    /**
     * 存在network-conf中的ip
     */
    public static final String IP = "ip";

    /**
     * 存在network-conf中的mask
     */
    public static final String MASK = "mask";

    /**
     * 存在network-conf中的name
     */
    public static final String NAME_KEY = "name";

    /**
     * 业务网络存在configmap中，节点名称对应的是nodeId
     */
    public static final String NODE_ID = "nodeId";

    /**
     * pm pod的名称
     */
    public static final String PM_ENDPOINT_NAME = "pm-system-base";

    /**
     * https请求
     */
    public static final String HTTP_URL_SCHEME = "https://";

    /**
     * 超级管理员角色ID
     */
    public static final String ROLE_SYS_ADMIN = "1";

    /**
     * 数据保护管理员角色ID
     */
    public static final String ROLE_DP_ADMIN = "2";

    /**
     * 设备管理员角色ID
     */
    public static final String ROLE_DEVICE_MANAGER = "4";

    /**
     * 审计员角色ID
     */
    public static final String ROLE_AUDITOR = "5";

    /**
     * 远程设备管理员角色ID
     */
    public static final String ROLE_RD_ADMIN = "6";

    /**
     * 灾备管理员角色ID
     */
    public static final String ROLE_DR_ADMIN = "7";

    /**
     * B版本分割版本号
     */
    public static final int SPLIT_VERSION = 37;

    /**
     * SIMPLE_DATE_FORMAT
     */
    public static final SimpleDateFormat SIMPLE_DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    /**
     * 系统初始化成功标志
     */
    public static final String SYSTEM_INITIALIZED = "system_initialized";

    /**
     * 自己创建用户标志
     */
    public static final String SYSTEM_SUPER_ALREADY_CREATE = "System_Super_Already_Create";

    /**
     * 标准备份服务状态的键
     */
    public static final String STANDARD_SERVICE_STATUS = "standard_backup";

    /**
     * SCHEDULE_JOB_GROUP
     */
    public static final String SCHEDULE_JOB_GROUP = "schedule_job_group";

    /**
     * TRIGGER_GROUP
     */
    public static final String TRIGGER_GROUP = "trigger_group";

    /**
     * 扩展字段错误信息 key
     */
    public static final String ERROR_CODE = "errorCode";

    /**
     * 扩展字段错误信息 key
     */
    public static final String ERROR_MSG = "errorMsg";

    /**
     * 扩展字段错误参数 key
     */
    public static final String ERROR_PARAM = "errorParam";

    /**
     * 吊销列表Key
     */
    public static final String CRL_KEY = "revocationlist";

    /**
     * 证书Key
     */
    public static final String CERT_KEY = "certification";

    /**
     * 资源的extendInfo里的吊销列表key，针对vmvare
     */
    public static final String REVOCATION_LIST_VMWARE = "revocation_list";

    /**
     * DMA POD名 REDIS KEY
     */
    public static final String DMA_POD_NAME_KEY = "DMA_POD_NAME_KEY";

    /**
     * session REDIS KEY前缀
     */
    public static final String LOCAL_STORAGE_SESSION_PREFIX = "LOCAL_STORAGE_SESSION_";

    /**
     * 华为云平台
     */
    public static final String HUAWEI_CLOUD_STACK = "HuaweiCloudStack";

    /**
     * resources extend info签名是否使用老的私钥
     */
    public static final String USE_OLD_PRIVATE = "use_old_private";

    /**
     * 本控session REDIS KEY
     */
    public static final String LOCAL_STORAGE_SESSION_KEY = LOCAL_STORAGE_SESSION_PREFIX + System.getenv("NODE_NAME");

    /**
     * backup network flag
     */
    public static final String BACKUP_NETWORK_FLAG = "backupNetwork";

    /**
     * archive network flag
     */
    public static final String ARCHIVE_NETWORK_FLAG = "archiveNetwork";

    /**
     * replication network flag
     */
    public static final String REPLICATION_NETWORK_FLAG = "replicationNetwork";

    /**
     * 初始化配置文件所在的k8s命名空间
     */
    public static final String NAME_SPACE = "dpa";

    /**
     * 初始化配置文件名称
     */
    public static final String CONFIG_MAP = "network-conf";

    /**
     * error status flag
     */
    public static final String INIT_ERROR_FLAG = "initializeErrorCode";

    /**
     * modify status flag
     */
    public static final String MODIFY_STATUS_FLAG = "modifyStatusFlag";

    /**
     * 用户创建的逻辑端口
     */
    public static final String LOGIC_PORTS_CREATED_BY_USER = "logicPorts";

    /**
     * 复用底座的逻辑端口
     */
    public static final String REUSE_LOGIC_PORTS = "reuseLogicPorts";

    /**
     * 执行成功
     */
    public static final int ERROR_CODE_OK = 0;

    /**
     * Builtin
     *
     */
    public static class Builtin {
        /**
         * 超级管理员角色
         */
        public static final String ROLE_SYS_ADMIN = "Role_SYS_Admin";

        /**
         * 数据保护管理员角色
         */
        public static final String ROLE_DP_ADMIN = "Role_DP_Admin";

        /**
         * 远程设备管理员角色
         */
        public static final String ROLE_RD_ADMIN = "Role_RD_Admin";

        /**
         * 灾备管理员角色
         */
        public static final String ROLE_DR_ADMIN = "Role_DR_Admin";

        /**
         * 设备管理员角色
         */
        public static final String ROLE_DEVICE_MANAGER = "Role_Device_Manager";

        /**
         * 审计员角色
         */
        public static final String ROLE_AUDITOR = "Role_Auditor";

        /**
         * ADMIN_AUDITOR
         */
        public static final List<String> ADMIN_AUDITOR = Arrays.asList(ROLE_SYS_ADMIN, ROLE_AUDITOR);

        /**
         * GET_USER_ADMIN_AUDITOR
         */
        public static final List<String> GET_USER_ADMIN_AUDITOR =
            Collections.unmodifiableList(Arrays.asList(ROLE_SYS_ADMIN, ROLE_AUDITOR, ROLE_RD_ADMIN));

        /**
         * LDAP支持的用户角色
         */
        public static final List<String> LDAP_SUPPORT_ROLES =
            Collections.unmodifiableList(Arrays.asList(ROLE_SYS_ADMIN, ROLE_DP_ADMIN, ROLE_AUDITOR));

        /**
         * 支持CLEAN RESOURCE选项的用户角色
         */
        public static final List<String> CLEAN_RESOURCE_ROLES =
            Collections.unmodifiableList(Arrays.asList(ROLE_SYS_ADMIN, ROLE_DP_ADMIN));

        /**
         * 默认内置角色集合
         */
        public static final List<String> DEFAULT_BUILT_IN_ROLES_LIST = ImmutableList.of(Constants.ROLE_SYS_ADMIN,
            Constants.Builtin.AUDITOR_ROLE_ID, Constants.ROLE_DEVICE_MANAGER, Constants.ROLE_DR_ADMIN,
            Constants.ROLE_RD_ADMIN);

        /**
         * E1000系列需要屏蔽的角色id集合
         */
        public static final List<String> E1000_NONE_VIEW_ROLES_LIST = ImmutableList.of(Constants.ROLE_DR_ADMIN);

        /**
         * mmdp_admin用户id
         */
        public static final String MMDP_ADMIN_USER_ID = "88a94c476f12a21e016f12a246e50010";

        /**
         * mm_audit用户id
         */
        public static final String MM_AUDIT_USER_ID = "88a94c476f12a21e016f12a246e50011";

        /**
         * cluster_admin用户id
         */
        public static final String CLUSTER_ADMIN_USER_ID = "88a94c476f12a21e016f12a246e50013";

        /**
         * mmdp_admin用户名
         */
        public static final String MMDP_ADMIN_USER_NAME = "mmdp_admin";

        /**
         * mm_audit用户名
         */
        public static final String MM_AUDIT_USER_NAME = "mm_audit";

        /**
         * cluster_admin用户名
         */
        public static final String CLUSTER_ADMIN_USER_NAME = "cluster_admin";

        /**
         * 数据保护管理员角色id
         */
        public static final String DP_ADMIN_ROLE_ID = "2";

        /**
         * 审计员角色id
         */
        public static final String AUDITOR_ROLE_ID = "5";

        /**
         * 系统管理员角色id
         */
        public static final String SYS_ADMIN_ROLE_ID = "1";

        /**
         * guassdb 通信网络平面
         */
        public static final String GAUSSDB_NET_PLANE_NAME = "gaussdb_internal_communicate_net_plane";

        /**
         * infrastrucutre 通信网络平面
         */
        public static final String INFRA_NET_PLANE_NAME = "infrastructure_internal_communicate_net_plane";

        /**
         * sftp 通信网络平面
         */
        public static final String SFTP_NET_PLANE_NAME = "sftp_net_plane";

        /**
         * 只分域的资源类型
         */
        public static final List<String> ONLY_IN_DOMAIN_RESOURCE_TYPE_LIST = ImmutableList.of(
            ResourceSetTypeEnum.JOB.getType(), ResourceSetTypeEnum.JOB_LOG.getType(),
            ResourceSetTypeEnum.ALARM.getType(), ResourceSetTypeEnum.EVENT.getType(),
            ResourceSetTypeEnum.KERBEROS.getType());
    }
}
