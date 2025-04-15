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
#ifndef C_DPP_MESSAGE_H
#define C_DPP_MESSAGE_H

#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"
#include "common/Types.h"
#include "common/CMpTime.h"
#include "message/Message.h"
#include "message/tcp/CSocket.h"
#include "message/tcp/MessageImpl.h"

// max message size 1M
static const mp_uint32 MAX_MESSAGE_SIZE = 1024 * 1024 * 10;
static const mp_uint32 MAX_VMFS_MOUNT_NUM = 128;

static const mp_string MANAGECMD_KEY_CMDNO = "cmd";
static const mp_string MANAGECMD_KEY_BODY = "body";
static const mp_string MANAGECMD_KEY_ERRORCODE = "errorcode";
static const mp_string MANAGECMD_KEY_ERRORDETAIL = "errordetail";
static const mp_string MANAGECMD_KEY_LOGIN_USER = "username";
static const mp_string MANAGECMD_KEY_LOGIN_PWD = "password";
static const mp_string MANAGECMD_KEY_APPIP = "AppIP";
static const mp_string MANAGECMD_KEY_APPPORT = "AppPort";
static const mp_string MANAGECMD_KEY_TASKID = "taskId";
static const mp_string MANAGECMD_KEY_PARENT_TASKID = "parentTaskId";

// oracle native backup
static const mp_string PARAM_KEY_DBNAME = "dbName";
static const mp_string PARAM_KEY_INSTNAME = "instName";
static const mp_string PARAM_KEY_DBUSER = "dbUser";
static const mp_string PARAM_KEY_DBPWD = "dbPwd";
static const mp_string PARAM_KEY_DBTYPE = "dbType";
static const mp_string PARAM_KEY_ASMINST = "ASMInstance";
static const mp_string PARAM_KEY_ASMUSER = "ASMUser";
static const mp_string PARAM_KEY_ASMPWD = "ASMPwd";
static const mp_string PARAM_KEY_DBSTATE = "state";
static const mp_string PARAM_KEY_DBVERSION = "version";

// task manager
static const mp_string PARAM_KEY_TASKID = "taskId";
static const mp_string PARAM_KEY_PARENT_TASKID = "parentTaskId";
static const mp_string PARAM_KEY_TASKSTATUS = "state";
static const mp_string PARAM_KEY_TASKDESC = "desc";
static const mp_string PARAM_KEY_TASKSPEED = "speed";
static const mp_string PARAM_KEY_TASKPROGRESS = "progress";
static const mp_string PARAM_KEY_BACKUP_SIZE = "backupsize";
static const mp_string PARAM_KEY_LOGDETAIL = "log_detail";
static const mp_string PARAM_KEY_LOGPARAMS = "log_detail_param";
static const mp_string PARAM_KEY_LOGDETAILINFO = "log_detail_info";
static const mp_string PARAM_KEY_TASKRESULT = "taskresult";
static const mp_string PARAM_KEY_DATA_TRANS_MODE = "transmissionMode";
static const mp_string PARAM_KEY_DATA_TRANS_SPEED = "trans_speed";
static const mp_string PARAM_KEY_LOGLABEL = "log_label";
static const mp_string PARAM_KEY_LOGLABELPARAM = "log_label_param";
static const mp_string PARAM_KEY_ZEROBLOCK = "zero_blocks";
static const mp_string PARAM_KEY_REDUCTION_BLOCKS = "reduction_blocks";

// host info
static const mp_string PARAM_KEY_HOSTIP = "hostIp";
static const mp_string PARAM_KEY_HOSTSN = "hostSn";
static const mp_string PARAM_KEY_HOSTNAME = "hostName";
static const mp_string PARAM_KEY_HOSTOS = "os";
static const mp_string PARAM_KEY_HOSTOSVER = "osVer";
static const mp_string PARAM_KEY_HOSTAGENTVER = "agentVer";
static const mp_string PARAM_KEY_HOSTBUILDNUM = "buildNum";
static const mp_string PARAM_KEY_HOSTNICS = "nics";

// vmware native backup
static const mp_string PARAM_KEY_VM_ID = "vmID";
static const mp_string PARAM_KEY_VM_NAME = "vmName";
static const mp_string PARAM_KEY_VM_REF = "vmRef";
static const mp_string PARAM_KEY_VM_INFO = "vmInfo";

