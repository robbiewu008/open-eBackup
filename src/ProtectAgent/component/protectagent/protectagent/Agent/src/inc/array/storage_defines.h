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
#ifndef AGENT_STORAGE_DEFINE_H
#define AGENT_STORAGE_DEFINE_H
#include "common/Types.h"
#include <vector>
#include <map>
#ifdef WIN32
#include <ntddscsi.h>
#include <setupapi.h>
const static mp_string GET_DEVICE_MEDIA_TYPE_BAT = "device_media_type.bat";
#endif

#include "common/Defines.h"

#ifdef WIN32
using FILE_HANDLE = HANDLE;
#else
typedef int FILE_HANDLE;
#endif

static const mp_uchar COUNT_LUNWWN = 0;
static const mp_uchar COUNT_LUNID  = 1;
static const mp_uchar C8_COUNT_LUNID = 0;

static const mp_uchar COUNT_VENDOR  = 0;
static const mp_uchar COUNT_PRODUCT = 1;
static const mp_uchar COUNT_VERSION = 2;

static const mp_uchar PAGE_83   = 0x83;
static const mp_uchar PAGE_80   = 0x80;
static const mp_uchar PAGE_00   = 0x00;
static const mp_uchar PAGE_C8   = 0xC8;
static const mp_uchar PAGE_CDB_0 = 0x00;
static const mp_uchar PAGE_CDB_1 = 0x01;

// scsi
static const mp_uchar SCSI_MAX_SENSE_LEN  = 64;   // sense的最大长度
static const mp_uint32 SCSI_DISK_DATA_LEN = 512;  // 磁盘数据长度
static const mp_uint32 DISK_MAX_PAGE_NUM  = 255;   // 磁盘支持的最大VPD Page数
static const mp_uchar DISK_NORECORD_ERROR = 4;   // 磁盘 无记录错误码
static const mp_uchar SC_INQ_PAGE_HEAD_LEN = 4;
#define VDS_HS_PAGE_BUF_LEN(buf) \
    (((buf)[SC_INQ_PAGE_HEAD_LEN - 2] << 8) + (buf)[SC_INQ_PAGE_HEAD_LEN - 1] + SC_INQ_PAGE_HEAD_LEN)
static const mp_uchar C8_PAGE_WITH_LUN_ID_LEN = 196;

// CDB长度
static const mp_uchar CDB6GENERIC_LENGTH  = 6;    // 6 字节
static const mp_uchar CDB10GENERIC_LENGTH = 10;  // 10 字节
static const mp_uchar CDB16GENERIC_LENGTH = 16;  // 16 字节

static const mp_uchar SCSIOP_READ_CAPACITY = 0x25;  // READ_CAPACITY命令操作码
static const mp_uchar SCSIOP_INQUIRY       = 0x12;        // CBD查询码

static const mp_uint32 NAME_PATH_LEN = 512;    // 磁盘名称长度
static const mp_uint32 DISK_PATH_MAX = 260;    // 磁盘路径最大值
static const mp_uchar MAX_VENDOR_LEN = 64;    // 厂商长度
static const mp_uint32 MAX_PRODUCT_LEN = 256;  // 型号长度
static const mp_uchar MAX_VVERSION_LEN = 4;   // 阵列V版本信息
static const mp_uchar MAX_SN_LEN  = 30;        // 序列号长度
static const mp_uchar MAX_WWN_LEN = 64;       // WWN
static const mp_uchar MAX_LUNID_LEN = 11;     // LUN ID
static const mp_uchar MAX_SN_HW   = 21;         // 序列号长度
static const mp_uchar MAX_PAGETCODE_LEN = 3;

static const mp_string ARRAY_VENDER_HUAWEI = "HUAWEI";             // 华 为 阵 列 VENDERID
static const mp_string VENDOR_ULTRAPATH_HUAWEI = "up";          // 华为多路径
static const mp_string ARRAY_VENDER_HUASY  = "HUASY";            // 华赛 阵 列 VENDERID
static const mp_string ARRAY_VENDER_FUSION_STORAGE = "Huawei";  // FusionStorage存储VENDERID
static const mp_string PRODUCT_VBS = "VBS fileIO";              // FusionStorage产品名称

static const mp_uchar HOST_PATH_NAME = 100;
static const mp_uint32 FILESYS_NAME_LEN = 256;      // 文件系统名称长度
static const mp_uchar FILESYS_TYPE_LEN  = 20;       // 文件系统类型长度
static const mp_uchar FILESYS_FATHER_LEN = 64;     // 文件系统所属磁盘名长度
static const mp_uint32 FILESYS_MOUNT_LEN = 256;     // 文件系统挂载点
static const mp_uint32 CAP_DATA_CONVERSION = 1024;  // 容量单位转换
static const mp_uchar MAX_DISK_EXTENS    = 10;

