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
#ifndef _AGENT_ERROR_CODE_H_
#define _AGENT_ERROR_CODE_H_

#include "common/Types.h"
#include <map>
#include <set>
// BEGIN********内部错误码，不返回给Server(部分函数保留内部错误码，需要时直接返回给Server)
static const mp_int32 ERROR_INNER_THAW_NOT_MATCH = 0x70000001;  // 解冻操作和冻结操作不匹配

// BEGIN***********************R5版本返回给server错误码，范围0x4003291A-0x400329FF************************//
static const mp_int32 ERROR_COMMON_OPER_FAILED = 0x5E02500B;  // 执行失败

// ************公共错误码***************************范围0x4003291A - 0x4003294F//
static const mp_int32 ERROR_COMMON_INVALID_PARAM = 0x4003291A;                    // 参数错误
static const mp_int32 ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED = 0x4003291B;         // 检查脚本签名失败
static const mp_int32 ERROR_COMMON_RECOVER_INSTANCE_NOSTART = 0x4003291C;         // 数据库实例未启动
static const mp_int32 ERROR_COMMON_DB_USERPWD_WRONG = 0x4003291D;                 // 数据库用户名或密码错误
static const mp_int32 ERROR_COMMON_DB_INSUFFICIENT_PERMISSION = 0x4003291E;       // 数据库用户权限不足
static const mp_int32 ERROR_COMMON_FUNC_UNIMPLEMENT = 0x4003291F;                 // 功能未实现
static const mp_int32 ERROR_COMMON_READ_CONFIG_FAILED = 0x40032921;               // 读取配置文件失败
static const mp_int32 ERROR_COMMON_DLL_LOAD_FAILED = 0x40032924;                  // 动态库加载失败
static const mp_int32 ERROR_COMMON_SYSTEM_CALL_FAILED = 0x40032925;               // 系统调用失败(替换成具体场景错误码)
static const mp_int32 ERROR_COMMON_CLIENT_IS_LOCKED = 0x40032926;                 // 客户端被锁定(描述信息待讨论)
static const mp_int32 ERROR_COMMON_SCRIPT_FILE_NOT_EXIST = 0x40032927;            // 脚本文件不存在
static const mp_int32 ERROR_COMMON_SCRIPT_EXEC_FAILED = 0x40032928;               // 脚本执行失败
static const mp_int32 ERROR_COMMON_PLUGIN_LOAD_FAILED = 0x40032929;               // 插件加载失败
static const mp_int32 ERROR_COMMON_NOT_HUAWEI_LUN = 0x4003292B;                   // 存储信息无法识别
static const mp_int32 ERROR_COMMON_USER_OR_PASSWD_IS_WRONG = 0x4003292C;          // 用户名或密码错误
static const mp_int32 ERROR_COMMON_QUERY_APP_LUN_FAILED = 0x4003292F;             // 查询应用LUN信息失败
static const mp_int32 ERROR_COMMON_DEVICE_NOT_EXIST = 0x40032931;                 // 指定设备不存在
static const mp_int32 ERROR_COMMON_APP_FREEZE_FAILED = 0x40032932;                // 冻结失败(Oracle/db2不适用冻结术语,需修改)
static const mp_int32 ERROR_COMMON_APP_THAW_FAILED = 0x40032933;                  // 解冻失败(Oracle/db2不适用冻结术语,需修改)
static const mp_int32 ERROR_COMMON_APP_FREEZE_TIMEOUT = 0x40032934;               // 冻结超时(Oracle/db2不适用冻结术语,需修改)
static const mp_int32 ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE = 0x40032935;  // 不支持数据库文件部署在不同类型的磁盘上
static const mp_int32 ERROR_COMMON_PROC_REQUEST_BUSY = 0x40032936;                // Agent业务忙
static const mp_int32 ERROR_COMMON_DB_FILE_NOT_EXIST = 0x40032937;                // 数据库文件不存在
static const mp_int32 ERROR_COMMON_JSON_KEY_NOT_EXIST = 0x40032938;               // json key不存在
static const mp_int32 ERROR_COMMON_TASK_NOT_EXIST     = 0x40032939;               // 任务不存在
static const mp_int32 ERROR_COMMON_BODY_TOO_LONG      = 0x4003293A;               // 请求Body过长
static const mp_int32 ERROR_COMMON_MSG_BLOCK_OVERLOAD = 0x4003293B;               // HTTP流中获取的消息块大小超过限制

// dataturbo 错误码
static const mp_int32 ERR_CREATE_DATA_TURBO_LINK = 0x5E006702;            // dataturbo建联失败
static const mp_int32 ERR_MOUNT_DATA_TURBO_FILE_SYSTEM = 0x5E025C01;      // dataturbo 挂载文件系统失败
static const mp_int32 ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT = 0x5E025C02;  // 未配置Dataturbo协议的逻辑端口
static const mp_int32 ERR_NOT_SUPPORT_DATA_TURBO = 0x64044341;  // 内置agent不支持Dataturbo

// sanclient 错误码
static const mp_int32 ERR_SANCLIENT_PREPAREJOB_FAILED = 0x5E037303;
static const mp_int32 ERR_CREATE_FILEIO_FAILED = 0x5E037302;
static const mp_int32 ERR_TARGETCLI_STATUS = 0x5E037300;
static const mp_int32 ERR_QLA2XXX_STAUTS_FAILED = 0x5E037301;

// ************Array&Disk***********************************范围0x40032950 - 0x4003295F//
static const mp_int32 ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED = 0x40032954;  // 获取磁盘的裸设备名称失败
static const mp_int32 ERROR_DISK_ONLINE_FAILED = 0x40032956;               // 上线磁盘失败
static const mp_int32 ERROR_DISK_SCAN_FAILED = 0x40032957;                 // 扫描磁盘失败
static const mp_int32 ERROR_DISK_GET_PARTITION_INFO_FAILED = 0x40032958;   // 获取磁盘分区信息失败
static const mp_int32 ERROR_DISK_GET_DISK_INFO_FAILED = 0x40032959;        // 获取磁盘信息失败
static const mp_int32 ERROR_DISK_GET_VOLUME_PATH_FAILED = 0x4003295A;      // 获取卷路径失败