static const mp_string PARAM_KEY_DISK_TRANSPORTMODE = "TransportMode";
static const mp_string PARAM_KEY_VM_SNAPSHOTREF = "snapshotRef";
static const mp_string PARAM_KEY_BACKUPDATALAYOUT = "BackupDataLayout";
static const mp_string PARAM_KEY_BACKUPSTORAGETYPE = "BackupStorageType";
static const mp_string PARAM_KEY_DOCRC = "DoCrc";
static const mp_string PARAM_KEY_SNAPTYPE = "SnapType";
static const mp_string PARAM_KEY_PROTECT_ENV = "ProtectEnv";
static const mp_string PARAM_KEY_PRODUCTMANAGER_PROTOCOL = "Protocol";
static const mp_string PARAM_KEY_PRODUCTMANAGER_IP = "IP";
static const mp_string PARAM_KEY_PRODUCTMANAGER_PORT = "Port";
static const mp_string PARAM_KEY_PRODUCTMANAGER_USERNAME = "UserName";
static const mp_string PARAM_KEY_PRODUCTMANAGER_PASSWORD = "Password";
static const mp_string PARAM_KEY_PRODUCTMANAGER_CERTS = "Certs";
static const mp_string PARAM_KEY_PRODUCTMANAGER_VERSION = "Version";
static const mp_string PARAM_KEY_PRODUCTMANAGER_THUMBPRINT = "thumbPrint";
static const mp_string PARAM_KEY_PRODUCTMANAGER_STATUS = "Status";
static const mp_string PARAM_KEY_VOLUME_WWN = "wwn";
static const mp_string PARAM_KEY_VOLUME_DISKID = "diskID";
static const mp_string PARAM_KEY_VOLUME_DISKTYPE = "diskType";
static const mp_string PARAM_KEY_VOLUME_DISKSIZE = "diskSize";
static const mp_string PARAM_KEY_VOLUME_DISKPATH = "diskPath";
static const mp_string PARAM_KEY_VOLUME_DISK_DATASTORE_WWN = "dsLunsnapWwn";
static const mp_string PARAM_KEY_VOLUME_DISK_RELATIVE_PATH = "diskRelativePath";
static const mp_string PARAM_KEY_VOLUME_DISK_MOUNT_PATH = "diskMountPath";
static const mp_string PARAM_KEY_VMFSIO_FLAG = "vmfsioFlag";
static const mp_string PARAM_KEY_VOLUME_ISSYSTEM = "isSystem";
static const mp_string PARAM_KEY_VOLUME_DIRTYRANGE = "dirtyRange";
static const mp_string PARAM_KEY_VOLUME_LIMITSPEED = "limitSpeed";
static const mp_string PARAM_KEY_VOLUME_BACKUPLEVEL = "backupLevel";
static const mp_string PARAM_KEY_VOLUME_DIRTYRANGE_START = "start";
static const mp_string PARAM_KEY_VOLUME_DIRTYRANGE_LENGTH = "length";
static const mp_string PARAM_KEY_SCSITARGET_IP = "targetIp";
static const mp_string PARAM_KEY_SCSITARGET_PORT = "targetPort";
static const mp_string PARAM_KEY_SCSITARGET_NORMALTYPE = "normalType";
static const mp_string PARAM_KEY_SCSITARGET_DISCOVERYTYPE = "discoveryType";
static const mp_string PARAM_KEY_SCSITARGET_CHAPNAME = "chapName";
static const mp_string PARAM_KEY_SCSITARGET_CHAPPWD = "chapPwd";
static const mp_string PARAM_KEY_AGENT_HOST_IP = "AgentIP";
static const mp_string PARAM_KEY_VOLUME_MEDIUMID = "mediumID";
static const mp_string PARAM_KEY_VOLUME_BACKUPED_DISKID = "backupedDiskID";
static const mp_string PARAM_KEY_DESCFILE_ATTRS = "descFileAttrs";
static const mp_string PARAM_KEY_DESCFILE_ATTRS_CYLINDER = "cylinder";
static const mp_string PARAM_KEY_DESCFILE_ATTRS_HEAD = "head";
static const mp_string PARAM_KEY_DESCFILE_ATTRS_SECTOR = "sector";
static const mp_string PARAM_KEY_VOLUME_EAGERLY_CRUB = "eagerlyCrub";
static const mp_string PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE = "isSupportSAN";
static const mp_string PARAM_KEY_VOLUME_EXPECTED_TRANSPORTMODE = "expectedTransportMode";
static const mp_string PARAM_KEY_HOSTAGENT_SYSTEM_VIRT = "systemVirt";
static const mp_string PARAM_KEY_VMFS_MOUNT_POINT = "mountPoint";
static const mp_string PARAM_KEY_VMFS_MOUNT_PATH = "mountPath";