static const mp_uchar VALUE_LEN = 64;

// linux
static const mp_uint32 DISK_BYTE_OF_SECTOR = 512;  // io命令返回的缓存大小
static const mp_uint32 DATA_LEN_256 = 256;
static const mp_uint32 SCSI_CMD_TIMEOUT_VAL = 60;
static const mp_uint32 SCSI_CMD_TIMEOUT_UNIT = 1000;
static const mp_uint32 SCSI_CMD_TIMEOUT_LINUX = (SCSI_CMD_TIMEOUT_VAL * SCSI_CMD_TIMEOUT_UNIT);
static const mp_uint32 EXE_CMD_SCR_LEN = 256;  // LINUX/AIX命令长度
static const mp_uint32 LINE_DATA_LEN   = 256;    // 临时文件一行的长度
static const mp_uint32 FILE_ROW_COUNT  = 255;
static const mp_uint32 MAX_NAME_LEN    = 256;
static const mp_uint32 LINUX_BLOCK     = 1024;  // LINUX文件块大小

// AIX
static const mp_uchar BUFFER_LEN_36    = 36;
static const mp_uchar SCSI_CMD_TIMEOUT_AIX = 60;
static const mp_uchar MAX_STATUS_LEN   = 20;      // 磁盘状态长度
static const mp_string DISK_STATE_ACTIVE = "0";  // 激活的
static const mp_string DISK_STATE_CLOSE  = "1";   // 未激活的
static const mp_string AIX_DISK_ACTIVE   = "Available";
static const mp_uint32 BUFFER_LEN_255    = 255;
static const mp_uint32 AIX_BLOCK_SIZE    = 1024;

// HP
// 宏定义
static const mp_uint32 HP_BLOCK_SIZE     = 1024;  // HP文件块大小
#ifdef HP_UX_IA
#define SIOC_IO _IOWR('S', 22, struct sctl_io)  // IO控制码
static const mp_uint32 B_READ            = 0x00000001; // 读I/O标志
#endif
static const mp_uint32 SCSI_CMD_TIMEOUT_HP = (SCSI_CMD_TIMEOUT_VAL * SCSI_CMD_TIMEOUT_UNIT);  // IO命令超时时间
static const mp_uint32 DISK_EXECMD_SCR_LEN = 256;
static const mp_uint32 DISK_CMD_DATA_LEN   = 256;

// solaris
static const mp_uchar SCSI_CMD_TIMEOUT_SOLARIS = 60;  // IO命令超时时间
static const mp_uint32 BLOCK_SIZE_SOLARIS  = 1024;

static const mp_uchar CMD_DEV_BLOCKS       = 1;

static const mp_uchar MAX_DISK_NUM         = 128; // 主机侧的硬盘名称
static const mp_uchar MAX_LUN_ID           = 4;         // LUN ID最大长度
static const mp_uchar MAX_LUN_WWN_LEN      = 16;   // LUN WWN最大长度
static const mp_uchar FILE_SYSTEM_TYPE     = 20;

static const mp_uint32 UNIT_KB             = 1024;
static const mp_uint32 DEF_RLUNS_BUFF_VAL  = 32;
static const mp_uint32 DEF_RLUNS_BUFF_LEN  = (UNIT_KB * DEF_RLUNS_BUFF_VAL);
static const mp_uint32 MAX_RLUNS_BUFF_VAL  = 64;
static const mp_uint32 MAX_RLUNS_BUFF_LEN  = (UNIT_KB * MAX_RLUNS_BUFF_VAL);
static const mp_uint32 DEF_COM_TIMEOUT     = 30000;

#ifdef WIN32
// 宽字节字符串结构定义
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// 对象属性定义
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    UNICODE_STRING* ObjectName;
    ULONG Attributes;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

// 返回值或状态类型定义
static const mp_uint32 OBJ_CASE_INSENSITIVE = 0x00000040L;
static const mp_uint32 DIRECTORY_QUERY      = (0x0001);
static const mp_uint32 STATUS_SUCCESS       = ((ULONG)0x00000000L);  // ntsubauth

#define InitObjectAttributes(p, n, a, r, s) do { \
        (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
        (p)->RootDirectory = r;                  \
        (p)->Attributes = a;                     \
        (p)->ObjectName = n;                     \
        (p)->SecurityDescriptor = s;             \
        (p)->SecurityQualityOfService = NULL;    \
    } while (0)