// ************Host***********************************范围0x40032960 - 0x4003296F//
static const mp_int32 ERROR_HOST_VERIFY_SNMP_FAILED = 0x40032960;         // SNMP协议参数不匹配
static const mp_int32 ERROR_HOST_GETINFO_FAILED = 0x40032961;             // 查询主机信息失败
static const mp_int32 ERROR_HOST_UNREG_TRAPSERVER_FAILED = 0x40032962;    // 删除Trap IP地址失败
static const mp_int32 ERROR_HOST_THIRDPARTY_GETFILE_FAILED = 0x40032963;  // 查询第三方脚本失败
static const mp_int32 ERROR_HOST_THIRDPARTY_EXEC_FAILED = 0x40032964;     // 执行第三方脚本失败
static const mp_int32 ERROR_HOST_REG_TRAPSERVER_FAILED = 0x40032965;      // 注册Trap IP地址失败
static const mp_int32 ERROR_HOST_GET_INIATOR_FAILED = 0x40032966;         // 查询启动器信息失败
static const mp_int32 ERROR_HOST_GET_TIMEZONE_FAILED = 0x40032967;        // 查询主机时区信息失败
static const mp_int32 ERROR_HOST_LOG_IS_BEENING_COLLECTED = 0x40032967;   // 日志正在收集
static const mp_int32 ERROR_HOST_LOG_LINK_ISCSITARGET = 0x40032968;       // 连接iscsi target失败
static const mp_int32 ERROR_HOST_GETINFO_AGENT_FAILED = 0x40032969;       // 获取agent信息失败
static const mp_int32 ERROR_HOST_UPGRADE_AGENT_FAILED = 0x4003296A;       // 更新agent失败
static const mp_int32 ERROR_AGENT_UPGRADE_FAIL_DOWNLOAD_PACKAGE = 0x5E02500D;  // 下载升级包失败
static const mp_int32 ERROR_AGENT_EXPORT_LOG_SIZE_INVALID = 0x64034807;   // 日志包超过导出上限
static const mp_int32 ERROR_AGENT_DISK_NOT_ENOUGH = 0x64033361;           // 磁盘空间不足
static const mp_int32 ERROR_AGENT_GET_LOG_FILE_FAILED = 0x64034808;       // 压缩日志失败

// ************Device*********************************范围0x40032970 - 0x4003298F//
// filesys
static const mp_int32 ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST = 0x40032970;        // 挂载点目录不存在
static const mp_int32 ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED = 0x40032971;         // 设备已经挂载到其他挂载点
static const mp_int32 ERROR_DEVICE_FILESYS_MOUNT_POINT_OCCUPIED = 0x40032972;         // 指定的挂载点已经被占用
static const mp_int32 ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED = 0x40032973;        // 下线卷失败
static const mp_int32 ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED = 0x40032974;  // 删除盘符失败
static const mp_int32 ERROR_DEVICE_FILESYS_UNMOUNT_FAILED = 0x40032975;               // 去挂载失败
static const mp_int32 ERROR_DEVICE_FILESYS_GET_DEV_FAILED = 0x40032976;               // 查询设备信息失败
static const mp_int32 ERROR_DEVICE_FILESYS_MOUNT_FAILED = 0x40032977;                 // 挂载失败
static const mp_int32 ERROR_DEVICE_FILESYS_QUERY_INFO_FAILED = 0x40032978;            // 查询文件系统信息失败
// raw
static const mp_int32 ERROR_DEVICE_RAW_USED_BY_OTHER_DEV = 0x40032979;  // 裸设备已被占用
static const mp_int32 ERROR_DEVICE_RAW_START_FAILED = 0x4003297A;       // 启动裸设备服务失败
static const mp_int32 ERROR_DEVICE_RAW_DELETE_FAILED = 0x4003297B;      // 删除裸设备失败
static const mp_int32 ERROR_DEVICE_RAW_CREATE_FAILED = 0x4003297C;      // 创建裸设备失败
// lvm
static const mp_int32 ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED = 0x4003297D;  // 查询卷组信息失败
static const mp_int32 ERROR_DEVICE_LVM_EXPORT_VG_FAILED = 0x4003297E;        // 导出卷组失败
static const mp_int32 ERROR_DEVICE_LVM_IMPORT_VG_FAILED = 0x4003297F;        // 导入卷组失败
static const mp_int32 ERROR_DEVICE_LVM_GET_PV_FAILED = 0x40032980;           // 查询物理卷信息失败
static const mp_int32 ERROR_DEVICE_LVM_DEACTIVE_VG_FAILED = 0x40032982;      // 去激活卷组失败
static const mp_int32 ERROR_DEVICE_VXVM_SCAN_DISK_FAILED = 0x4003298A;       // VXVM扫描磁盘失败

static const mp_int32 ERROR_DEVICE_LVM_CREATE_LV_FAILED = 0x5E03730E;     // 创建LV失败
static const mp_int32 ERROR_DEVICE_LVM_CREATE_PV_FAILED = 0x5E03730F;     // 创建PV失败
static const mp_int32 ERROR_DEVICE_LVM_CREATE_VG_FAILED = 0x5E037310;     // 创建VG失败
static const mp_int32 ERROR_DEVICE_SCAN_ADAPTER_FAILED = 0x5E037311;     // 扫描设备失败
static const mp_int32 ERROR_DEVICE_SCAN_DISK_FAILED = 0x5E037312;     // 发现磁盘设备失败
static const mp_int32 ERROR_DEVICE_LVM_ACTIVE_VG_FAILED = 0x5E037313;     // 激活卷组失败


// link
static const mp_int32 ERROR_DEVICE_LINK_USED_BY_OTHER_DEV = 0x40032983;  // 软连接已被占用
static const mp_int32 ERROR_DEVICE_LINK_CREATE_FAILED = 0x40032984;      // 创建软连接失败
static const mp_int32 ERROR_DEVICE_LINK_DELETE_FAILED = 0x40032985;      // 删除软连接失败
// udev
static const mp_int32 ERROR_DEVICE_UDEV_CREATE_FAILED = 0x40032986;  // 写入udev规则失败
static const mp_int32 ERROR_DEVICE_UDEV_DELETE_FAILED = 0x40032987;  // 删除udev规则失败
// multipath
static const mp_int32 ERROR_DEVICE_MULTIPATH_CREATE_FAILED = 0x400329FA;  // 写入multipath规则失败
static const mp_int32 ERROR_DEVICE_MULTIPATH_DELETE_FAILED = 0x400329FB;  // 删除multipath规则失败
// asm
static const mp_int32 ERROR_DEVICE_ASM_SCAN_ASMLIB_FAILED = 0x40032988;  // 扫描ASM磁盘失败
// permission
static const mp_int32 ERROR_DEVICE_PERMISSION_SET_FAILED = 0x40032989;  // 设置权限失败

