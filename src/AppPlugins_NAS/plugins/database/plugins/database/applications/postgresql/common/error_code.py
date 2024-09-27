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

class ErrorCode:
    # 用户不存在
    USER_IS_NOT_EXIST = 1577209963

    # 连接pgsql失败
    CHECK_CONNECTIVITY_FAILED = 1577209942

    # 用户名或密码错误
    LOGIN_FAILED = 1677929488

    # 校验集群失败
    CHECK_CLUSTER_FAILED = 1577209956

    # 数据库安装路径不正确
    DATABASE_INSTALLATION_PATH_IS_INVALID = 1577213577

    # Pgpool安装路径不正确
    PGPOOL_INSTALLATION_PATH_IS_INVALID = 1577213578

    # Patroni配置文件完整路径不正确
    FULL_PATH_OF_PATRONI_CONFIGURATION_FILE_IS_INVALID = 1577213579

    # 业务IP地址不正确
    SERVICE_IP_IS_INVALID = 1577213580

    # 数据库端口不正确
    DATABASE_PORT_IS_INVALID = 1577213581

    # Pgpool端口不正确
    PGPOOL_PORT_IS_INVALID = 1577213582

    # 获取集群节点失败
    GET_CLUSTER_NODE_FAILED = 1577209966

    # 获取版本信息失败
    GET_VERSION_FAILED = 1577209944

    # 客户端路径不存在
    CLIENT_PATH_IS_NOT_EXIST = 1577209968

    # 执行恢复前数据库实例正在运行
    DB_RUNNING_ERR_BEFORE_RESTORE = 1577210053

    # 执行恢复前数据库端口被占用
    DB_PORT_LISTEN_ERR_BEFORE_RESTORE = 1577210054

    # 目标实例数据目录空间不足
    NO_ENOUGH_SPACE_BEFORE_RESTORE = 1577210055

    # 副本版本和目标实例版本不匹配
    VERSION_NOT_MATCH_BEFORE_RESTORE = 1577210056

    # 副本的OS用户和目标实例的OS用户不一致
    OS_USER_NOT_EQUAL_BEFORE_RESTORE = 1577210057

    # 目标实例数据目录不可读写
    TARGET_NOT_RW_BEFORE_RESTORE = 1577210058

    # 执行恢复前Pgpool端口被占用
    PGPOOL_PORT_LISTEN_ERR_BEFORE_RESTORE = 1577210062

    # 恢复数据失败
    RESTORE_DATA_FAILED = 1577210059

    # 执行启动数据库命令失败
    EXEC_START_DB_CMD_FAILED = 1577210060

    # 恢复后数据库状态异常
    DB_STATUS_ERR_AFTER_RESTORE = 1577210061

    # 执行启动Pgpool命令失败
    EXEC_START_PGPOOL_CMD_FAILED = 1577210063

    # 恢复后Pgpool状态异常
    PGPOOL_PORT_NOT_LISTEN_ERR_AFTER_RESTORE = 1577210064

    # 执行启动Patroni命令失败
    EXEC_START_PATRONI_CMD_FAILED = 1577210063

    # 日志恢复时patroni.yml配置不正确
    PATRONI_YML_NOT_SET_BEFORE_PITR = 1577144352

    # 当前版本的patroni，不支持日志恢复
    PATRONI_VERSION_NOT_SUPPORT = 1677933075

    # 恢复表空间失败
    RESTORE_TABLE_SPACE_FAILED = 1577213466

    # 恢复时，表空间目录与原位置表空间目录权限不匹配
    TB_DIR_PERMISSION_IS_NOT_MATCH = 1577213467

    # 备份表空间失败
    BACKUP_TABLE_SPACE_FAILED = 1577213468

    # 清空表空间目录失败
    CLEAR_TABLE_SPACE_DIR_FAILED = 1577213469

    # 归档模式配置错误
    ARCHIVE_MODE_CONFIG_ERROR = 1577213470

    # 备份失败
    BACKUP_FAILED = 1577213471

    # 备份工具返回失败
    BACKUP_TOOL_FAILED = 1577213505

    # 日志副本不连续，请执行全量备份后重试
    LOG_INCONSISTENT = 1577213480