static const mp_string PARAM_KEY_EXCLUDE_LIST = "InvalidDataReduction";
static const mp_string PARAM_KEY_EXCLUDE_DELETED_FILES = "DeletedData";
static const mp_string PARAM_KEY_EXCLUDE_TMP_FILES = "SysTmpData";
static const mp_string PARAM_KEY_EXCLUDE_SPECIFIED_OPTION = "Specified";
static const mp_string PARAM_KEY_EXCLUDE_SPECIFIED_LIST = "SpecifiedFiles";
// DataProcess
static const mp_string EXT_CMD_PROTECT_VOL_NAMAE = "volname";
static const mp_string EXT_CMD_PROTECT_VOL_ID = "volid";
static const mp_string EXT_CMD_PROTECT_VM_INFO = "vmInfo";
static const mp_string EXT_CMD_PROTECT_PRODUCTMANAGER_INFO = "ProductManager";
static const mp_string EXT_CMD_PROTECT_VM_UUID = "vmUuid";

// prepare media
static const mp_string PARAM_KEY_STPRAGE = "storage";
static const mp_string PARAM_KEY_TASKTYPE = "taskType";
static const mp_string PARAM_KEY_STORTYPE = "Type";
static const mp_string PARAM_KEY_STOR_IP = "storageIp";
static const mp_string PARAM_KEY_STOR_PORT = "storagePort";
static const mp_string PARAM_KEY_STORAGE_TYPE = "storageType";

// query host role in cluster
static const mp_string PARAM_KEY_CLUSTER_TYPE = "taskType";

// test backup or restore medium
static const mp_string PARAM_KEY_MEDIUMID = "mediumID";

// manage cmd error code
static const mp_uint32 DPP_ERRCODE_OK = 0;  // DPP response error code success

// communication role, include soruce and taget
typedef enum _MessageRole {
    ROLE_ADMINNODE = 0x01,
    ROLE_HOST_AGENT = 0x02,
    ROLE_EBK_APP = 0x03,
    ROLE_EBK_VMWARE = 0x04,
    ROLE_EBK_CDP = 0x05,
    ROLE_HOST_DATAPROCESS = 0x06,
    ROLE_DME_ARCHIVE = 0x07
} MESSAGE_ROLE;

typedef enum _MsgDataType {
    MSG_DATA_TYPE_MANAGE = 0xFFFF,

    // archive
    CMD_ARCHIVE_GET_FILE_DATA = 0x0702,
    CMD_ARCHIVE_GET_FILE_DATA_BIN_ACK = 0x0703,
    CMD_ARCHIVE_GET_FILE_DATA_JSON_ACK = 0x0704,

    CMD_ARCHIVE_PREPARE_RECOVERY = 0x0706,
    CMD_ARCHIVE_PREPARE_RECOVERY_ACK = 0x0707,

    CMD_ARCHIVE_QUERY_PREPARE_STATE = 0x0708,
    CMD_ARCHIVE_QUERY_PREPARE_STATE_ACK = 0x0709,

    CMD_ARCHIVE_GET_BACKUP_INFO = 0x070A,
    CMD_ARCHIVE_GET_BACKUP_INFO_ACK = 0x070B,

    CMD_ARCHIVE_GET_RECOVERY_OBJECT_LIST = 0x070C,
    CMD_ARCHIVE_GET_RECOVERY_OBJECT_LIST_ACK = 0x070D,

    CMD_ARCHIVE_GET_FILE_META_DATA = 0x070E,
    CMD_ARCHIVE_GET_FILE_META_DATA_ACK = 0x070F,

    CMD_ARCHIVE_GET_DIR_META_DATA = 0x0710,
    CMD_ARCHIVE_GET_DIR_META_DATA_ACK = 0x0711,

    CMD_ARCHIVE_END_RECOVERY = 0x0712,
    CMD_ARCHIVE_END_RECOVERY_ACK = 0x0713,

    CMD_ARCHIVE_ADD_WHITE_LIST = 0x0714,
    CMD_ARCHIVE_ADD_WHITE_LIST_ACK = 0x0715,

    CMD_ARCHIVE_SSH_CHECK = 0x0716,
    CMD_ARCHIVE_SSH_CHECK_ACK = 0x0717,
 } MsgDataType;

