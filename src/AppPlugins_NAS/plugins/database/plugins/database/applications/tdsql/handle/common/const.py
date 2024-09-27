#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

from enum import Enum


class TdsqlConstant:
    pass


class ClusterNodeCheckType:
    """
    集群节点校验类型
    """
    CHECK_NODE = "check_node"
    CHECK_MANAGE_NODE_NUM = "check_manage_node_num"
    CHECK_GROUP_INFO = "check_group_info"
    CHECK_GROUP_DATA_NODE = "check_group_data_node"


class TDSQLNodeService:
    """
    各个服务的列表
    """
    SERVICE_DICT = {"ossNode": "oss_server", "schedulerNode": "scheduler", "dataNode": "mysqld"}


class TDSQLResourceType:
    """
    集群资源类型
    """
    INSTANCE = "0"
    GROUP = "1"


class TDSQLResourceInterface:
    """
    oss接口列表
    """
    INTERFACE_DICT = {
        TDSQLResourceType.INSTANCE: {"interface": "TDSQL.GetInstance", "paraName": "instance"},
        TDSQLResourceType.GROUP: {"interface": "TDSQL.GetGroup", "paraName": "groups"}
    }


class TDSQLSubType:
    """
    TDSQL涉及的类型
    """
    TYPE = "Database"
    SUBTYPE_CLUSTER = "TDSQL-cluster"
    SUBTYPE_CLUSTER_INSTANCE = "TDSQL-clusterInstance"
    SUBTYPE_CLUSTER_GROUP = "TDSQL-clusterGroup"


class ErrorCode(int, Enum):
    # 错误码编号待定
    # 集群节点数不一致
    ERROR_DIFFERENT_TOPO = 1577209972
    # 数据库用户名不一致
    ERROR_DIFFERENT_USER = 1577209973
    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210000
    # 某个服务未正常开启，返回参数 服务名称
    ERROR_SERVICE = 1577213479
    # 节点的业务ip地址不属于所选的代理主机
    ERROR_IP_NOT_MATCH_AGENT = 1677930028
    # oss端口错误
    ERROR_OSS_PORT = 1677930052
    # oss业务IP地址错误
    ERROR_OSS_IP_ADDRESS = 1677873227
    # 数据节点conf文件路径错误
    ERROR_CONF_PATH = 1677931343
    # 数据节点socket文件路径错误
    ERROR_SOCKET_PATH = 1677931342
    # 参数错误
    ERROR_PARAM = 1677929218
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    # 检查TDSQL用户权限不足
    CHECK_PRIVILEGE_FAILED = 1677873187
    # set分片状态异常
    ERROR_SET_IS_NOT_ONLINE = 1677947143
    # 注册的分片信息和实例实际的分片信息不一致
    ERROR_SETS_NOT_MATCH = 1677947145
    # 注册的数据节点信息和实例实际的数据节点不一致
    ERROR_DATANODES_NOT_MATCH = 1677947147


class TDSQLResourceKeyName:
    """
    TDSQL资源接入用户key
    """
    APPLICATION_AUTH_AUTHKEY = "application_auth_authKey_"
    APPLICATION_AUTH_AUTHPWD = "application_auth_authPwd_"
    LIST_APPLICATION_AUTH_AUTHKEY = "applications_0_auth_authKey_"
    LIST_APPLICATION_AUTH_AUTHPWD = "applications_0_auth_authPwd_"


class TDSQLProtectKeyName:
    """
    TDSQL保护用户key
    """
    PROTECTENV_AUTH_AUTHKEY = "job_protectEnv_auth_authKey_"
    PROTECTENV_AUTH_AUTHPWD = "job_protectEnv_auth_authPwd_"
    IAM_USERNAME_RESTORE = "job_targetEnv_auth_authKey_"
    IAM_PASSWORD_RESTORE = "job_targetEnv_auth_authPwd_"


class TDSQLDataNodeStatus:
    """
    TDSQL数据节点在线状态
    """
    # 在线
    ONLINE = "1"
    # 离线
    OFFLINE = "0"


class TDSQLGroupSetStatus:
    """
    TDSQL分布式实例set在线状态
    """
    # 在线
    ONLINE = 0


class TDSQLGroupBackupTaskStatus:
    """
    TDSQL分布式实例备份任务状态
    """
    # 成功
    SUCCEED = 0
    RUNNING = 2
    FAILED = 1


class TDSQLRestoreTaskStatus:
    """
    TDSQL分布式/非分布式实例恢复任务状态
    """
    # 成功
    SUCCEED = 0
    RUNNING = 2
    FAILED = 1


class TDSQLReportLabel:
    # 分布式实例全量备份提示：未检测到zkmeta，会导致跨集群恢复数据库失败，如需跨集群恢复请先开启zkmeta自动备份功能，并重新生成副本。
    CHECK_ZKMETA_NOT_EXIT_LABEL = "job_tdsql_check_zkmeta_not_exit_label"
    # 分布式实例全量备份任务失败，提示：如果执行ddl操作，可能导致备份失败
    TDSQL_GROUP_FULL_BACKUP_FAIL_LABEL = "job_tdsql_group_full_backup_fail_label"
    # 分布式实例跨集群恢复检查zkmeta，如果不存在，提示：副本没有zkmeta，不支持跨集群恢复
    RESTORE_CHECK_ZKMETA_NOT_EXIT_LABEL = "job_tdsql_restore_zkmeta_not_exit_label"
    # 分布式实例恢复失败，提示：恢复失败，原因：{0}。恢复过程中会新建实例，请到赤兔管理台手动销毁。
    TDSQL_GROUP_RESTORE_FAIL_LABEL = "job_tdsql_restore_fail_label"
    # 非分布式实例备份，所有主机离线提示：数据节点全部离线，请确保备份节点在线后重试。
    CHECK_IS_ALL_NODE_NOT_ALIVE = "job_tdsql_check_is_all_node_not_alive"
    # 节点({0})的TDSQL业务IP地址不属于所选的代理主机，请确保节点的TDSQL业务IP地址属于所选的代理主机。
    TDSQL_CHECK_HOST_AGENT_NOT_MATCH = "job_tdsql_check_host_agent_not_match"
    # 该实例暂未生成新日志文件，建议降低SLA备份配置里的日志备份频率后重试。
    TDSQL_GROUP_LOG_BACKUP_FAIL_LABEL = "job_tdsql_group_log_backup_fail_label"
    # 副本和待恢复的实例版本不匹配。
    TDSQL_INSTANCE_RESTORE_VERSION_CHECK_FAIL_LABEL = "job_tdsql_instance_restore_version_check_fail_label"
    # 创建非分布式新实例({0})成功
    TDSQL_CREATE_NEW_INSTANCE_SUCCESS_LABEL = "job_tdsql_create_new_instance_success_label"
    # 创建非分布式新实例失败，原因：{0}。请确保磁盘空间充足并检查新建实例的配置信息。
    TDSQL_CREATE_NEW_INSTANCE_FAIl_LABEL = "job_tdsql_create_new_instance_fail_label"
    # 恢复失败，可能原因：/data/oc_agent 所在磁盘剩余空间不足。建议清理磁盘空间或扩容后重试。
    TDSQL_ORIGINAL_LOCATION_RESTORE_FAIL = "job_tdsql_original_location_restore_fail_label"


class TDSQLQueryType:
    """
    TDSQL分布式请求集群主机和支持的机型
    """
    RESOURCE = "resource"
