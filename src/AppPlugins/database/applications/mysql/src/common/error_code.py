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


class BodyErr(Enum):
    """
    返回给框架的错误码
    """
    ERR_PLUGIN_CANNOT_BACKUP = 0x64032B0C  # 插件无法执行该备份任务
    ERR_INC_TO_FULL = 0x5E02502D
    ERROR_COMMON_INVALID_PARAMETER = 0x5F025102


class MySQLErrorCode:
    # 系统异常
    SYSTEM_ERROR = 1677929221

    # 连接mysql服务失败
    CONNECT_MYSQL_SERVER_FAILED = 1577209942

    # 获取mysql版本信息失败
    GET_MYSQL_VERSION_FAILED = 1577209944

    # log_bin日志未开启
    LOG_BIN_ID_OFF_ERROR = 1577209945

    # 有relay-log，需要打开relay_log_recovery=on 否则恢复至本机会失败
    RELAY_LOG_DIRECTORY_OFF_ERROR = 1677931356

    # 获取数据库列表失败
    GET_DATABASES_FAILED = 1577209946

    # 校验集群失败
    CHECK_CLUSTER_FAILED = 1577209956

    # 检验集群类型失败
    CHECK_CLUSTER_TYPE_FAILED = 1577209995

    # 检验实例数量失败
    CHECK_CLUSTER_NUM_FAILED = 1577209991

    # 检查mysql用户密码错误
    CHECK_AUTHENTICATION_INFO_FAILED = 1577209994

    # 检查mysql未关闭
    CHECK_MYSQL_NOT_CLOSE = 1577210048

    # 所选主机为集群节点
    CHECK_MYSQL_REGISTER_TYPE_ERROR = 1577210068

    # 检查备份工具失败
    CHECK_BACKUP_TOOL_FAILED = 1577209967

    # mysql未处于运行状态
    CHECK_MYSQL_NOT_RUNNING = 1577209965

    # 检查mysqlbinlog失败
    CHECK_MYSQL_BIN_LOG_FAILED = 1577209964

    # 重命名存在同名数据库，恢复失败
    CHECK_DATABASE_WITH_THE_SAME_NAME_EXIST = 1577209992

    # 检查mysql配置文件失败
    CHECK_MYSQL_CONF_FAILED = 1577213442

    # 恢复时导入表空间失败
    RESTORE_IMPORT_TABLE_SPACE_FAILED = 1577213441

    # 新位置存在和副本同名数据库，恢复失败
    CHECK_NEW_POS_DATABASE_WITH_THE_SAME_NAME_EXIST = 1577213443

    # 检查MySQL或MariaDB用户权限不足
    CHECK_PRIVILEGE_FAILED = 1677873187

    # 异机恢复，可能导致用户丢失，进而访问数据库失败
    RESTORE_CHECK_USER_FAILED = 1577213464

    # 副本undo数和my.cnf配置innodb_undo_tablespaces不一致
    DIFF_INNODB_UNDO_TABLESPACES = 1577213542

    # 检查到不能做备份
    ERR_DATABASE_STATUS = 1577210002

    # 执行备份、或恢复、即时挂载失败 	该错误码打印异常
    EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL = 1577209989

    # 调用备份工具失败
    ERR_BACKUP_TOOL_FAILED = 1577213505

    # PXC集群MySQL严格模式开启,影响数据库表空间导入
    ERR_PXC_STRICT_MODE = 1577209943

    # 访问ip错误
    ERR_IP = 1577213530

    # 访问端口错误
    ERR_PORT = 1577213531

    # binlog文件丢失
    ERR_BIN_LOG_NOT_EXIST = 1577213532

    # 集群拓扑结构不同
    ERROR_DIFFERENT_TOPO = 1577213533

    # 错误的用户名或密码
    ERROR_LOGIN_INFO = 1677929488

    # 实例未处于运行状态
    ERROR_INSTANCE_IS_NOT_RUNNING = 1577209975

    # 数据库集群同步状态有误
    ERROR_CLUSTER_SYNC_STATUS = 1577213534

    # 检查mysqld工具失败
    CHECK_MYSQLD_TOOL_FAILED = 1677931355

    # 检查mysqladmin工具失败
    CHECK_MYSQL_ADMIN_TOOL_FAILED = 1677931358

    # 即时挂载或者销毁即时挂载失败
    CANCEL_LIVE_MOUNT_FAILED = 1677931373

    # 输入的my.cnf不存在
    INPUT_MY_CNF_NOT_EXIST = 1677876739

    # xtrabackup 不支持 INSTANT ADD/DROP COLUMN
    UNSUPPORTED_INSTANT_COLUMN = 1677876740


class MySQLCode(Enum):
    """
    返回给框架的code
    """
    SUCCESS = 0
    FAILED = 200