static const mp_uint32 MANAGE_ERRNO_INVALID = -1;

typedef enum _ManageCmdNo {
    // system interface
    MANAGE_CMD_NO_INVALID = 0,
    MANAGE_CMD_NO_LOGIN = 0x0001,
    MANAGE_CMD_NO_LOGIN_ACK = 0x0002,
    MANAGE_CMD_NO_LOGOFF = 0x0003,
    MANAGE_CMD_NO_HEARTBEATE = 0x0004,
    MANAGE_CMD_NO_HEARTBEATE_ACK = 0x0005,
    // host interface
    MANAGE_CMD_NO_HOST_REPORT = 0x0101,
    MANAGE_CMD_NO_HOST_REPORT_ACK = 0x0102,
    MANAGE_CMD_NO_HOST_GETINITIATOR = 0x0103,
    MANAGE_CMD_NO_HOST_GETINITIATOR_ACK = 0x0104,
    MANAGE_CMD_NO_HOST_GET_IPS = 0x0105,
    MANAGE_CMD_NO_HOST_GET_IPS_ACK = 0x0106,

    MANAGE_CMD_NO_HOST_SCAN_DISK = 0x0109,
    MANAGE_CMD_NO_HOST_SCAN_DISK_ACK = 0x010A,

    MANAGE_CMD_NO_HOST_LINK_ISCSI = 0x010B,
    MANAGE_CMD_NO_HOST_LINK_ISCSI_ACK = 0x010C,
    // task interface
    MANAGE_CMD_NO_TASK_REPORT_PROGRESS = 0x0201,
    MANAGE_CMD_NO_TASK_COMPLETED = 0x0202,
    // oracle interface
    MANAGE_CMD_NO_ORACLE_GETSTOR_INFO = 0x0303,
    MANAGE_CMD_NO_ORACLE_GETSTOR_INFO_ACK = 0x0304,
    MANAGE_CMD_NO_ORACLE_PREPARE_MEDIA = 0x0305,
    MANAGE_CMD_NO_ORACLE_PREPARE_MEDIA_ACK = 0x0306,
    MANAGE_CMD_NO_ORACLE_BACKUP_DATA = 0x0307,
    MANAGE_CMD_NO_ORACLE_BACKUP_DATA_ACK = 0x0308,
    MANAGE_CMD_NO_ORACLE_BACKUP_LOG = 0x0309,
    MANAGE_CMD_NO_ORACLE_BACKUP_LOG_ACK = 0x030A,
    MANAGE_CMD_NO_ORACLE_RESTORE = 0x030B,
    MANAGE_CMD_NO_ORACLE_RESTORE_ACK = 0x030C,
    MANAGE_CMD_NO_ORACLE_LIVEMOUNT = 0x030D,
    MANAGE_CMD_NO_ORACLE_LIVEMOUNT_ACK = 0x030E,
    MANAGE_CMD_NO_ORACLE_CLIVEMOUNT = 0x030F,
    MANAGE_CMD_NO_ORACLE_CLIVEMOUNT_ACK = 0x0310,
    MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE = 0x0311,
    MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE_ACK = 0x0312,
    MANAGE_CMD_NO_ORACLE_EXPIRE_COPY = 0x0313,
    MANAGE_CMD_NO_ORACLE_EXPIRE_COPY_ACK = 0x0314,
    MANAGE_CMD_NO_ORACLE_CHECK_AUTH = 0x0315,
    MANAGE_CMD_NO_ORACLE_CHECK_AUTH_ACK = 0x0316,
    MANAGE_CMD_NO_ORACLE_DISMOUNT_MEDIA = 0x0317,
    MANAGE_CMD_NO_ORACLE_DISMOUNT_MEDIA_ACK = 0x0318,
    MANAGE_CMD_NO_ORACLE_STOP_TASK = 0x0319,
    MANAGE_CMD_NO_ORACLE_STOP_TASK_ACK = 0x031A,
    MANAGE_CMD_NO_ORACLE_QUERY_HOST_ROLE = 0x031B,
    MANAGE_CMD_NO_ORACLE_QUERY_HOST_ROLE_ACK = 0x031C,
    MANAGE_CMD_NO_ORACLE_QUERY_BACKUPLEVEL = 0x031D,
    MANAGE_CMD_NO_ORACLE_QUERY_BACKUPLEVEL_ACK = 0x031E,

    // vmware native backup interfaces' commands
    MANAGE_CMD_NO_VMWARENATIVE_PREPARE_BACKUP = 0x0401,
    MANAGE_CMD_NO_VMWARENATIVE_PREPARE_BACKUP_ACK = 0x0402,

    MANAGE_CMD_NO_VMWARENATIVE_OPENDISK_BACKUP = 0x0468,
    MANAGE_CMD_NO_VMWARENATIVE_OPENDISK_BACKUP_ACK = 0x0469,

    MANAGE_CMD_NO_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE = 0x0403,
    MANAGE_CMD_NO_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE_ACK = 0x0404,

    MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP = 0x0408,
    MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP_ACK = 0x0409,

    MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS = 0x040A,
    MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS_ACK = 0x040B,

    MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP = 0x040C,
    MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP_ACK = 0x040D,

    MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK = 0x040E,
    MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK_ACK = 0x040F,

    MANAGE_CMD_NO_VMWARENATIVE_CANCEL_BACKUP_TASK = 0x0410,
    MANAGE_CMD_NO_VMWARENATIVE_CANCEL_BACKUP_TASK_ACK = 0x0411,

    MANAGE_CMD_NO_VMWARENATIVE_PREPARE_RECOVERY = 0x0412,
    MANAGE_CMD_NO_VMWARENATIVE_PREPARE_RECOVERY_ACK = 0x0413,

    MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY = 0x0414,
    MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY_ACK = 0x0415,

    MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS = 0x0416,
    MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS_ACK = 0x0417,

    MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY = 0x0418,
    MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY_ACK = 0x0419,

    MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK = 0x041A,
    MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK_ACK = 0x041B,

    MANAGE_CMD_NO_VMWARENATIVE_CANCEL_RECOVERY_TASK = 0x041C,
    MANAGE_CMD_NO_VMWARENATIVE_CANCEL_RECOVERY_TASK_ACK = 0x041D,

    MANAGE_CMD_NO_VMWARENATIVE_CONFIRM_PRODUCTENV_CONNECTION = 0x041E,
    MANAGE_CMD_NO_VMWARENATIVE_CONFIRM_PRODUCTENV_CONNECTION_ACK = 0x041F,

    MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_RESOURCES = 0x044E,
    MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_RESOURCES_ACK = 0x044F,

    MANAGE_CMD_NO_VMWARENATIVE_INIT_VDDKLIB = 0x0452,
    MANAGE_CMD_NO_VMWARENATIVE_INIT_VDDKLIB_ACK = 0x0453,

    MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_VDDKLIB = 0x0456,
    MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_VDDKLIB_ACK = 0x0457,

    MANAGE_CMD_NO_VMWARENATIVE_CHECK_VMFS_TOOL = 0x0470,
    MANAGE_CMD_NO_VMWARENATIVE_CHECK_VMFS_TOOL_ACK = 0x0471,

    MANAGE_CMD_NO_VMWARENATIVE_VMFS_MOUNT = 0x0472,
    MANAGE_CMD_NO_VMWARENATIVE_VMFS_MOUNT_ACK = 0x0473,

    MANAGE_CMD_NO_VMWARENATIVE_VMFS_UMOUNT = 0x0474,
    MANAGE_CMD_NO_VMWARENATIVE_VMFS_UMOUNT_ACK = 0x0475,

    // invalid data indentify
    MANAGE_CMD_NO_VMWARENATIVE_ALLDISK_AFS_BITMAP = 0x0476,
    MANAGE_CMD_NO_VMWARENATIVE_ALLDISK_AFS_BIITMAP_ACK = 0x0477,

    // Storage Layer Nas BackUp
    MANAGE_CMD_NO_VMWARENATIVE_SLNAS_MOUNT = 0x0480,
    MANAGE_CMD_NO_VMWARENATIVE_SLNAS_MOUNT_ACK = 0x0481,

    MANAGE_CMD_NO_VMWARENATIVE_SLNAS_UMOUNT = 0x0482,
    MANAGE_CMD_NO_VMWARENATIVE_SLNAS_UMOUNT_ACK = 0x0483,

    // Data Process
    EXT_CMD_PROTECT_VOL = 0x0500,
    EXT_CMD_ADD_VOL = 0x0501,
    EXT_CMD_DEL_VOL = 0x0502,
    EXT_CMD_MOD_VOL = 0x0503,
    EXT_CMD_VOL_READY = 0x0504,
    EXT_CMD_PAUSE = 0x0505,
    EXT_CMD_RESUME = 0x0506,
    EXT_CMD_CLOSE = 0x0507,
    EXT_CMD_PROTECT_PLACEHOLDER = 0x1111,

    // internal usage commands
    EXT_CMD_VMWARENATIVE_CHECK_PRODUCTENV_CONNECTION = 0x0430,
    EXT_CMD_VMWARENATIVE_CHECK_PRODUCTENV_CONNECTION_ACK = 0x0431,
    EXT_CMD_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE = 0x0432,
    EXT_CMD_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE_ACK = 0x0433,
    EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE = 0x0434,
    EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE_ACK = 0x0435,
    EXT_CMD_VMWARENATIVE_BACKUP_PREPARATION = 0x0436,
    EXT_CMD_VMWARENATIVE_BACKUP_PREPARATION_ACK = 0x0437,
    EXT_CMD_VMWARENATIVE_BACKUP_OPENDISK = 0x046A,
    EXT_CMD_VMWARENATIVE_BACKUP_OPENDISK_ACK = 0x046B,
    EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK = 0x0438,
    EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK = 0x0439,
    EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS = 0x043A,
    EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK = 0x043B,
    EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP = 0x043C,
    EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP_ACK = 0x043D,
    EXT_CMD_VMWARENATIVE_FINISH_VMBACKUP_TASK = 0x043E,
    EXT_CMD_VMWARENATIVE_FINISH_VMBACKUP_TASK_ACK = 0x043F,
    EXT_CMD_VMWARENATIVE_CANCEL_VMBACKUP_TASK = 0x0440,
    EXT_CMD_VMWARENATIVE_CANCEL_VMBACKUP_TASK_ACK = 0x0441,
    EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION = 0x0442,
    EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK = 0x0443,
    EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK = 0x0444,
    EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK = 0x0445,
    EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS = 0x0446,
    EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK = 0x0447,
    EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY = 0x0448,
    EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY_ACK = 0x0449,
    EXT_CMD_VMWARENATIVE_FINISH_VMRECOVERY_TASK = 0x044A,
    EXT_CMD_VMWARENATIVE_FINISH_VMRECOVERY_TASK_ACK = 0x044B,
    EXT_CMD_VMWARENATIVE_CANCEL_VMRECOVERY_TASK = 0x044C,
    EXT_CMD_VMWARENATIVE_CANCEL_VMRECOVERY_TASK_ACK = 0x044D,
    EXT_CMD_VMWARENATIVE_CLEANUP_RESOURCES_TASK = 0x044E,
    EXT_CMD_VMWARENATIVE_CLEANUP_RESOURCES_TASK_ACK = 0x044F,
    EXT_CMD_VMWARENATIVE_INIT_VDDKLIB = 0x0450,
    EXT_CMD_VMWARENATIVE_INIT_VDDKLIB_ACK = 0x0451,
    EXT_CMD_VMWARENATIVE_CLEANUP_VDDKLIB = 0x0454,
    EXT_CMD_VMWARENATIVE_CLEANUP_VDDKLIB_ACK = 0x0455,
    EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK = 0x047A,
    EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK = 0x047B,

    EXT_CMD_VMWARENATIVE_BACKUP_AFS = 0x0476,
    EXT_CMD_VMWARENATIVE_BACKUP_AFS_ACK = 0x0477,

    // DWS
    EXT_CMD_BSA_CREATEOBJECT = 0x0601,
    EXT_CMD_BSA_CREATEOBJECT_ACK = 0x0602,
    EXT_CMD_BSA_SENDRESULT = 0x0603,
    EXT_CMD_BSA_SENDRESULT_ACK = 0x0604,
    EXT_CMD_BSA_QUERYOBJECT = 0x0605,
    EXT_CMD_BSA_QUERYOBJECT_ACK = 0x0606,
    EXT_CMD_BSA_INIT = 0x0607,
    EXT_CMD_BSA_INIT_ACK = 0x0608,
    EXT_CMD_BSA_DELETEOBJECT = 0x0609,
    EXT_CMD_BSA_DELETEOBJECT_ACK = 0x0610,
    EXT_CMD_BSA_GET_IP_INFO = 0x0611,
    EXT_CMD_BSA_GET_IP_INFO_ACK = 0x0612,
} ManageCmdNo;

