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

import os
from enum import Enum

import pexpect


class RestoreAction:
    # 查询恢复前置进度action name
    QUERY_RESTORE_PRE = "QueryRestorePreProgress"
    # 查询恢复进度action name
    QUERY_RESTORE = "QueryRestoreProgress"
    # 查询恢复后置进度action name
    QUERY_RESTORE_POST = "QueryRestorePostProgress"


class PgConst:
    # 主版本号9
    MAJOR_VERSION_NINE = 9
    # 主版本号10
    DATABASE_V10 = 10
    # 主版本号11
    DATABASE_V11 = 11
    # 主版本号12
    DATABASE_V12 = 12
    # 主版本号13
    MAJOR_VERSION_THIRTEEN = 13
    # 主版本号15
    DATABASE_V15 = 15
    # 子版本号4
    MINOR_VERSION_FOUR = 4
    # 版本号第二位
    DATABASE_MINOR_VERSION_SIX = 6
    # 操作系统root用户名
    OS_USER_ROOT = "root"
    # PostgreSQL数据库默认端口号
    DB_DEFAULT_PORT = 5432
    # Pgpool默认端口号
    PGPOOL_DEFAULT_PORT = 9999

    # recovery_target_time时间格式
    RECOVERY_TARGET_TIME_FORMATTER = "%Y-%m-%d %H:%M:%S"
    # 日志文件时间格式
    LOG_FILE_TIME_FORMATTER = "%Y%m%d%H%M%S"
    # 提取WAL中时间的正则表达式
    MATCH_WAL_TIME_REGEX = r"(\d{4}-\d{1,2}-\d{1,2}\s\d{1,2}:\d{1,2}:\d{1,2})"

    # “recovery_target”参数值“immediate”
    RECOVERY_TARGET_IMMEDIATE = "immediate"
    # “recovery_target_timeline”参数值“latest”
    RECOVERY_TGT_TIMELINE_LATEST = "latest"
    # “recovery_target_timeline”参数值“current”
    RECOVERY_TGT_TIMELINE_CURRENT = "current"

    # 计算恢复速度间隔时间
    CALC_RESTORE_SPEED_INTERVAL = 5
    QUERY_PROGRESS_TIME = 5
    PROGRESS_FIVE = 5
    PROGRESS_NINETY = 90
    PROGRESS_ONE_HUNDRED = 100
    PROGRESS_FIFTY = 50

    # 安装路径属主
    OWNER_FILE_NAME = "switch.conf"

    # v9及以下archive_status相对路径列表
    ARCHIVE_STATUS_DIR_V9_AND_BELOW_PATHS = ("pg_xlog", "archive_status")
    # v10及以上archive_status相对路径列表
    ARCHIVE_STATUS_DIR_V10_AND_ABOVE_PATHS = ("pg_wal", "archive_status")
    POSTGRESQL_CONF_FILE_NAME = "postgresql.conf"
    POSTGRESQL_BASE_CONF_FILE = "postgresql.base.conf"
    POSTGRESQL_AUTO_CONF_FILE_NAME = "postgresql.auto.conf"
    PG_HBA_CONF_FILE_NAME = "pg_hba.conf"
    RECOVERY_CONF_FILE_NAME = "recovery.conf"
    RECOVERY_CONF_SAMPLE_NAME = "recovery.conf.sample"
    RECOVERY_SIGNAL_FILE_NAME = "recovery.signal"
    STANDBY_SIGNAL_FILE_NAME = "standby.signal"
    # 恢复前需要备份的配置文件列表
    NEED_BAK_CFG_FILES = (PG_HBA_CONF_FILE_NAME, POSTGRESQL_AUTO_CONF_FILE_NAME, POSTGRESQL_CONF_FILE_NAME)
    # recovery.conf样例文件相对路径
    RECOVERY_CONF_SAMPLE_PATHS = ("share", RECOVERY_CONF_SAMPLE_NAME)
    # patroni日志恢复前数据拷贝目录
    OCEAN_PROTECT_DATA_COPY = "OceanProtectData"
    # 数据库data目录恢复前需删除文件名称
    DELETE_FILE_NAMES_OF_DATA_DIR = (
        "postmaster.pid", "postmaster.opts", "recovery.conf", "recovery.done",
        "recovery.signal", "backup_label.old"
    )
    DELETE_FILE_NAMES_OF_BAK_CONF = ("pg_hba.conf.bak", "postgresql.auto.conf.bak", "postgresql.conf.bak")
    # 当前PostgreSQL插件下的conf目录路径
    CURR_RECOVERY_CONF_SAMPLE_PATH = os.path.realpath(os.path.join(os.path.split(__file__)[0], "..", "conf"))
    # 恢复数据后数据库data目录需删除文件名称
    DELETE_FILE_NAMES_OF_DATA_DIR_AFTER_RESTORE = ("backup_label.old",)
    RESTORE_NOT_COPIED_DIRS = (".snapshot",)
    RESTORE_PROGRESS_ACTIONS = (
        RestoreAction.QUERY_RESTORE_PRE,
        RestoreAction.QUERY_RESTORE,
        RestoreAction.QUERY_RESTORE_POST
    )
    RESTORE_SPEED_FILE = "RestoreSpeed"
    # 路径黑名单
    PATH_BLACK_LIST = r"^/$|^/bin$|^/bin/.*|^/boot$|^/boot/.*|^/dev$|^/dev/.*|^/etc$|^/etc/.*|" \
                      r"^/lib$|^/lib/.*|^/lib64$|^/lib64/.*|^/lost+found$|^/lost+found/.*|^/media$|^/media/.*|" \
                      r"^/proc$|^/proc/.*|^/root$|^/run$|" \
                      r"^/sbin$|^/sbin/.*|^/selinux$|^/selinux/.*|^/srv$|^/srv/.*|^/sys$|^/sys/.*|" \
                      r"^/usr$|^/usr/bin$|^/usr/include$|^/usr/lib$|^/usr/local$|" \
                      r"^/usr/local/bin$|^/usr/local/include$|^/usr/local/sbin$|^/usr/local/share$|" \
                      r"^/usr/sbin$|^/usr/share$|^/usr/src$|^/var$"
    # 删除路径白名单
    DELETING_PATH_WHITELIST = r"^/mnt/databackup/"
    # 数据库自动WAL检查点之间的最长时间
    CHECK_POINT_TIME_OUT = 86400
    # 备份停止命令的最长执行时间
    STOP_PG_BACKUP_TIME_OUT = 300
    # 单次日志备份最大文件数量
    MAX_FILE_NUMBER_OF_LOG_BACKUP = 1000

    # 非恢复后首次全量备份
    NOT_THE_FIRST_BACKUP_AFTER_RESTORE = '0'

    # 不支持的patroni版本
    NOT_SUPPORT_PATRONI_VERSION = '3.2.0'

    # CLup Server集群校验键key
    ACTION_TYPE = 'actionType'

    # CLup Server集群校验值value
    QUERY_CLUP_SERVER = 'queryClupServer'

    # CLup Server集群配置文件路径
    CLUP_SERVER_PATH = '/opt/clup/conf/clup.conf'



