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
from dataclasses import dataclass
from pydantic import BaseModel, Field
from common.env_common import get_install_head_path

from common.const import CopyDataTypeEnum


class ActionResponse(BaseModel):
    code: int = Field(default=0, description="执行结果")
    body_err: int = Field(None, description="错误码", alias='bodyErr')
    message: str = Field(default='', description="错误信息")
    body_err_params: list = Field(None, description="错误码具体参数", alias="bodyErrParams")


class OceanBaseCode(Enum):
    """
    返回给框架的code
    """
    SUCCESS = 0
    FAILED = 200


class ClusterCheckType:
    """
    集群节点校验类型
    """
    CHECK_OBCLINET = "check_obclient"
    CHECK_OBSERVER = "check_observer"


class OceanBaseSubJobName:
    SUB_CHECK_LOG_STATUS = "sub_check_log_status"
    SUB_EXEC_DATA_BACKUP = "sub_exec_data_backup"
    SUB_EXEC_DATA_COPY = "sub_exec_data_copy"
    SUB_EXEC_LOG_COPY = "sub_exec_log_copy"
    SUB_EXEC_MOUNT_JOB = "sub_exec_mount_job"


class OceanBaseBackupLevel(int, Enum):
    BACKUP_CLUSTER_LEVEL = 1
    BACKUP_TENANT_LEVEL = 2


class SubJobType(int, Enum):
    """
    子任务类型
    """
    # 前置任务
    PRE_SUB_JOB = 0
    # 生成子任务
    GENERATE_SUB_JOB = 1
    # 执行子任务
    BUSINESS_SUB_JOB = 2
    # 后置子任务
    POST_SUB_JOB = 3


class ErrorCode(int, Enum):  # From GoldenDB
    # 增量转全量
    ERROR_INCREMENT_TO_FULL = 1577209901
    # 数据库版本不一致
    ERROR_DIFFERENT_VERSION = 1577209971
    # 集群节点数不一致
    ERROR_DIFFERENT_TOPO = 1577209972
    # OBClient服务异常
    ERROR_OBCLIENT_SERVICES = 1577213526
    # observer不属于同一个集群
    ERROR_OBSERVERS_NOT_MATCH = 1577213525
    # 节点的业务ip地址不属于所选的代理主机
    ERROR_IP_NOT_MATCH_AGENT = 1677947141
    # 集群状态异常
    ERR_CLUSTER_STATUS = 1577213524
    # 数据库用户名不一致
    ERROR_DIFFERENT_USER = 1577209973
    # 挂载路径失败
    ERROR_MOUNT_PATH = 1577209974
    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210000
    # 集群用户不存在
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7
    # 恢复失败
    ERR_RESTORED = 1577210101
    # 不支持修改数据库名称
    ERROR_RENAME_DATABASE_NAME = 1577209901
    # 某个服务未正常开启，返回参数 服务名称
    ERROR_SERVICE_1577213479 = 1577213479
    # 数据库不存在，返回参数 数据库名称
    ERROR_DB_NOT_EXIST = 1577213477
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 参数错误
    ERROR_PARAM = 1677929218
    DB_NODES_NOT_FULL_MATCH = 1677931026
    # 不能修改数据库为新的数据库
    ERR_RENAME_DATABASE = 1577213482
    # 系统异常
    SYSTEM_ERROR = 1677929221
    # 不是所有数据库的服务都运行
    NOT_ALL_DB_SERVICE_RUNNING = 9
    # 增量转全量
    INC_TO_FULL = 0x5E02502D
    # 数据库不存在
    ERR_DB_NOT_EXIST = 1577210047
    # 部署类型错误
    ERR_DEPLOY_TYPE = 0x5E02508B
    # 版本不支持
    ERR_NOT_SUPPORT_VERSION = 1677935120
    # 数据库环境异常不支持备份恢复
    ERR_ENVIRONMENT = 1577213475
    # 账号密码错误
    LOGIN_FAILED = 1677929488
    # 数据库服务异常
    ERR_DB_SERVICES = 1577213522
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    # 主机选择有误
    ERR_CHOSEN_HOSTS = 1577209936
    # 校验socket文件路径失败
    ERR_SOCKET_PATH = 10
    # cnf文件路径错误
    ERR_CONF_PATH = 11
    # keyring文件错误
    ERR_KEYRING_PATH = 12
    # log_bin日志未开启
    LOG_BIN_OFF_ERROR = 1577209945
    # 校验节点类型失败
    ERROR_NODE_TYPE = 1577209938
    # 校验分片结构不一致
    ERROR_GOLDENDB_STRUCTURE = 1577210032
    # 内部错误
    ERROR_INTERNAL = 0x5F025101  # 1593987329
    # 未配置实例级存储配置文件
    ERROR_STORAGE_CONFIG_FAIL = 1577213500
    # 实例存在异常
    ERROR_CLUSTER_ABNORMAL = 1577213501
    # 执行备份/恢复命令失败
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989