#define NT_SUCCESS(Status) ((ULONG)(Status) >= 0)
static const mp_uint32 STATUS_INSUFFICIENT_RESOURCES = ((ULONG)0xC000009AL);  // ntsubauth

// 字符串初始化
typedef VOID(WINAPI* RTLINITUNICODESTRING)(PUNICODE_STRING, PCWSTR);
// 打开对象
typedef ULONG(WINAPI* ZWOPENDIRECTORYOBJECT)(
    OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
// 打开符号链接对象
typedef ULONG(WINAPI* ZWOPENSYMBOLICKLINKOBJECT)(
    OUT PHANDLE SymbolicLinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
// 查询符号链接对象
typedef ULONG(WINAPI* ZWQUERYSYMBOLICKLINKOBJECT)(
    IN HANDLE SymbolicLinkHandle, IN OUT PUNICODE_STRING TargetName, OUT PULONG ReturnLength OPTIONAL);
// 关闭已经打开的对象
typedef ULONG(WINAPI* ZWCLOSE)(IN HANDLE Handle);

typedef struct tag_scsi_pass_through_with_buff {
    SCSI_PASS_THROUGH pt;
    mp_uint64 ulPadding;
    mp_uchar aucSense[SCSI_MAX_SENSE_LEN];
    mp_uchar aucData[SCSI_DISK_DATA_LEN];
} scsi_pass_through_with_buff_t;

// 硬盘设备信息-win
typedef struct tag_win_dev_info {
    mp_int32 iDiskNum;                        // 磁盘序号
    mp_int32 iLunId;                          // LUNID
    mp_int32 iScsiId;                         // SCSIID
    mp_int32 iPortId;                         // 端口ID
    mp_int32 iPathId;                         // 路径ID
    mp_uchar aucPageCode[DISK_MAX_PAGE_NUM];  // supported VPD page code
} win_dev_info_t;

typedef struct tag_disk_info {
    mp_string strLUNID;    // LUN ID
    mp_string strArraySN;  // LUN所在阵列的序列号
    mp_string strLUNWWN;   // LUN的WWN
    mp_int32 iDiskNum;     // LUN在主机上的硬盘名称
} disk_info;

typedef struct tag_sub_area_Info {
    mp_int32 iDiskNum;                       // 磁盘序号
    mp_uint64 llOffset;                      // 分区偏移量
    mp_uint64 ullTotalCapacity;              // 分区长度
    mp_char acDriveLetter[HOST_PATH_NAME];   // 驱动器号（盘符）
    mp_char acVolName[DISK_PATH_MAX];        // 卷名
    mp_char acVolLabel[DISK_PATH_MAX];       // 卷标
    mp_char acFileSystem[FILESYS_TYPE_LEN];  // 文件系统类型
    mp_char acDeviceName[DISK_PATH_MAX];
} sub_area_Info_t;
#else
typedef struct tag_luninfo {
    mp_string strDeviceName;  // Device name
    mp_string strVendor;      // vendor
    mp_string strProduct;     // product
    mp_string strArraySN;     // LUN所在阵列的序列号
    mp_string strLUNID;       // LUN ID
    mp_string strLUNWWN;      // LUN的WWN
} luninfo_t;

#endif

typedef enum media_type {
    MEDIA_TYPE_UNSPECIFIED = 0,
    MEDIA_TYPE_HDD = 3,
    MEDIA_TYPE_SSD = 4,
    MEDIA_TYPE_SCM = 5
}media_type_t;


#pragma pack(1)
typedef struct tagISSP_SCSI_SENSE_HDR_S {  // See SPC-3 section 4.5
    mp_uchar ucResponseCode;               // permit: 0x0, 0x70, 0x71, 0x72, 0x73
    mp_uchar ucSenseKey;
    mp_uchar ucAsc;
    mp_uchar ucAscq;
    mp_uchar ucByte4;
    mp_uchar ucByte5;
    mp_uchar ucByte6;
    mp_uchar ucAdditionalLength;  // always 0 for fixed sense format
} ISSP_SCSI_SENSE_HDR_S;
#pragma pack()

#define CHECK_CLOSE_FD(Call) do                                                                        \
    {                                                                                                  \
        mp_int32 iCheckNotOkRet = Call;                                                                \
        if (EOK != iCheckNotOkRet) {                                                                   \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet); \
            close(iFd);                                                                                \
            return MP_FAILED;                                                                          \
        }                                                                                              \
    } while (0)

#endif