// ***********Oracle错误码****************************范围0x40032990 - 0x400329AF//
static const mp_int32 ERROR_ORACLE_ASM_DBUSERPWD_WRONG = 0x40032990;           // ASM用户或密码错误
static const mp_int32 ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART = 0x40032991;  // ASM实例未启动
static const mp_int32 ERROR_ORACLE_ASM_INSUFFICIENT_WRONG = 0x40032992;        // ASM用户权限不足
static const mp_int32 ERROR_ORACLE_NOARCHIVE_MODE = 0x40032993;                // 数据库未开启归档模式
static const mp_int32 ERROR_ORACLE_OVER_ARCHIVE_USING = 0x40032994;            // 数据库归档目录空闲空间超过阈值
static const mp_int32 ERROR_ORACLE_OVER_MAX_LINK = 0x40032995;                 // 数据库连接已超过最大连接数
static const mp_int32 ERROR_ORACLE_IN_BACKUP = 0x40032996;                     // 数据库已处于热备模式
static const mp_int32 ERROR_ORACLE_NOT_IN_BACKUP = 0x40032997;                 // 数据库未处于热备模式
static const mp_int32 ERROR_ORACLE_ARCHIVE_FAILED = 0x40032998;                // 数据库强制归档失败
static const mp_int32 ERROR_ORACLE_DB_ALREADYRUNNING = 0x40032999;             // 数据库已处于运行状态
static const mp_int32 ERROR_ORACLE_DB_ALREADYMOUNT = 0x4003299A;               // 数据库已处于挂载状态
static const mp_int32 ERROR_ORACLE_DB_ALREADYOPEN = 0x4003299B;                // 数据库已处于打开状态
static const mp_int32 ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT = 0x4003299C;    // ASM磁盘组已被挂载
static const mp_int32 ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT = 0x4003299D;        // ASM磁盘组未被挂载
static const mp_int32 ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED = 0x4003299E;       // 数据库开启热备模式失败
static const mp_int32 ERROR_ORACLE_END_HOT_BACKUP_FAILED = 0x4003299F;         // 数据库结束热备模式失败
static const mp_int32 ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT = 0x400329A0;      // 数据库开启热备模式超时
static const mp_int32 ERROR_ORACLE_TRUNCATE_LOG_FAILED = 0x400329A1;           // 删除数据库实例的归档日志失败
static const mp_int32 ERROR_ORACLE_TNS_PROTOCOL_ADAPTER = 0x400329A2;          // TNS适配器错误
static const mp_int32 ERROR_ORACLE_START_INSTANCES_FAILED = 0x400329A3;        // 启动数据库实例失败
static const mp_int32 ERROR_ORACLE_INSTANCE_NOT_CDB = 0x400329A4;              // 查询插件数据库时下发的实例不是容器数据库实例
static const mp_int32 ERROR_ORACLE_EXESQL_FAILED = 0x400329A5;                  // 执行oracle sql失败
static const mp_int32 ERROR_ORACLE_EXERMAN_FAILED = 0x400329A6;                 // 执行oracle rman失败
static const mp_int32 ERROR_ORACLE_EXEASMCMD_FAILED = 0x400329A7;               // 执行oracle asmcmd失败
static const mp_int32 ERROR_ORACLE_RECOVERPATH_NOT_EXIT = 0x400329A8;           // oracle目标恢复目录不存在
static const mp_int32 ERROR_ORACLE_DB_NOT_MOUNT = 0x400329A9;                   // oracle数据库未挂载
static const mp_int32 ERROR_ORACLE_DB_NOT_OPEN = 0x400329AA;                    // oracle数据库未打开
static const mp_int32 ERROR_ORACLE_DISKGROUP_NOT_EXIT = 0x400329AB;             // 磁盘组不存在
static const mp_int32 ERROR_ORACLE_BASE_NOT_EXIT = 0x400329AC;                  // ORACLE_HOME 不存在
static const mp_int32 ERROR_ORACLE_HOME_NOT_EXIT = 0x400329AD;                  // ORACLE_BASE 不存在
static const mp_int32 ERROR_ORACLE_GET_ASMDG_FAILED = 0x400329AE;               // 获取ASM磁盘组错误
static const mp_int32 ERROR_ORACLE_START_RACDB = 0x400329AF;                    // 启动集群数据库错误

// ***********DB2错误码*******************************范围0x400329B0 - 0x400329B9//
static const mp_int32 ERROR_DB2_SUSPEND_IO_FAILED = 0x400329B0;   // 数据库悬挂IO失败
static const mp_int32 ERROR_DB2_RESUME_IO_FAILED = 0x400329B1;    // 数据库解除悬挂IO失败
static const mp_int32 ERROR_DB2_SUSPEND_IO_TIMEOUT = 0x400329B2;  // 数据库悬挂IO超时

// **********SqlServer错误码**************************范围0x400329BA - 0x400329C9//
static const mp_int32 ERROR_SQLSERVER_GET_DB_STATUS_FAILED = 0x400329BA;   // 查询数据库状态失败
static const mp_int32 ERROR_SQLSERVER_DB_STATUS_OFFLINE = 0x400329BB;      // 数据库不在线
static const mp_int32 ERROR_SQLSERVER_DB_NOT_EXIST = 0x400329BC;           // 数据库不存在
static const mp_int32 ERROR_SQLSERVER_INSTANCE_NOT_EXIST = 0x400329BD;     // 数据库实例不存在
static const mp_int32 ERROR_SQLSERVER_START_INSTANCE_FAILED = 0x400329BE;  // 启动数据库实例失败
static const mp_int32 ERROR_SQLSERVER_DB_LIST_IS_NULL = 0x400329BF;        // 数据库信息列表为空
static const mp_int32 ERROR_SQLSERVER_START_DB_FAILED = 0x400329C0;        // 启动数据库失败
static const mp_int32 ERROR_SQLSERVER_STOP_DB_FAILED = 0x400329C1;         // 停止数据库失败

// **********Exchange错误码**************************范围0x400329CA - 0x400329D5//
static const mp_int32 ERROR_EXCHANGE_REMOVE_FAILED = 0x400329CA;        // 邮箱数据库卸载清理失败
static const mp_int32 ERROR_EXCHANGE_MOUNT_FAILED = 0x400329CB;         // 邮箱数据库装载启动失败
static const mp_int32 ERROR_EXCHANGE_SOFTRECVERY_FAILED = 0x400329CC;   // 邮箱数据库软恢复失败
static const mp_int32 ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL = 0x400329CD;  // 多AD域下挂载失败

