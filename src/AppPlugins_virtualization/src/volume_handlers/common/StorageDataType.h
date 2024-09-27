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
#ifndef STORAGE_DATA_TYPE_H
#define STORAGE_DATA_TYPE_H
#include "common/Macros.h"

namespace VirtPlugin {
/* =====Get WWN and ID by disk path ===== */
#define PAGE_83 0x83
#define PAGE_80 0x80
#define PAGE_CDB_0 0x00
#define PAGE_CDB_1 0x01

// scsi
#define SCSI_MAX_SENSE_LEN 64  // sense的最大长度
#define SCSI_DISK_DATA_LEN 512  // 磁盘数据长度
#define DISK_MAX_PAGE_NUM 255  // 磁盘支持的最大VPD Page数
#define DISK_NORECORD_ERROR 4  // 磁盘 无记录错误码

// CDB长度
#define CDB6GENERIC_LENGTH 6  // 6 字节
#define CDB10GENERIC_LENGTH 10  // 10 字节
#define CDB16GENERIC_LENGTH 16  // 16 字节

#define SCSIOP_READ_CAPACITY 0x25  // READ_CAPACITY命令操作码
#define SCSIOP_INQUIRY 0x12  // CBD查询码

#define NAME_PATH_LEN 512  // 磁盘名称长度
#define DISK_PATH_MAX 260  // 磁盘路径最大值
#define MAX_VENDOR_LEN 64  // 厂商长度
#define MAX_PRODUCT_LEN 256  // 型号长度
#define MAX_VVERSION_LEN 4  // 阵列V版本信息
#define MAX_SN_LEN 30  // 序列号长度
#define MAX_WWN_LEN 64  // WWN
#define MAX_LUNID_LEN 11  // LUN ID
#define MAX_SN_HW 21  // 序列号长度

// linux
#define DISK_BYTE_OF_SECTOR 512  // io命令返回的缓存大小
#define DATA_LEN_256 256
#define SCSI_CMD_TIMEOUT_LINUX (60 * 1000)
#define EXE_CMD_SCR_LEN 256  // LINUX/AIX命令长度
#define LINE_DATA_LEN 256  // 临时文件一行的长度
#define FILE_ROW_COUNT 255
#define MAX_NAME_LEN 256
#define LINUX_BLOCK 1024  // LINUX文件块大小
/* =====Get WWN and ID by disk path ===== */

// type 对象类型枚举
typedef enum {
    MO_UNKNOWN = 0,
    MO_DISK = 10,
    MO_LUN = 11,
    MO_HOST_GROUP = 14,
    MO_HOST = 21,
    MO_SNAPSHOT = 27,
    MO_FILESYSTEM = 40,
    MO_SYSTEM = 201,
    MO_ENCLOSURE = 206,
    MO_CONTROLLER = 207,
    MO_INTERFACE = 209,  // 定义接口模块，启动器的PARAENTTYPE是209
    MO_FCPORT = 212,
    MO_ETHPORT = 213,
    MO_STORAGEPOOL = 216,
    MO_STORAGETIER = 217,
    MO_LUNCOPY = 219,
    MO_LUNCOPYMEMBER = 227,
    MO_CONSISTENTGROUP = 221,
    MO_ISCSIINITIATOR = 222,
    MO_FCINITIATOR = 223,
    MO_FCLINK = 225,
    MO_ISCSISESSION = 229,
    MO_ISCSILINK = 243,
    MO_MAPPINGVIEW = 245,
    MO_ISCSISESSIONSETTING = 247,
    MO_ISCSITGTNODE = 248,
    MO_ISCSITGTPORT = 249,
    MO_REMOTEDEVICE = 224,
    MO_REMOTELUN = 250,
    MO_LUNGROUP = 256,
    MO_PORTGROUP = 257,
    MO_REPLICATIONPAIR = 263,
    MO_DISKPOOL = 266,
    MO_STORAGEENGINE = 267,
    MO_SNAS_CIFS_SERVICE = 16399,
    MO_SNAS_FILE = 16400,
    MO_SNAS_NFS_SHARE = 16401,
    MO_SNAS_CIFS_SHARE = 16402,
    MO_SNAS_CIFS_SHARE_AUTH_CLIENT = 16404,
    MO_SNAS_NFS_SHARE_AUTH_CLIENT = 16409,
    MO_QUOTATREE = 16445,
    MO_FILESYSTEM_CONFIG_ABILITY = 16486,
    MO_FILE_SHARE_CONFIG = 16487,
    MO_FILE_SHARE_TYPE_CONFIG = 16488,
    MO_FILESYSTEM_TYPE_CONFIG = 16489,
    MO_QUOTA_CONFIG_ABILITY = 16493,
    MO_FILESYSTEM_QUOTA = 16496,
    MO_IBINITIATOR = 16499,
    MO_IBPORT = 16500
} MO_TYPE;

// HEALTH_STATUS_E 健康状态
typedef enum {
    HSE_UNKNOWN = 0,        // 未知
    HSE_NORMAL,             // 正常
    HSE_FAULT,              // 故障
    HSE_PRE_FAIL,           // 即将故障
    HSE_PART_BROKEN,        // 部分损坏
    HSE_DEGRADE,            // 降级
    HSE_HAS_BAD_BLOCK,      // 有坏块
    HSE_HAS_ERR_CODE,       // 有误码
    HSE_CONSISTENT,         // 一致
    HSE_INCONSISTENT,       // 不一致
    HSE_BUSY,               // 繁忙
    HSE_POWER_NO_INPUT,     // 无输入
    HSE_POWER_NOT_ENOUGH,   // 电量不足
    HSE_SINGLE_LINK_FAULT,  // 单链路故障
    HSE_INVALID,            // 失效
    HSE_WRITE_PROTECT       // 写保护
} HEALTH_STATUS_E;

// RUNNING_STATUS_E 运行状态
typedef enum {
    RUN_UNKNOWN = 0,               // 未知
    RUN_NORMAL,                    // 正常
    RUN_RUNNING,                   // 运行
    RUN_NOT_RUNNING,               // 未运行
    RUN_NOT_EXIST,                 // 不存在
    RUN_HIGH_TEMPERATURE_SLEEP,    // 高温休眠
    RUN_STARTING,                  // 正在启动
    RUN_POWER_FAILURE_PROTECTING,  // 掉电保护
    RUN_SLEEPING,                  // 休眠
    RUN_SPINGUP,                   // 已启动
    RUN_LINK_UP,                   // 已连接 10
    RUN_LINK_DOWN,                 // 未连接
    RUN_POWER_ON,                  // 正在上电
    RUN_POWER_OFF,                 // 已下电
    RUN_PRE_COPY,                  // 预拷贝
    RUN_COPYBACK,                  // 回拷
    RUN_RECONSTRUCTION,            // 重构
    RUN_EXPANSION,                 // 扩容
    RUN_NOT_FORMAT,                // 未格式化
    RUN_FORMATTING,                // 正在格式化
    RUN_UNMAPPING,                 // 未映射 20
    RUN_INITIAL_SYNCHRONIZING,     // 正在初始同步
    RUN_CONSISTENT,                // 数据一致
    RUN_SYNCHRONIZING,             // 正在同步
    RUN_SYNCHRONIZED,              // 已同步
    RUN_NOT_SYNCHRONIZED,          // 未同步
    RUN_SPLIT,                     // 已分裂
    RUN_ONLINE,                    // 在线
    RUN_OFFLINE,                   // 离线
    RUN_LOCKED,                    // 已锁定
    RUN_ENABLE,                    // 已启用 30
    RUN_DISABLED,                  // 已禁用
    RUN_LEVELING,                  // 正在均衡
    RUN_TO_BE_RECOVERD,            // 待恢复
    RUN_INTERRUPTED,  // 异常断开				如果对于一致性组来讲，异常断开将导致成员复制异常断开
    RUN_INVALID,      // 失效
    RUN_NOSTART,      // 新创建
    RUN_QUEUING,      // 正在排队
    RUN_STOP,         // 已停止
    RUN_COPYING,      // 正在拷贝
    RUN_COMPLETED,    // 拷贝完成/完成
    RUN_PAUSE,        // 暂停
    RUN_REVSYNCHRONIZING,           // 正在反向同步
    RUN_ACTIVATED,                  // 已激活
    RUN_ROLLBACK,                   // 正在回滚
    RUN_INACTIVATED,                // 未激活
    RUN_IDLE,                       // 等待
    RUN_POWERING_OFF,               // 正在下电
    RUN_CHARGING,                   // 正在充电				电源状态
    RUN_CHARGED,                    // 充电完成				电源状态
    RUN_DISCHARGING,                // 正在放电				电源状态
    RUN_UPGRADING,                  // 正在升级				系统状态
    RUN_POWER_LOST,                 // 掉电中
    RUN_INITIALIZING,               // 初始化中
    RUN_APPLY_CONFIG_CHANGE,        // 正在应用变更
    RUN_ONLINE_DISABLE,             // 在线禁用				N9000新增(南方基地)
    RUN_OFFLINE_DISABLE,            // 离线禁用				N9000新增(南方基地)
    RUN_ONLINE_FROZEN,              // 在线冻结				N9000新增(南方基地)
    RUN_OFFLINE_FROZEN,             // 离线冻结				N9000新增(南方基地)
    RUN_CLOSED,                     // 已关闭				N9000新增(南方基地)
    RUN_REMOVING,                   // (节点)删除中			N9000新增(南方基地)
    RUN_INSERVICE,                  // 服务中				N9000新增(南方基地)
    RUN_OUTOFSERVICE,               // 退出服务				N9000新增(南方基地)
    RUN_ERASEMENT_RUNNING_NORMAL,   // 正在销毁				用于数据销毁
    RUN_ERASEMENT_RUNNING_FAIL,     // 销毁失败				用于数据销毁
    RUN_ERASEMENT_RUNNING_SUCCESS,  // 销毁成功				用于数据销毁
    RUN_SUCCESS,                    // 任务执行成功			备份恢复新增
    RUN_FAILED,                     // 任务执行失败			备份恢复新增
    RUN_WAITING,                    // 任务正在等待			备份恢复新增
    RUN_CANCELLING,                 // 任务正在取消			备份恢复新增
    RUN_CANCELLED,                  // 任务已取消			备份恢复新增
    RUN_RUNNING_PREPARE_SYNC,       // 在线|即将灾备同步	备份恢复新增
    RUN_RUNNING_SYNCHRONINING,      // 在线|正在灾备同步	备份恢复新增
    RUN_RUNNING_SYNC_FAILED,        // 在线|灾备同步失败	备份恢复新增
    RUN_MIGRATION_FAULT,            // 迁移故障				LUN迁移
    RUN_MiGRATING,                  // 迁移中				LUN迁移
    RUN_MIGRATION_COMPLETED,        // 迁移完成				LUN迁移
    RUN_ACTIVATING,                 // 正在激活				IOClass运行状态
    RUN_DEACTIVATING,               // 正在取消激活			IOClass运行状态
    RUN_START_FAILED,               // 启动失败				9000 C01
    RUN_STOP_FAILED,                // 停止失败				9000 C01
    RUN_DECOMMISSIONING,            // 正在退出服务			9000 C01
    RUN_DECOMMISSIONED,             // 已经退出服务			9000 C01
    RUN_RECOMMISSIONING,            // 重新进入服务			9000 C01
    RUN_REPLACING_NODE,             // 正在替换节点			9000 C10
    RUN_SCHEDULING,                 // 正在调度				9000 C20
    RUN_PAUSING,                    // 暂停中				9000 C20
    RUN_SUSPENDING,                 // 挂起中				9000 C20
    RUN_SUSPENDED,                  // 挂起					9000 C20
    RUN_OVERLOAD,                   // 超载					防病毒
    RUN_TO_BE_SWITCH,               // 等待切换				租户迁移
    RUN_SWITCHING,                  // 切换中				租户迁移
    RUN_TO_BE_CLEANUP,              // 等待清理				租户迁移
    RUN_FORCED_START,               // 运行状态：强制启动	双活定义
    RUN_ERROR,                      // 运行状态：故障		双活定义
    RUN_JOB_COMPLETED,              // 任务结束				一体化备份新增
    RUN_PARTITION_MIGRATING,        // 分区迁移中			OceanStor 5800 V3 V300R002C10B032
    RUN_Mount,                      // 已挂载
    RUN_Umount,                     // 未挂载
    RUN_INSTALLING,                 // 正在安装中
    RUN_TO_BE_SYNCHRONIZED,         // 待同步				双活定义
    RUN_CONNECTING,                 // 正在连接				15H1复制业务新增
    RUN_SERVICE_SWITCHING,          // 服务正在切换中		9000 V3R5新增
    RUN_POWER_ON_ERROR              // 上电失败
} RUNNING_STATUS_E;

// LUN的空间分配类型
typedef enum {
    SAN_FAT = 0,       // Thick LUN
    SAN_THIN,          // Thin LUN
    SAN_UNKNOWN = 100  // 初始值用
} LUN_ALLOC_TYPE_E;

// 写策略
typedef enum {
    SAN_WRITE_BACK = 1,        // 回写
    SAN_WRITE_THROUGH,         // 透写
    SAN_WRITE_BACK_MANDATORY,  // 强制回写
    SAN_MIXED                  // 混合
} CACHE_WRITE_BACK_E;

// 镜像策略
typedef enum {
    SAN_CACHE_MIRROR_DISABLE = 0,  // 非镜像
    SAN_CACHE_MIRROR_ENABLE        // 镜像
} CACHE_MIRROR_STATUS_E;

// 预取策略
typedef enum {
    SAN_CACHE_READ_AHEAD_STRATEGY_NOTHING = 0,  // 不预取
    SAN_CACHE_READ_AHEAD_STRATEGY_FASTNESS,     // 固定预取
    SAN_CACHE_READ_AHEAD_STRATEGY_MULTIPLIER,   // 可变预取
    SAN_CACHE_READ_AHEAD_STRATEGY_INTELLIGENT   // 智能预取
} CACHE_READ_AHEAD_STRATEGY_E;

// LUN 数据迁移策略
typedef enum {
    SAN_MIGRATE_LUN_POLICY_NONE = 0,  // 不迁移
    SAN_MIGRATE_LUN_POLICY_AUTO,      // 自动迁移
    SAN_MIGRATE_LUN_POLICY_HIGHEST,   // 向高性能层迁移
    SAN_MIGRATE_LUN_POLICY_LOWEST     // 向低性能层迁移
} MIGRATE_LUN_POLICY_E;

// 初始分配策略
typedef enum {
    SAN_INIT_TIER_POLICY_AUTO = 0,             // 自动
    SAN_INIT_TIER_POLICY_EXTREME_PERFORMANCE,  // 最高性能
    SAN_INIT_TIER_POLICY_PERFORMANCE,          // 性能
    SAN_INIT_TIER_POLICY_CAPACITY              // 容量
} INIT_TIER_POLICY_E;

// 应用类型
typedef enum {
    APP_OTHER = 0,  // 其他
    APP_ORACEL,
    APP_EXCHANGE,
    APP_SQLSERVER,
    APP_VMWARE,
    APP_HYPER_V,
    APP_UNKNOWN = 100
} APP_TYPE_E;

// 组类型
typedef enum {
    GRP_LUNGRP = 0,
    GRP_FSGRP = 1,
    GRP_UNKNOWN = 100
} GROUP_TYPE_E;

// 操作系统
typedef enum {
    OS_LINUX = 0,
    OS_WINDOWS,
    OS_SOLARIS,
    OS_HPUX,
    OS_AIX,
    OS_XENSERVER,
    OS_MACOS,
    OS_ESX,
    OS_LINUX_VIS,
    OS_WINDOWS_SERVER,
    OS_UNKNOWN = 100
} OS_TYPE_E;

// 端口类型
typedef enum {
    PORT_HOST = 0,             // 主机端口/业务端口
    PORT_EXP,                  // 级联端口
    PORT_MNGT,                 // 管理端口
    PORT_INNER,                // 内部端口
    PORT_MAINTEGERENANCE,      // 维护端口
    PORT_MNGT_SRV,             // 管理/业务混合端口
    PORT_MAINTEGERENANCE_SRV,  // 维护/业务混合端口
    PORT_UNKNOWN = 100         // 未知
} PORT_LOGIC_TYPE_E;

// 启动器/目标
typedef enum {
    IT_UNKNOWN = 0,  // 未知
    IT_INI = 2,      // 启动器
    IT_TGT,          // 目标器
    IT_INI_AND_TGT   // 启动器和目标器
} INI_OR_TGT_E;

// FC端口模式
typedef enum {
    FC_FABRIC = 0,
    FC_LOOP,
    FC_POINT2POINT,
    FC_AUTO,
    FC_UNKNOWN = 100,
} FC_PORT_MODE_E;

typedef enum {
    SYSTEM_BUSY = 1077949006,
    SYSTEM_TIMEOUT = 1077949001
} ERROR_CODE_E;

typedef enum {
    NOT_RUN_UPRESCAN = 0,
    RUNNING_UPRESCAN,
    RUNNED_UPRESCAN
} UP_RESCAN_STATE_E;
}
#endif