class CConnection;

// message不能保存connection，connection属于business client，可能被释放，只保存IP和port
class CDppMessage : public CBasicReqMsg, public CBasicRspMsg {
public:
    CDppMessage();
    explicit CDppMessage(const CDppMessage &msg);
    ~CDppMessage();
    mp_void InitMsgHead(mp_uint16 cmdNo, mp_uint16 flag, mp_uint64 seqNo);
    mp_int32 InitMsgBody();
    mp_int32 ReinitMsgBody();
    mp_void CloneMsg(CDppMessage& msg);

    mp_string GetIpAddr();
    mp_uint16 GetPort();
    mp_string GetIpAddrStr();

    mp_bool IsValidPrefix();
    mp_uint32 GetPrefix();
    mp_uint16 GetCmd();
    mp_uint16 GetFlag();
    mp_uint64 GetOrgSeqNo();
    mp_uint32 GetSize();
    mp_uint32 GetSize1();
    mp_uint32 GetSize2();

    mp_char* GetBuffer();
    mp_char* GetStart();

    mp_void SetMsgSrc(MESSAGE_ROLE role);
    mp_void SetMsgTgt(MESSAGE_ROLE role);
    mp_void SwapSrcTgt();
    MESSAGE_ROLE GetMsgSrc();
    MESSAGE_ROLE GetMsgTgt();
    mp_int32 SetMsgBody(Json::Value& msgBody);
    mp_void SetLinkInfo(const mp_string& uiIpAddr, const mp_uint16& uiPort);
    mp_void SetOrgSeqNo(mp_uint64 uiOrgSeqNo);

    mp_uint32 GetManageCmd();
    mp_int32 GetManageBody(Json::Value& dppBody);
    mp_int32 GetManageSessionId(mp_string& sessionId);
    mp_int32 GetManageError(mp_int64& errNo, mp_string& errDetail);
    mp_int32 SetManageSessionId(const mp_string& sessionId);

    mp_void UpdateTime();
    mp_time GetUpdateTime();
    mp_void ItemEendianSwap();
    mp_void DEndianSwaps(mp_uint16 *pData);
    mp_void DEndianSwapl(mp_uint32 *pData);
    mp_void DEndianSwapll(mp_uint64 *pData);
    mp_void EndianSwap(mp_char *pData, int startIndex, int length);
private:
    DppMessage dppMessage;
    mp_void DestroyMsgBody();
    mp_int32 AnalyzeManageMsg();

    // save manager cmd, after get manager cmd, save this value, next time do nothing
    mp_uint32 mCmd;
    Json::Value manageBody;
    mp_int64 mErrNo;
    mp_string mErrDetail;
    mp_time mLastUpTime;    // record last udpate time
};
#endif  // !defined(EA_C7849FDE_421E_43f5_A7EB_6D01673A8C72__INCLUDED_)