// **********Cluster错误码****************************范围0x400329D6 - 0x400329EF//
static const mp_int32 ERROR_CLUSTER_QUERY_FAILED = 0x400329D6;                     // 查询集群信息失败
static const mp_int32 ERROR_CLUSTER_QUERY_NODE_FAILED = 0x400329D7;                // 查询集群节点信息失败
static const mp_int32 ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED = 0x400329D8;       // 查询集群服务状态失败
static const mp_int32 ERROR_CLUSTER_START_SERVICE_FAILED = 0x400329D9;             // 启动集群服务失败
static const mp_int32 ERROR_CLUSTER_PACKAGE_ONLINE_FAILED = 0x400329DA;            // 上线程序包(资源组)失败
static const mp_int32 ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED = 0x400329DD;           // 下线程序包(资源组)失败
static const mp_int32 ERROR_CLUSTER_QUERY_ACTIVE_HOST_FAILED = 0x400329DE;         // 查询活动节点失败
static const mp_int32 ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED = 0x400329DF;          // 查询程序包(资源组)信息失败
static const mp_int32 ERROR_CLUSTER_SQLSERVER_RESOURCE_NOT_EXIST = 0x400329E0;     // SQL Server资源组不存在
static const mp_int32 ERROR_CLUSTER_GET_CLUSTER_NETWORK_NAME_FAILED = 0x400329E1;  // 查询网络资源信息失败
static const mp_int32 ERROR_CLUSTER_GET_DISK_PATITION_TYPE_FAILED = 0x400329E2;    // 查询磁盘的分区类型失败
static const mp_int32 ERROR_CLUSTER_GET_DISK_RESOURCE_FAILED = 0x400329E3;         // 查询磁盘资源信息失败
static const mp_int32 ERROR_CLUSTER_RESUME_DISK_RESOURCE_FAILED = 0x400329E4;      // 恢复磁盘资源失败
static const mp_int32 ERROR_CLUSTER_REPAIR_DISK_RESOURCE_FAILED = 0x400329E5;      // 修复磁盘资源失败
static const mp_int32 ERROR_CLUSTER_ONLINE_DISK_RESOURCE_FAILED = 0x400329E6;      // 上线磁盘资源失败
static const mp_int32 ERROR_CLUSTER_SUSPEND_DISK_RESOURCE_FAILED = 0x400329E7;     // 挂起磁盘资源失败
static const mp_int32 ERROR_CLUSTER_SERVICE_NOSTART = 0x400329E8;                  // 集群服务未启动
static const mp_int32 ERROR_CLUSTER_DB_NOT_INCLUSTER = 0x400329E9;                 // 数据库未加入集群
static const mp_int32 ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL = 0x400329EA;         // 资源状态异常
static const mp_int32 ERROR_CLUSTER_NOT_ACTIVE_NODE = 0x400329EB;                  // 集群节点为非活动节点

// **********VSS错误码****************************范围0x400329F0 - 0x400329FF//
static const mp_int32 ERROR_VSS_INIT_FILEDES_GETVOLUME_FAILED = 0x400329F0;  // 初始化文件信息时获取卷信息失败
static const mp_int32 ERROR_VSS_TIME_OUT = 0x400329F1;                       // VSS操作超时
static const mp_int32 ERROR_VSS_FREEZE_TIMEOUT = 0x400329F2;                 // 冻结超时
static const mp_int32 ERROR_VSS_OTHER_FREEZE_RUNNING = 0x400329F3;           // 已经有冻结操作在执行
static const mp_int32 ERROR_VSS_EXCHANGE_DB_NOT_EXIST = 0x400329F4;          // 指定的存储组或邮箱数据库不存在

// Sun Cluster
static const mp_int32 ERROR_DEVICE_VXVM_EXPORT_DG_FAILED = 0x400329F5;        // 导出磁盘组失败
static const mp_int32 ERROR_DEVICE_VXVM_IMPORT_DG_FAILED = 0x400329F6;        // 导入磁盘组失败
static const mp_int32 ERROR_DEVICE_VXVM_ACTIVE_DG_FAILED = 0x400329F7;        // 激活磁盘组失败
static const mp_int32 ERROR_DEVICE_VXVM_DEACTIVE_DG_FAILED = 0x400329F8;      // 去激活磁盘组失败
static const mp_int32 ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED = 0x400329F9;  // 查询磁盘信息失败

// **********APP错误码****************************范围0x40032A00 - 0x40032A0F//
static const mp_int32 ERROR_APP_FREEZE_TOO_FREQUENCE_FAILED = 0x40032A00;  // APP冻结接口调用太频繁

// **********v2c error code **********************range 0x40032B00 -0X40032B0F
static const mp_int32 ERROR_LINUX_V2C_MOUNT_FAILED = 0x400329FC;
static const mp_int32 ERROR_LINUX_V2C_CHECK_FSTAB_FAILED = 0x400329FD;
static const mp_int32 ERROR_LINUX_V2C_VERSION_UNSUPPORTED = 0x400329FE;
static const mp_int32 ERROR_LINUX_V2C_GRUBFILE_NOT_FOUND = 0x400329FF;
static const mp_int32 ERROR_LINUX_V2C_INITRD_NOT_FOUND = 0x40032A00;
static const mp_int32 ERROR_LINUX_V2C_MODIFY_GRUB_FAILED = 0x40032A04;
static const mp_int32 ERROR_LINUX_V2C_DELETE_OLD_DRIVER_FAILED = 0x40032A05;
static const mp_int32 ERROR_LINUX_V2C_DELETE_VMWARE_TOOL_FAILED = 0x40032A06;
static const mp_int32 ERROR_LINUX_V2C_ADD_KERNEL_PARAM_FAILED = 0x40032A07;
static const mp_int32 ERROR_LINUX_V2C_MODIFY_MKINITRD_CONF_FAILED = 0x40032A08;
static const mp_int32 ERROR_LINUX_V2C_REMAKE_INITRD_FAILED = 0x40032A09;
static const mp_int32 ERROR_WINDOWS_V2C_RESCAN_DISK_FAILED = 0x40032A0A;
static const mp_int32 ERROR_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE = 0x40032A0B;
static const mp_int32 ERROR_WINDOWS_V2C_REGLOAD_FAILED = 0x40032A0C;
static const mp_int32 ERROR_WINDOWS_V2C_VERSION_UNSUPPORTED = 0x40032A0D;
static const mp_int32 ERROR_WINDOWS_V2C_INSTALL_DRIVER_FAILED = 0x40032A0E;