class LogLevel(int, Enum):
    INFO = 1
    WARN = 2
    ERROR = 3
    SERIOUS = 4


class GetIPConstant:
    LOCAL_HOST = "127.0.0.1"


class RpcParamKey:
    LOG_COPY = "log"
    APPLICATION = "application"
    TYPES = "types"
    FULL_COPY = "full"
    INCREMENT_COPY = "increment"
    COPY_ID = "copyId"
    JOB_ID = "jobId"
    INPUT_FILE_PREFFIX = "rpcInput"
    OUTPUT_FILE_PREFFIX = "rpcOutput"
    QUERY_PREVIOUS_CPOY = "QueryPreviousCopy"
    RPC_TOOL = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh"
    # 输入参数文件目录
    PARAM_FILE_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # 输出结果文件目录
    RESULT_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"
    DB_BIN_PATH = f'{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/'
    REPORT_COPY_INFO = 'ReportCopyAdditionalInfo'


class FormatCapacity(int, Enum):
    BASE_SIZE = 1024
    KB_SIZE = BASE_SIZE
    MB_SIZE = BASE_SIZE * KB_SIZE
    GB_SIZE = BASE_SIZE * MB_SIZE
    TB_SIZE = BASE_SIZE * GB_SIZE


class MountBindPath:
    DATA_FILE_PATH = "/mnt/databackup/OceanBase-cluster/data"
    META_FILE_PATH = "/mnt/databackup/OceanBase-cluster/meta"
    DB_BIN_PATH = f'{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/'


class SubJobPolicy(int, Enum):
    LOCAL_NODE = 1,
    EVERY_NODE_ONE_TIME = 2,
    RETRY_OTHER_NODE_WHEN_FAILED = 3,
    FIXED_NODE = 4


class CommandReturnCode(Enum):
    ERROR = -1
    # plugin or proxy framework executes successfully
    SUCCESS = 0
    # plugin or proxy framework executes successfully, client need to perform this operation continue
    CONTINUE = 100
    # plugin or proxy framework is busy, client should perform this operation after a period of time
    BUSY = 101
    # an internal error occurred in the plugin or proxy framework
    INTERNAL_ERROR = 200


class NormalErr(int, Enum):
    NO_ERR = 0
    FALSE = -1
    WAITING = -2


class CMDResult(str, Enum):
    """
    执行命令的错误码
    """
    SUCCESS = "0"
    FAILED = "1"
    STORAGE_CONFIG_FAILED = "3"
    CLUSTER_ABNORMAL = "84"


class CmdRetCode(str, Enum):
    EXEC_SUCCESS = "0"
    OP_NOT_PERMITTED = "1"
    NO_SUCH_FILE = "2"
    NO_SUCH_PROCESS = "3"
    EXEC_ERROR = "4"
    CONFIG_ERROR = "5"


class OceanBaseSubType:
    TYPE = "Database"
    SUBTYPE_CLUSTER = "OceanBase-cluster"
    SUBTYPE_TENANT = "OceanBase-tenant"


class OceanBaseNodeType:
    OBSERVER_NODE = "observerNode"
    OBCLENT_NODE = "obclientNode"


class OceanBaseSupportVersion(str, Enum):
    """
    VERSION_3_1_2 = "3.1.2" 没有安装包，暂时不支持
    """
    VERSION_3_2_4 = "3.2.4"
    VERSION_3_2_3 = "3.2.3"
    VERSION_3_2_2 = "3.2.2"
    VERSION_3_2_1 = "3.2.1"


