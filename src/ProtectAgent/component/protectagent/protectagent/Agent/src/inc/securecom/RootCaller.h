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
#ifndef __ROOT_CALLER_H__
#define __ROOT_CALLER_H__

#include <map>
#include <vector>
#include <functional>
#include "common/CMpThread.h"
#include "common/Types.h"
#include "common/Pipe.h"

typedef enum ROOT_COMMAND_E {
    ROOT_COMMAND_SCRIPT_BEGIN = 0,  // ===============================================================
    ROOT_COMMAND_SCRIPT_QUERYORACLEINFO,         //
    ROOT_COMMAND_SCRIPT_QUERYORACLEPDBINFO,      // query PDB list
    ROOT_COMMAND_SCRIPT_STARTORACLEPDB,          // start PDB
    ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO,      //
    ROOT_COMMAND_SCRIPT_QUERYORACLECLUSTERINFO,  // oracleclusterinfo.sh
    ROOT_COMMAND_SCRIPT_ORACLERESOURCEGROUP,     // oracleresourcegroup.sh
    ROOT_COMMAND_SCRIPT_ORACLECHECKCDB,          // oraclecheckcdb.sh
    ROOT_COMMAND_SCRIPT_ORACLE_QUERYROLE,        // oraclequeryrole.sh
    ROOT_COMMAND_SCRIPT_ORACLE_QUERYTABLESPACE,  // oraclequerytablespace.sh
    ROOT_COMMAND_SCRIPT_ORACLE_ASM,              // oracleasm.sh

    ROOT_COMMAND_SCRIPT_TESTORACLE,              //
    ROOT_COMMAND_SCRIPT_CHECKARCHIVETHRESHOLD,   //
    ROOT_COMMAND_SCRIPT_STORAGEINFO,  // 获取oracle的存储信息，包括数据文件容量、日志文件容量、FS的VG、ASM的DG信息等
    ROOT_COMMAND_SCRIPT_QUERYBACKUPLEVEL,               // 查询备份类型
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPSTATUS,      // 获取oracle备份状态和进度
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPDATA,        // oracle native backup data
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPLOG,         // oracle native backup archive log
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_CHECKDBSTASTUS,    // before restore database, check database status, 0 if close, 1
                                                        // if open, 2 if exists
    ROOT_COMMAND_SCRIPT_ORACLENATIVERESTORE,            // Oracle native restore by rman
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_LIVEMOUNT,         // oracle native live mount
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_CLIVEMOUNT,        // cancel oracle native live mount
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_INSTRESTORE,       // oracle native instance restore
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_MOVEDBF,           // oracle native move dbf online
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_MEDIA_UMOUNT,      // oracle native media umount
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_EXPIRE_COPY,       // oracle native expire copy data
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_DISMOUNT_MEDIUM,   // oracle native dismount backup or restore medium
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_STOPRMANTASK,      // Stop Rman task
    ROOT_COMMAND_SCRIPT_ORACLENATIVE_PREPARE_NASMEDIA,  //
    ROOT_COMMAND_SCRIPT_QUERYORACLE_CLUSTERINFO,
    ROOT_COMMAND_SCRIPT_BACKUP_PREPAREFS,       // prepare FS backup media
    ROOT_COMMAND_SCRIPT_PREPARE_NASMEDIA,       // prepare nas media
    ROOT_COMMAND_SCRIPT_PREPARE_DATATURBOMEDIA, // prepare dataturbo media
    ROOT_COMMAND_SCRIPT_UMOUNT_NASMEDIA,        // umount nas media
    ROOT_COMMAND_SCRIPT_INIT,                   // initiator.sh
    ROOT_COMMAND_SCRIPT_LINK_ISCISITARGET,      // linkiscsitarget.sh
    ROOT_COMMAND_SCRIPT_GETHOSTOS,              // gethostos.sh
    ROOT_COMMAND_THIRDPARTY,                    // 第三方脚本
    ROOT_COMMAND_SCRIPT_USER_DEFINED_USER_DO,   // 用户指定的并且要以脚本用户执行的脚本
    ROOT_COMMAND_SCRIPT_PACKAGELOG,             // 打包日志脚本
    ROOT_COMMAND_SCRIPT_SCANDISK,               // 扫盘脚本
    ROOT_COMMAND_SCRIPT_UPGRADECALLER,          // 执行call agent升级的脚本
    ROOT_COMMAND_SCRIPT_PUSHUPDATECERT,      // 执行call agent升级证书的脚本
    ROOT_COMMAND_SCRIPT_CHECKBEFOREUPGRADE,     // 升级前的资源检查
    ROOT_COMMAND_SCRIPT_PREPAREFORUPGRADE,      // 升级前的准备
    ROOT_COMMAND_SCRIPT_PREPAREFORMODIFY,       // 修改前的准备
    ROOT_COMMAND_SCRIPT_MODIFYCALLER,           // 执行call agent修改的脚本
    ROOT_COMMAND_SCRIPT_CHECKBEFOREMODIFY,      // 修改前的资源检查
    ROOT_COMMAND_SCRIPT_FREEZEAPP,
    ROOT_COMMAND_SCRIPT_THAWAPP,
    ROOT_COMMAND_SCRIPT_ADD_FRIEWALL,
    ROOT_COMMAND_USER_DEFINED,     // 执行用户完全指定的脚本
    ROOT_COMMAND_SCRIPT_MOUNT_NAS_FILESYS,   // 新框架挂载NAS脚本
    ROOT_COMMAND_SCRIPT_MOUNT_DATATURBO_FILESYS, // 挂载Dataturbo脚本
    ROOT_COMMAND_SCRIPT_MOUNT_SANCLIENT_FILESYS,  // 挂载载SanClient fileio脚本
    ROOT_COMMAND_CHECK_AND_CREATE_DATATURBO_LINK,
    ROOT_COMMAND_SCRIPT_UMOUNT_NAS_FILESYS,  // 新框架卸载NAS脚本
    ROOT_COMMAND_SCRIPT_CLEAR_MOUNT_POINT,   // 定时清理挂载点脚本
    ROOT_COMMAND_SCRIPT_SET_CGROUP,   // 设置cgroup参数，限制插件cpu，内存
    ROOT_COMMAND_SANCLIENT_ENVCHECK,  // SANCLIENT主机检查环境
    ROOT_COMMAND_SANCLIENT_ACTION, // 创建lun的操作
    ROOT_COMMAND_SANCLIENT_ACTION_ISCSI, // 通过iscsi协议创建lun的操作
    ROOT_COMMAND_SANCLIENT_COPY_LOG_META, // 拷贝日志仓meta文件的操作
    ROOT_COMMAND_SANCLIENT_CLEAR, // 删除lun等操作
    ROOT_COMMAND_SCRIPT_VMFS_CHECK_TOOL,     // VMFS工具查询
    ROOT_COMMAND_SCRIPT_VMFS_MOUNT,     // VMFS文件系统挂载
    ROOT_COMMAND_SCRIPT_VMFS_UMOUNT,    // VMFS文件系统解除挂载
    ROOT_COMMAND_SCRIPT_END,       // ===============================================================
    ROOT_COMMAND_SYSTEM_BEGIN,     // ***************************************************************
    ROOT_COMMAND_FSCK,             // fsck
    ROOT_COMMAND_FSTYP,            // fstyp
    ROOT_COMMAND_CAT,              // cat
    ROOT_COMMAND_LS,               // ls -l
    ROOT_COMMAND_MOUNT,            // mount
    ROOT_COMMAND_GETCONF,          // getconf
    ROOT_COMMAND_IOSCANFNC,        // ioscan -fnC disk
    ROOT_COMMAND_CFGMGR,           // cfgmgr -v
    ROOT_COMMAND_CFGADM,           // cfgadm
    ROOT_COMMAND_DEVFSADM,      // devfsadm
    ROOT_COMMAND_RM,            // rm
    ROOT_COMMAND_MOUNT_NOECHO,  // mount command, mount exec failed, return error, add by wangguitao 2017-11-29
    ROOT_COMMAND_KILL,          // kill process
    ROOT_COMMAND_DU,          // du
    ROOT_COMMAND_CHMOD_OTHER_READ,
    ROOT_COMMAND_SYSTEM_END,
    ROOT_COMMAND_80PAGE,            // 查询80页信息
    ROOT_COMMAND_83PAGE,            // 查询83页信息
    ROOT_COMMAND_00PAGE,            // 查询00页信息
    ROOT_COMMAND_C8PAGE,            // 查询C8页信息
    ROOT_COMMAND_CAPACITY,          // 查询磁盘容量
    ROOT_COMMAND_VENDORANDPRODUCT,  // 查询阵列信息
    ROOT_COMMAND_WRITE_CERT_CN,     // 写入证书CN
    ROOT_COMMAND_SCAN_DIR_FILE,     // 即时挂载扫描文件系统目录和文件
    ROOT_COMMAND_WRITE_SCAN_RESULT,     // 扫描结果写入NFS文件系统
    // 如:传入/dev/sdb;/dev/sdc,获取
    // /dev/sdb,HUAWEI,S5500T,210235G6GR10D7000004,218,6200bc71001f37540769e56b000000da
    // /dev/sdc,HUAWEI,S5500T,210235G6GR10D7000004,218,6200bc71001f37540769e56b000000da
    ROOT_COMMAND_BATCH_GETLUN_INFO,  // 批量传入设备信息，获取LUN的基本信息
    ROOT_COMMAND_HOSTLUNID,          // 获取 通道的设备列表
    ROOT_COMMAND_SIGN_SCRIPT,
    ROOT_COMMAND_OM_GET_STATISTICS,    // 获取内核的状态信息
    ROOT_COMMAND_DATAPROCESS_START,    // 启动dataprocess进程
    ROOT_COMMAND_DPC_CONFIG_FLOW_CONTROL,          // 修改DPC计算节点上的流控配置
    ROOT_COMMAND_ADD_HOSTS,         // Add CN in system file /etc/hosts
    ROOT_COMMAND_BUTT,
    ROOT_COMMAND_SCRIPT_PID,          // 获取脚本运行的PID
} ROOT_COMMAND;

typedef mp_int32 (*pFunc)(mp_string);

class CRootCaller {
public:
    CRootCaller();
    ~CRootCaller();

    mp_int32 Exec(mp_int32 iCommandID, const mp_string& strParam, std::vector<mp_string> pvecResult[],
        mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL);
    mp_int32 ExecEx(mp_int32 iCommandID, const std::vector<mp_string>& vecParam, std::vector<mp_string> pvecResult[],
        mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL);
    mp_int32 ExecUserDefineScript(mp_int32 iCommandID, const std::string &scriptCmd);
    mp_int32 RemoveFile(const mp_string& fileName);
    mp_int32 ReadResultFile(mp_int32 iCommandID, const mp_string& strUniqueID, std::vector<mp_string> pvecResult[]);
};

#endif