// **********Mobility错误码****************************范围0x40032B00 - 0x40032BFF//
static const mp_int32  ERROR_MOBILITY_PROTECT_STARTED     = 0x40032B00;  // 保护操作已经开始执行
static const mp_int32  ERROR_MOBILITY_PROTECT_STOPED      = 0x40032B01;  // Mobility has not started protect
static const mp_int32  ERROR_MOBILITY_PROTECT_CONFIGURE   = 0x40032B02;  // Mobility configure failed
static const mp_int32  ERROR_MOBILITY_PROTECT_DISK_REDUCE = 0x40032B03;  // 保护的磁盘被减去
static const mp_int32  ERROR_MOBILITY_HW_INFO_CHANGED     = 0x40032B04;  // VM 硬件信息发生了变化
static const mp_int32  ERROR_MOBILITY_ROLLBACK_ADD_DEL    = 0x40032B05;  // 加减卷操作回滚失败, 导致保护状态不一致
static const mp_int32  ERROR_MOBILITY_ADD_VOL_EXIST       = 0x40032B06;  // 待添加的磁盘盘符或UUID已经存在
static const mp_int32  ERROR_MOBILITY_DEL_VOL_NOT_EXIST   = 0x40032B07;  // 待删除的卷没有保护，无需删除
static const mp_int32  ERROR_MOBILITY_DEL_VOL_NOT_ALLOW   = 0x40032B08;  // driver 处于resync状态，不能删除卷
static const mp_int32  ERROR_MOBILITY_DEL_ONLY_ONE_VOL    = 0x40032B09;  // 待删除的卷是保护信息中唯一的卷，不允许del

// **********备份 操作错误码****************************
static const mp_int32 ERROR_AGENT_INTERNAL_ERROR = 0x5E02500B;                // Agent内部业务错误
static const mp_int32 ERROR_AGENT_DB_ISRUNNING = 0x5E025007;                         // 目标数据库正在运行中
static const mp_int32 ERROR_MOUNTPATH = 0x5E025008;                           // 挂载目录不存在或无操作权限
static const mp_int32 ERROR_ORACLE_VERSION_DISMATCH = 0x5E02500F;             // oracle版本不匹配
static const mp_int32 ERROR_CHECK_ARCHIVELOG_FAILED = 0x5E02503E;                // 校验归档日志失败，日志有空档

static const mp_int32 ERROR_AGENT_PREPARE_FSMEDIA_FAILED = 0x64025002;         // 创建oracle的FS备份介质失败
static const mp_int32 ERROR_AGENT_DISMOUNT_BACKUP_MEDIUM_FAILED = 0x64025003;  // oracle dismount备份或恢复介质失败
static const mp_int32 ERROR_AGENT_PREPARE_ASMMEDIA_FAILED = 0x64025005;        // 创建oracle的ASM备份介质失败
static const mp_int32 ERROR_AGENT_RESTORE_DATABASE_FAILED = 0x64025006;        // 恢复oracle失败
static const mp_int32 ERROR_DISCONNECT_STORAGE_NETWORK = 0x6402500A;           // 创建oracle备份介质失败，与存储网络不通
static const mp_int32 ERROR_MOUNTPATH_OCCUPIED = 0x6402500B;                   // 创建oracle备份介质失败，挂载目录已被占用
static const mp_int32 ERROR_DEL_ARCHIVELOG_FAILED = 0x6402500C;                // 日志过期失败
static const mp_int32 ERROR_FILESYSTEM_NO_SPACE = 0x6402500D;                  // 文件系统没有空间

static const mp_int32 ERROR_ORACLE_CHECKDBOPEN_FAILED = 0x40032B12;                  // 检查Oracle DB是否运行失败

static const mp_int32 ERROR_VMWARE_NETWORK = 0x5E008701;                  // 代理与Vcenter间网络问题

// error code of 100P
static const mp_int32 ERROR_DEVICE_TESTNAS_FAILED = 0x40032B13;             // test nas failed
// nas
static const mp_int32 ERROR_DEVICE_NAS_MOUNT_FAILED = 0x5E025012;  // 挂载nas失败

static const mp_int32 ERROR_VM_PROCESS_RESTART_NEED_RETRY = 0x5E025013;  // DP进程重启

static const mp_int32 ERROR_ESN_MISMATCH = 0x5E025055;  // 执行源端重删操作时ESN不匹配

static const mp_int32 ERROR_MOUNT_SRICPT_FAILED = 0x6402505D;  // mountnasfilesystem脚本执行失败

// *********************框架通用错误码************************
static const mp_int32 ERR_OBJ_NOT_EXIST = 0x64032B01; // 对象不存在
static const mp_int32 ERR_INVALID_PARAM = 0x64032B02;
static const mp_int32 ERR_OPERATION_FAILED = 0x64032B03;        // 通用错误码，操作失败
static const mp_int32 ERR_SYSTEM_EXCEPTION = 0x64032B05;
static const mp_int32 ERR_SYSTEM_RESPONSE_TIMEOUT = 0x64032B0A;
static const mp_int32 ERR_DISCONNECT_STORAGE_NETWORK = 0x64032B0B;  // 与dorado逻辑端口网络不通
static const mp_int32 ERR_PLUGIN_AUTHENTICATION_FAILED = 0x64032B0C;                // 插件鉴权失败
static const mp_int32 ERR_INCONSISTENT_STATUS = 0x5F025196; // 状态不一致
static const mp_int32 ERR_INTERNAL_ERROR = 0x5F025101;
static const mp_int32 ERR_NETWORK_EXCEPTION = 0x5F025116;          // 网络异常, 1593987350
static const mp_int32 ERR_INC_TO_FULL = 0x5E02502D;
static const mp_int32 ERR_LOG_TO_FULL = 0x5E025E4D; // 日志备份转全量
static const mp_int32 ERR_INC_TO_DIFF = 0x5E025110; // 增量备份转差异
static const mp_int32 ERROR_COMMON_INVALID_PARAMETER = 	0x5F025102;
static const mp_int32 ERROR_PLUGIN_CANNOT_EXEC_ON_AGENT = 0x5E02503B; // 插件无法在该Agent执行任务
static const mp_int32 ERROR_INTERNAL = 0x5F02573B; // 任务执行过程中，由于内部错误导致任务失败