class OceanBaseNodeStatus:
    # 在线
    ONLINE = "1"
    # 离线
    OFFLINE = "0"
    # 异常
    ABNORMAL = "8"


class ExecutePolicy(int, Enum):
    ANY_NODE = 0
    LOCAL_NODE = 1
    EVERY_NODE_ONE_TIME = 2
    RETRY_OTHER_NODE_WHEN_FAILED = 3
    FIXED_NODE = 4


class MasterSlavePolicy:
    MASTER = "master"
    SLAVE = "slave"


class ProgressInfo(str, Enum):
    START = "Start to do restore"
    FAILED = "Failed to do task, unique id: TU34IHS"
    SUCCEED = "Succeed to do task, unique id: TU34IHS"


class Report:
    REPORT_INTERVAL = 30


class OceanBaseResourceKeyName:
    APPLICATION_AUTH_AUTHKEY = "application_auth_authKey_"
    APPLICATION_AUTH_AUTHPWD = "application_auth_authPwd_"
    LIST_APPLICATION_AUTH_AUTHKEY = "applications_0_auth_authKey_"
    LIST_APPLICATION_AUTH_AUTHPWD = "applications_0_auth_authPwd_"
    APPENV_AUTH_AUTHKEY = "appEnv_auth_authKey_"
    APPENV_AUTH_AUTHPWD = "appEnv_auth_authPwd_"
    JOB_TARGETENV_AUTH_AUTHKEY = "job_targetEnv_auth_authKey_"
    JOB_TARGETENV_AUTH_AUTHPWD = "job_targetEnv_auth_authPwd_"


class OceanBaseQueryStatus(str, Enum):
    FAILED = "FAILED"
    SUCCESS = "SUCCESS"
    RUNNING = "RUNNING"
    DOING = "DOING"
    STOP = "STOP"
    BEGINNING = "BEGINNING"


class OceanBaseSqlCmd(Enum):
    QUERY_LOG_MAX_NEXT_TIME = "QUERY_LOG_MAX_NEXT_TIME"
    QUERY_INCARNATION_ID = "QUERY_INCARNATION_ID"
    QUERY_BACKUP_DESTINATION = "QUERY_BACKUP_DESTINATION"
    CLUSTER_FULL_BACKUP = "CLUSTER_FULL_BACKUP"
    QUERY_LOG_STATUS = "QUERY_LOG_STATUS"
    FIND_MAX_BS_KEY = "FIND_MAX_BY_KEY"
    MAX_BS_KEY_STATUS = "MAX_BS_KEY_STATUS"
    CLUSTER_INCRE_BACKUP = "CLUSTER_INCRE_BACKUP"
    TENANT_FULL_BACKUP = "TENANT_FULL_BACKUP"
    QUERY_TENANT_ID_LIST = "QUERY_TENANT_ID_LIST"
    QUERY_BACKUP_TIME = "QUERY_BACKUP_TIME"
    QUERY_TENANT_ID_LIST_BY_NAME = 'QUERY_TENANT_ID_LIST_BY_NAME'
    QUERY_TENANT_NAME_LIST_BY_ID = 'QUERY_TENANT_NAME_LIST_BY_ID'
    UPDATE_LOG_ARCHIVE_INTERVAL = 'UPDATE_LOG_ARCHIVE_INTERVAL'
    QUERY_TABLE_FOR_DISPLAY = 'QUERY_TABLE_FOR_DISPLAY'
    QUERY_DATABASE_FOR_DISPLAY = 'QUERY_DATABASE_FOR_DISPLAY'
    QUERY_OLD_BACKUP_SET = 'QUERY_OLD_BACKUP_SET'


class OceanBaseQueryType(str, Enum):
    POOL = "pool"


copyMetaFileName = (
    'clog_info',
    'cluster_backup_piece_info_@versionset@',
    'cluster_backup_set_file_info_@versionset@',
    'cluster_clog_backup_info_@versionset@',
    'cluster_data_backup_info_@versionset@',
    'tenant_info_@versionset@',
    'tenant_name_info_@versionset@'
)

SYSTEM_TENANT_ID = '1'
INCARNATION_1 = 'incarnation_1'
CLUSTER = 'cluster'


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
        2: [RpcParamKey.FULL_COPY],
        3: [RpcParamKey.LOG_COPY],
        4: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY, RpcParamKey.LOG_COPY]
    }