class CmdRetCode(str, Enum):
    EXEC_SUCCESS = "0"
    OP_NOT_PERMITTED = "1"
    NO_SUCH_FILE = "2"
    NO_SUCH_PROCESS = "3"
    EXEC_ERROR = "4"
    CONFIG_ERROR = "5"


class ErrorCode(Enum):
    # 通用代理执行任务时，由于插件鉴权失败，操作失败
    PLUGIN_CANNOT_BACKUP_ERR = 0x64032B0C
    # Agent增量备份无法进行，需要转成全量备份
    INC_TO_FULL_ERR = 0x5E02502D
    # 收到的请求参数不正确
    INVALID_PARAMETER_ERR = 0x5F025102
    # 数据库离线
    DATABASE_OFFLINE_ERR = 0x5E0250C2
    # 数据库未开启归档模式
    ARCHIVE_MODE_ENABLED = 0x5E0250C3


class PexpectResult:
    OS_LOGIN_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "登录", "Last login", ""]
    DB_LOGIN_PASSWORD = [pexpect.TIMEOUT, pexpect.EOF, "Password for", "Password:", "口令:", "口令："]
    HTML_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "<p>"]
    LOGIN_DATABASE_SUCCESS = [pexpect.TIMEOUT, pexpect.EOF, "=#"]
    EXECUTE_CMD_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "$"]


class RestoreSubJob:
    PREPARE = "prepare"
    RESTORE = "restore"
    POST = "post"


class ConfigKeyStatus:
    """配置文件中key的状态
    """
    # 被注释
    ANNOTATED = "annotated"
    # 已配置
    CONFIGURED = "configured"
    # 不存在
    NOT_EXIST = "notExist"


class BackupStatus:
    COMPLETED = 1
    RUNNING = 2
    FAILED = 3


class DirAndFileNameConst:
    TABLE_SPACE_INFO_FILE = "PGSQL_TABLE_SPACE.info"
    TABLE_SPACE_INFO_DIR = "pgsql_table_space"
    COPY_FILE_INFO = "CopyInfoFile.info"
    LAST_LOG_BACKUP_STOP_WAL_INFO = "LastLogBackupStopWalInfo.json"


class InstallDeployType:
    PATRONI = "Patroni"
    PGPOOL = "Pgpool"
    CLUP = "CLup"