// End***********************返回给server错误码**********************************//
// BEGIN***********************脚本错误码，用于脚本错误返回，范围0-255************************//
// ************公共错误码***************************，范围5-19//
static const mp_uchar ERROR_SCRIPT_COMMON_EXEC_FAILED = 5;
static const mp_uchar ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST = 6;
static const mp_uchar ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST = 7;
static const mp_uchar ERROR_SCRIPT_COMMON_PATH_WRONG = 8;
static const mp_uchar ERROR_SCRIPT_COMMON_PARAM_WRONG = 9;
static const mp_uchar ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG = 10;
static const mp_uchar ERROR_SCRIPT_COMMON_INSTANCE_NOSTART = 11;
static const mp_uchar ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG = 15;
static const mp_uchar ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE = 16;
static const mp_uchar ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED = 17;
static const mp_uchar ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED = 18;

// ***********Oracle脚本错误码********************，范围20-69//
static const mp_uchar ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG = 21;
static const mp_uchar ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG = 22;
static const mp_uchar ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART = 23;
static const mp_uchar ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE = 24;
static const mp_uchar ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING = 25;
static const mp_uchar ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT = 26;
static const mp_uchar ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT = 27;
static const mp_uchar ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK = 28;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP = 29;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP = 30;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING = 31;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT = 32;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN = 33;
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR = 34;
static const mp_uchar ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED = 35;
static const mp_uchar ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED = 36;
static const mp_uchar ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT = 37;

static const mp_uchar ERROR_SCRIPT_ORACLE_DB_NOT_MOUNT = 40;        // oracle数据库未挂载
static const mp_uchar ERROR_SCRIPT_ORACLE_DB_NOT_OPEN = 41;         // oracle数据库未打开
static const mp_uchar ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED = 42;
static const mp_uchar ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER = 43;
static const mp_uchar ERROR_SCRIPT_ORACLE_NOT_INSTALLED = 44;
static const mp_uchar ERROR_SCRIPT_ORACLE_INST_NOT_CDB = 47;
static const mp_uchar ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT = 48;
static const mp_uchar ERROR_SCRIPT_ORACLE_START_PDB_FAILED = 49;
static const mp_uchar ERROR_SCRIPT_DB_FILE_NOT_EXIST = 50;

static const mp_uchar ERROR_SCRIPT_DEL_ARCHIVELOG_FAILED = 59;  // 删除归档日志失败
static const mp_uchar ERROR_SCRIPT_CHECK_ARCHIVELOG_FAILED = 60;  // 校验归档日志失败

static const mp_uchar ERROR_SCRIPT_ORACLE_EXESQL_FAILED = 61;           // 执行oracle sql失败
static const mp_uchar ERROR_SCRIPT_ORACLE_EXERMAN_FAILED = 62;          // 执行oracle rman失败
static const mp_uchar ERROR_SCRIPT_ORACLE_EXEASMCMD_FAILED = 63;        // 执行oracle asmcmd失败
static const mp_uchar ERROR_SCRIPT_ORACLE_RECOVERPATH_NOT_EXIT = 64;    // oracle目标恢复目录不存在
static const mp_uchar ERROR_SCRIPT_ORACLE_DISKGROUP_NOT_EXIT = 65;      // 磁盘组不存在
static const mp_uchar ERROR_SCRIPT_DISCONNECT_STORAGE_NETWORK = 66;     // 与存储网络不通
static const mp_uchar ERROR_SCRIPT_MOUNTPATH_OCCUPIED = 67;             // 挂载目录已被占用
static const mp_uchar ERROR_SCRIPT_ORACLEHOME_LOST = 68;                // ORACLE_HOME不存在
static const mp_uchar ERROR_SCRIPT_ORACLEBASE_LOST = 69;                // ORACLE_BASE不存在

// **********DB2脚本错误码*********************，范围 70-99 //
static const mp_uchar ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED = 70;
static const mp_uchar ERROR_SCRIPT_DB2_RESUME_IO_FAILED = 71;
static const mp_uchar ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT = 72;
// **********Exchange脚本错误码*********************，范围 100-129 //
static const mp_uchar ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED = 100;
static const mp_uchar ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED = 101;
static const mp_uchar ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED = 102;
static const mp_uchar ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL = 103;
// **********SqlServer脚本错误码*********************，范围 130-159 //
static const mp_uchar ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR = 130;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED = 131;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_132 = 132;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_133 = 133;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED = 134;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE = 135;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST = 136;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST = 137;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED = 138;
static const mp_uchar ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED = 139;
// **********Cluster脚本错误码*********************，范围 160-189 //
static const mp_uchar ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART = 160;
static const mp_uchar ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER = 161;
static const mp_uchar ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL = 162;
static const mp_uchar ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED = 163;
static const mp_uchar ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED = 164;
static const mp_uchar ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE = 165;
// **********APP脚本错误码*********************，范围 190-199 //
static const mp_uchar ERROR_SCRIPT_MOUNTPATH = 185;                 // 挂载目录不存在或无操作权限
static const mp_uchar ERROR_SCRIPT_ORACLE_VERSION_DISMATCH = 186;   // oracle版本不匹配
static const mp_uchar ERROR_SCRIPT_ASM_ASMDGNAME = 188;
static const mp_uchar ERROR_SCRIPT_START_RACDB = 189;
static const mp_uchar ERROR_SCRIPT_APP_FAILED = 190;
static const mp_uchar ERROR_SCRIPT_CREATE_LV = 193;
static const mp_uchar ERROR_SCRIPT_CREATE_PV = 194;
static const mp_uchar ERROR_SCRIPT_CREATE_VG = 195;
static const mp_uchar ERROR_SCRIPT_SCAN_ADAPTER = 196;
static const mp_uchar ERROR_SCRIPT_SCAN_DISK = 197;
static const mp_uchar ERROR_SCRIPT_VARYONVG = 198;
static const mp_uchar ERROR_SCRIPT_MOUNT = 199;