@dataclass
class JobInfo:
    """
    任务相关参数信息
    """
    copy_id: str
    job_id: str
    # 恢复或者备份
    type: str = None
    pid: str = None
    usr: str = None
    env_path: str = None
    sub_job_id: str = None
    sub_job_type: str = None
    cache_path: str = None
    meta_path: str = None
    meta_rep: dict = None
    cache_rep: dict = None
    data_reps: list = None
    metadata_destination: str = None
    backup_type: int = None
    progress: int = None
    task_status: int = None
    backup_result: int = None
    storage_id: str = None
    protect_env: dict = None
    backup_tool_type: int = None
    res_name: str = None
    nodes: list = None
    pro_obj_extend_info: dict = None
    pro_obj: dict = None
    storage_esn: set = None
    agents: list = None
    failed_agents: list = None
    cluster_agents: list = None
    host_agents: list = None
    # 此次备份是否开启源端重删
    open_source_delete: bool = False


class SubJobStatusForSqlite(int, Enum):
    NOT_STARTED = 0
    SUCCESS = 1
    RETRY = 2
    FAILED = 3
    DOING = 4


class ObclientStatus(str, Enum):
    OFFLINE = "0"
    ONLINE = "1"


class SqliteServiceName(str, Enum):
    SQLITE_DIR = "sqlite"
    SQLITE_DATABASE_NAME = 'copymetadata.sqlite'
    SQLITE_TABLE_NAME = "T_COPY_METADATA"
    TENANT_LEVEL = "tenant"
    DATABASE_LEVEL = "database"
    TABLE_LEVEL = "table"


class ArchiveType:
    archive_array = [CopyDataTypeEnum.TAP_ARCHIVE.value, CopyDataTypeEnum.S3_ARCHIVE.value]