// *********v2c error code******************** range 200-219 //
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_MOUNT_FAILED = 200;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_CHECK_FSTAB_FAILED = 201;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_VERSION_UNSUPPORTED = 202;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_GRUBFILE_NOT_FOUND = 203;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_INITRD_NOT_FOUND = 204;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_MODIFY_GRUB_FAILED = 205;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_DELETE_OLD_DRIVER_FAILED = 206;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_DELETE_VMWARE_TOOL_FAILED = 207;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_ADD_KERNEL_PARAM_FAILED = 208;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_MODIFY_MKINITRD_CONF_FAILED = 209;
static const mp_uchar ERROR_SCRIPT_LINUX_V2C_REMAKE_INITRD_FAILED = 210;
static const mp_uchar ERROR_SCRIPT_WINDOWS_V2C_RESCAN_DISK_FAILED = 211;
static const mp_uchar ERROR_SCRIPT_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE = 212;
static const mp_uchar ERROR_SCRIPT_WINDOWS_V2C_REGLOAD_FAILED = 213;
static const mp_uchar ERROR_SCRIPT_WINDOWS_V2C_VERSION_UNSUPPORTED = 214;
static const mp_uchar ERROR_SCRIPT_WINDOWS_V2C_INSTALL_DRIVER_FAILED = 215;

// **********sanclient脚本错误码*********************，范围 220-225 //
static const mp_uchar ERROR_TARGETCLI_STATUS = 220;
static const mp_uchar ERROR_QLA2XXX_STAUTS = 222;

static const mp_int32 ERROR_SCRIPT_DISK_NOT_ENOUNGH = 230;
static const mp_int32 INTER_ERROR_SRCIPT_FILE_NOT_EXIST = 255;

static const mp_int32 SPEC_COMMID_QUERYORACLEINFO = 9;
// *********************框架内部错误码************************
static const mp_int32 NO_EXTERNAL_PLUGIN_AVAILABLE = 256; // 对象不存在
static const mp_int32 RPC_ACTION_CONTINUE = 100;
static const mp_int32 RPC_ACTION_EXECUTIVE_BUSY = 101;
static const mp_int32 RPC_ACTION_EXECUTIVE_INTERNAL_ERROR = 200;
static const mp_int32 RPC_ACTION_CHANGE_STATUS_INTERNAL_ERROR = 300;
static const mp_int32 TASK_NOT_SUBCRILE = 10000;
static const mp_int32 TASK_NOT_RUN_IN_LOCAL_NODE = 10001;
// **********LOG LABEL*****************
static const mp_string ORACLE_POST_SCRIPT_FAIL_LABEL = "dme_databases_post_script_failed_label";
// END***********************脚本错误码，用于脚本错误返回，范围0-255************************//
// 脚本错误碼到真实错误码转换处理
class ErrorCode {
public:
    static ErrorCode& GetInstance()
    {
        static ErrorCode errorCode;
        return errorCode;
    }

    mp_int32 GetErrorCode(mp_int32 iRet)
    {
        std::map<mp_int32, mp_int32>::iterator it = m_mapErrorCode.find(iRet);
        return (it != m_mapErrorCode.end()) ? it->second : MP_FAILED;
    }

    mp_int32 GetSpecCommonID(mp_int32 iD)
    {
        std::set<mp_int32>::iterator it = m_setSpecCommonID.find(iD);
        return (it != m_setSpecCommonID.end()) ? *it : MP_FAILED;
    }

private:
    std::map<mp_int32, mp_int32> m_mapErrorCode;
    std::set<mp_int32> m_setSpecCommonID;
private:
    ErrorCode()
    {
        InitErrCode1();
        InitErrCode2();
        InitErrCode3();
        InitErrCode4();
        InitErrCode5();
        InitErrCode6();
        InitErrCode7();
        InitCommonID();
    }
    
    ~ErrorCode() { }