class OceanBaseReportLabel:
    # 数据保护代理主机（{0}）开始执行子任务（{1}）。
    EXECUTE_SUB_TASK_SUCCESS_LABEL = "agent_start_execute_sub_task_success_label"
    # 开始执行恢复任务，数据保护代理主机（{0}）待恢复表数量：{1}。
    RESTORE_TABLE_TOTAL_LABEL = "job_log_restore_table_table_total_label"
    # 数据保护代理主机（{0}），第{1}张表：（{2}：{3}）恢复完成。
    RESTORE_TABLE_SUCCESS_LABEL = "job_log_restore_table_success_label"
    # 数据保护代理主机（{0}），第{1}张表：（{2}：{3}）恢复失败。
    RESTORE_TABLE_FAIL_LABEL = "job_log_restore_table_fail_label"
    # 目标集群（{0}）不存在，恢复失败。
    CHECK_CLUSTER_EXIST_FAIL_LABEL = "job_log_restore_check_cluster_exist_fail_label"
    # 目标集群（{0}）为备模式，恢复失败。
    CHECK_CLUSTER_ROLE_FAIL_LABEL = "job_log_restore_check_cluster_role_fail_label"
    # 目标集群（{0}）状态为不可用，恢复失败。
    CHECK_CLUSTER_STATUS_FAIL_LABEL = "job_log_restore_check_cluster_status_fail_label"
    # 目标集群（{0}）恢复配置未打开，恢复失败。
    CHECK_RESTORE_CONCURRENCY_FAIL_LABEL = "job_log_restore_check_restore_concurrency_fail_label"
    # 开始执行恢复任务，数据保护代理主机（{0}）待恢复租户数量：{1}。
    RESTORE_TENANT_TOTAL_LABEL = "job_log_restore_tenant_total_label"
    # 数据保护代理主机（{0}），第{1}个租户：（{2}）恢复完成。
    RESTORE_TENANT_SUCCESS_LABEL = "job_log_restore_tenant_success_label"
    # 数据保护代理主机（{0}），第{1}个租户：（{2}）恢复失败。原因：{3}。
    RESTORE_TENANT_FAIL_LABEL = "job_log_restore_tenant_fail_label"
    # 数据保护代理主机（{0}），第{1}个租户：（{2}）恢复失败。原因：资源池（{3}）不可用。
    CHECK_RESOURCE_POOL_FAIL_LABEL = "job_log_restore_check_resource_pool_fail_label"
    # 数据保护代理主机（{0}），第{1}个租户：（{2}）恢复失败。原因：租户（{3}）已存在。
    CHECK_TENANT_EXIST_FAIL_LABEL = "job_log_restore_check_tenant_exist_fail_label"
    # 数据保护代理主机（{0}），第{1}个租户：（{2}）恢复失败。原因：时间戳（{3}）无效。
    CHECK_TIMESTAMP_FAIL_LABEL = "job_log_restore_check_timestamp_fail_label"
    # 请添加租户后重试。
    BACKUP_EMPTY_CLUSTER_FAIL_LABEL = "protection_oceanbase_empty_cluster_tips_label"
    # 数据保护代理主机（{0}）挂载目录失败。
    BACKUP_MOUNT_FAIL_LABEL = "agent_execute_mount_nas_fail_label"
    # 租户集备份前需要执行集群的全量备份
    FULL_BACK_SHOULD_BEFORE_TENANT_SET_BACKUP_LABEL = "full_backup_should_before_tenant_set_backup_label"
    # 数据保护代理主机（{0}）创建挂载目录失败。
    BACKUP_MKDIR_MOUNT_POINT_FAIL_LABEL = "oceanbase_execute_mkdir_mount_point_fail_label"
    # OBServer开启日志归档失败。
    BACKUP_OPEN_LOG_ARCHIVE_FAIL_LABEL = "oceanbase_open_log_archive_fail_label"
    # 执行日志归档任务失败：{0}。
    BACKUP_EXEC_LOG_SUB_JOB_FAIL_LABEL = "oceanbase_execute_log_archive_sub_job_fail_label"
    # 执行数据库命令失败：{0}。
    BACKUP_EXEC_COMMAND_FAIL_LABEL = "oceanbase_execute_sql_command_fail_label"
    # 开启日志归档失败：{0}。。
    BACKUP_EXEC_OPEN_LOG_ARCHIVE_FAIL_LABEL = "oceanbase_execute_open_log_archive_sql_command_fail_label"
    # 关闭日志归档失败：{0}。。
    BACKUP_EXEC_STOP_LOG_ARCHIVE_FAIL_LABEL = "oceanbase_execute_stop_log_archive_sql_command_fail_label"
    # 设置备份目的地失败：{0}。
    BACKUP_SET_BACKUP_DEST_FAIL_LABEL = "oceanbase_set_backup_dest_fail_label"
    # 未开启日志归档：{0}。
    BACKUP_EXEC_LOG_ARCHIVE_NOT_DOING_FAIL_LABEL = "oceanbase_log_archive_not_doing_label"
    # 执行备份任务失败：{0}。
    BACKUP_EXEC_BACKUP_SUB_JOB_FAIL_LABEL = "oceanbase_execute_backup_sub_job_fail_label"
    # 所有OBClient无法连接OBServer，任务失败。
    BACKUP_ALL_OBCLIENT_OFFLINE_LABEL = "oceanbase_all_obclient_disconnected_fail_label"
    # 当前日志归档未开启，执行开启命令。
    BACKUP_EXEC_OPEN_LOG_ARCHIVE_WARN_LABEL = "oceanbase_execute_open_log_archive_warn_label"
    # 当前日志归档需要关闭，执行关闭命令。
    BACKUP_EXEC_DISABLE_LOG_ARCHIVE_WARN_LABEL = "oceanbase_execute_disable_log_archive_warn_label"
    # 重新设置备份目的地。
    BACKUP_EXEC_RESET_BACKUP_DEST_WARN_LABEL = "oceanbase_execute_set_backup_dest_warn_label"
    # OBClient（{0}）无法连接OBServer，当前任务已转移至其他OBClient节点执行。
    BACKUP_TRANSFER_TO_OTHER_OBCLIENT_WARN_LABEL = "oceanbase_transfer_task_to_other_obclient_warn_label"


class RestoreConstant:
    RESTORE_SUCCESS_STATUS = 'RESTORE_SUCCESS'
    PRIMARY_CLUSTER_ROLE = 'PRIMARY'
    VALID_CLUSTER_STATUS = 'VALID'
    CLUSTER_SUB_TYPE = 'OceanBase-cluster'