    mp_void InitErrCode1()
    {
        // 初始化错误码
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_EXEC_FAILED, ERROR_COMMON_SCRIPT_EXEC_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED, ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_COMMON_PATH_WRONG, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_COMMON_PARAM_WRONG, ERROR_COMMON_INVALID_PARAM));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG, ERROR_COMMON_DB_USERPWD_WRONG));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_COMMON_INSTANCE_NOSTART, ERROR_COMMON_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG, ERROR_COMMON_DB_INSUFFICIENT_PERMISSION));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE, ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB_FILE_NOT_EXIST, ERROR_COMMON_DB_FILE_NOT_EXIST));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED, ERROR_DEVICE_FILESYS_MOUNT_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED));

        // DB2错误码
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED, ERROR_DB2_SUSPEND_IO_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_RESUME_IO_FAILED, ERROR_DB2_RESUME_IO_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT, ERROR_DB2_SUSPEND_IO_TIMEOUT));
    }

    mp_void InitErrCode2()
    {
        // oracle错误码
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG, ERROR_ORACLE_ASM_DBUSERPWD_WRONG));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG, ERROR_ORACLE_ASM_INSUFFICIENT_WRONG));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART, ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE, ERROR_ORACLE_NOARCHIVE_MODE));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING, ERROR_ORACLE_OVER_ARCHIVE_USING));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT, ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT, ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK, ERROR_ORACLE_OVER_MAX_LINK));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP, ERROR_ORACLE_IN_BACKUP));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP, ERROR_ORACLE_NOT_IN_BACKUP));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING, ERROR_ORACLE_DB_ALREADYRUNNING));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT, ERROR_ORACLE_DB_ALREADYMOUNT));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN, ERROR_ORACLE_DB_ALREADYOPEN));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR, ERROR_ORACLE_ARCHIVE_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED, ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED, ERROR_ORACLE_END_HOT_BACKUP_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT, ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED, ERROR_ORACLE_TRUNCATE_LOG_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER, ERROR_ORACLE_TNS_PROTOCOL_ADAPTER));
    }

    mp_void InitErrCode3()
    {
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED, ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART, ERROR_CLUSTER_SERVICE_NOSTART));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER, ERROR_CLUSTER_DB_NOT_INCLUSTER));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL, ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED, ERROR_SQLSERVER_GET_DB_STATUS_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE, ERROR_SQLSERVER_DB_STATUS_OFFLINE));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST, ERROR_SQLSERVER_INSTANCE_NOT_EXIST));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST, ERROR_SQLSERVER_DB_NOT_EXIST));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED, ERROR_SQLSERVER_START_INSTANCE_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE, ERROR_CLUSTER_NOT_ACTIVE_NODE));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_APP_FAILED, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_MOUNTPATH, ERROR_MOUNTPATH));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_VERSION_DISMATCH, ERROR_ORACLE_VERSION_DISMATCH));
    }

    mp_void InitErrCode4()
    {
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_LINUX_V2C_MOUNT_FAILED, ERROR_LINUX_V2C_MOUNT_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_CHECK_FSTAB_FAILED, ERROR_LINUX_V2C_CHECK_FSTAB_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_VERSION_UNSUPPORTED, ERROR_LINUX_V2C_VERSION_UNSUPPORTED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_GRUBFILE_NOT_FOUND, ERROR_LINUX_V2C_GRUBFILE_NOT_FOUND));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_INITRD_NOT_FOUND, ERROR_LINUX_V2C_INITRD_NOT_FOUND));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_MODIFY_GRUB_FAILED, ERROR_LINUX_V2C_MODIFY_GRUB_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_DELETE_OLD_DRIVER_FAILED, ERROR_LINUX_V2C_DELETE_OLD_DRIVER_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_DELETE_VMWARE_TOOL_FAILED, ERROR_LINUX_V2C_DELETE_VMWARE_TOOL_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_ADD_KERNEL_PARAM_FAILED, ERROR_LINUX_V2C_ADD_KERNEL_PARAM_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_MODIFY_MKINITRD_CONF_FAILED, ERROR_LINUX_V2C_MODIFY_MKINITRD_CONF_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_LINUX_V2C_REMAKE_INITRD_FAILED, ERROR_LINUX_V2C_REMAKE_INITRD_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_WINDOWS_V2C_RESCAN_DISK_FAILED, ERROR_WINDOWS_V2C_RESCAN_DISK_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE, ERROR_WINDOWS_V2C_DISK_LETTER_NOT_IN_RANGE));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_WINDOWS_V2C_REGLOAD_FAILED, ERROR_WINDOWS_V2C_REGLOAD_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_WINDOWS_V2C_VERSION_UNSUPPORTED, ERROR_WINDOWS_V2C_VERSION_UNSUPPORTED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_WINDOWS_V2C_INSTALL_DRIVER_FAILED, ERROR_WINDOWS_V2C_INSTALL_DRIVER_FAILED));
    }

    mp_void InitErrCode5()
    {
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED, ERROR_EXCHANGE_REMOVE_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED, ERROR_EXCHANGE_SOFTRECVERY_FAILED));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED, ERROR_EXCHANGE_MOUNT_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL, ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                INTER_ERROR_SRCIPT_FILE_NOT_EXIST, ERROR_COMMON_SCRIPT_FILE_NOT_EXIST));
        m_mapErrorCode.emplace(
            std::map<mp_int32, mp_int32>::value_type(
                ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED, ERROR_CLUSTER_QUERY_FAILED));

        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_NOT_INSTALLED, ERROR_SCRIPT_ORACLE_NOT_INSTALLED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_INST_NOT_CDB, ERROR_ORACLE_INSTANCE_NOT_CDB));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT, ERROR_SQLSERVER_DB_NOT_EXIST));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_START_PDB_FAILED, ERROR_SQLSERVER_START_DB_FAILED));

        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_EXESQL_FAILED, ERROR_ORACLE_EXESQL_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_EXERMAN_FAILED, ERROR_ORACLE_EXERMAN_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_EXEASMCMD_FAILED, ERROR_ORACLE_EXEASMCMD_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_RECOVERPATH_NOT_EXIT, ERROR_ORACLE_RECOVERPATH_NOT_EXIT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_DB_NOT_MOUNT, ERROR_ORACLE_DB_NOT_OPEN));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_DB_NOT_OPEN, ERROR_ORACLE_DB_NOT_OPEN));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLE_DISKGROUP_NOT_EXIT, ERROR_ORACLE_DISKGROUP_NOT_EXIT));

        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_DISCONNECT_STORAGE_NETWORK, ERROR_DISCONNECT_STORAGE_NETWORK));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_MOUNTPATH_OCCUPIED, ERROR_MOUNTPATH_OCCUPIED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_DEL_ARCHIVELOG_FAILED, ERROR_DEL_ARCHIVELOG_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CHECK_ARCHIVELOG_FAILED, ERROR_CHECK_ARCHIVELOG_FAILED));
    }

    mp_void InitErrCode6()
    {
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLEHOME_LOST, ERROR_ORACLE_HOME_NOT_EXIT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ORACLEBASE_LOST, ERROR_ORACLE_BASE_NOT_EXIT));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_ASM_ASMDGNAME, ERROR_ORACLE_GET_ASMDG_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_START_RACDB, ERROR_ORACLE_START_RACDB));
    }

    mp_void InitErrCode7()
    {
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CREATE_LV, ERROR_DEVICE_LVM_CREATE_LV_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CREATE_PV, ERROR_DEVICE_LVM_CREATE_PV_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_CREATE_VG, ERROR_DEVICE_LVM_CREATE_VG_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SCAN_ADAPTER, ERROR_DEVICE_SCAN_ADAPTER_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_SCAN_DISK, ERROR_DEVICE_SCAN_DISK_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_VARYONVG, ERROR_DEVICE_LVM_ACTIVE_VG_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_QLA2XXX_STAUTS, ERR_QLA2XXX_STAUTS_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_TARGETCLI_STATUS, ERR_TARGETCLI_STATUS));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_MOUNT, ERROR_MOUNT_SRICPT_FAILED));
        m_mapErrorCode.emplace(std::map<mp_int32, mp_int32>::value_type(
            ERROR_SCRIPT_DISK_NOT_ENOUNGH, ERROR_AGENT_DISK_NOT_ENOUGH));
    }

    mp_void InitCommonID()
    {
        m_setSpecCommonID.emplace(SPEC_COMMID_QUERYORACLEINFO);
    }
};

/* ------------------------------------------------------------
Function Name: TRANSFORM_RETURN_CODE
Description  : 转换错误码，对于返回的-1进行错误码替换
Others       :------------------------------------------------------------- */
#define TRANSFORM_RETURN_CODE(iRet, RETURN_CODE) iRet = (iRet == MP_FAILED ? RETURN_CODE : iRet)

/* ------------------------------------------------------------
Function Name: MP_RETURN
Description  : 函数返回，对于Call调用返回-1的情况进行特殊替换
Others       :------------------------------------------------------------- */
#define MP_RETURN(Call, RETURN_CODE) do                        \
    {                                                          \
        mp_int32 iFuncRet = Call;                              \
        return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet; \
    } while (0)

/* ------------------------------------------------------------
Function Name: CHECK_MP_RETURN
Description  : 函数在失败情况下返回，对于Call调用返回-1的情况进行特殊替换
Others       :------------------------------------------------------------- */
#define CHECK_MP_RETURN(Call, RETURN_CODE) do                      \
    {                                                              \
        mp_int32 iFuncRet = Call;                                  \
        if (iFuncRet != MP_SUCCESS)                                \
            return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet; \
    } while (0)

#endif  // _AGENT_ERROR_CODE_H_
