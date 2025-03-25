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
#ifndef __AGENT_REST_INTERFACES_H__
#define __AGENT_REST_INTERFACES_H__

// URL Method
static const mp_string REST_URL_METHOD_GET = "GET";
static const mp_string REST_URL_METHOD_PUT = "PUT";
static const mp_string REST_URL_METHOD_POST = "POST";
static const mp_string REST_URL_METHOD_DELETE = "DELETE";

/***********************REST interface published*************************************/
// host
static const mp_string REST_HOST_QUERY_INITIATOR = "/host/initiators";
static const mp_string REST_HOST_CONNECT_DME = "/host/dmeserver/action/connect";
static const mp_string REST_HOST_QUERY_INFO = "/host";
static const mp_string REST_HOST_V1_QUERY_INFO = "/v1/host";
static const mp_string REST_HOST_V1_QUERY_APPLUGINS = "/v1/host/applugins";
static const mp_string REST_HOST_V1_QUERY_WWPNS = "/v1/host/wwpns";
static const mp_string REST_HOST_V2_QUERY_WWPNS = "/v2/host/wwpns";
static const mp_string REST_HOST_V1_QUERY_IQNS = "/v1/host/iqns";
static const mp_string REST_HOST_V1_SCAN_IQN = "/v1/host/iqns/scan";
static const mp_string REST_HOST_V1_DATATURBO_RESCAN = "/v1/host/dataturbo/rescan";
static const mp_string REST_HOST_QUERY_AGENT_INFO = "/host/action/version/check";
static const mp_string REST_HOST_PUSH_CERT_AGENT = "/v1/host/action/cert/push";
static const mp_string REST_HOST_UPDATE_CERT_AGENT = "/v1/host/action/cert/update";
static const mp_string REST_HOST_CLEAN_CERT_AGENT = "/v1/host/action/cert/delete-old-cert";
static const mp_string REST_HOST_ROLLBACK_CERT_AGENT = "/v1/host/action/cert/fallback";
static const mp_string REST_HOST_CHECK_CONNECT_AGENT = "/v1/host/action/cert/network/check";
static const mp_string REST_HOST_UPGRADE_AGENT = "/host/action/agent/upgrade";
static const mp_string REST_HOST_QUERY_UPGRADE_STATUS = "/host/action/check/status/upgrade";
static const mp_string REST_HOST_UPDATE_VCENTER_INFO = "/host/updatelinks";
static const mp_string REST_HOST_V1_NOTIFY_MANAGER_SERVER = "/v1/host/managerserver";
static const mp_string REST_HOST_LOG_COLLECT = "/v1/host/log/collect";
static const mp_string REST_HOST_LOG_COLLECT_STATUS = "/v1/host/log/collecting-status";
static const mp_string REST_HOST_LOG_EXPORT = "/v1/host/log/export";
static const mp_string REST_HOST_LOG_LEVEL = "/v1/host/log/level";
static const mp_string REST_HOST_LOG_CLEAN = "/v1/host/log/clean";
static const mp_string REST_HOST_V1_MODIFY_PLUGINS = "/v1/host/action/modify-plugin";
static const mp_string REST_HOST_QUERY_MODIFY_STATUS = "/host/action/check/status/modify";
static const mp_string REST_HOST_V1_COMPRESS_TOOL = "/v1/host/action/compresstool";

// oracle
static const mp_string REST_ORACLE_QUERY_DB_INFO = "/oracle/databases";
static const mp_string REST_ORACLE_START = "/oracle/databases/action/start";
static const mp_string REST_ORACLE_STOP = "/oracle/databases/action/stop";
static const mp_string REST_ORACLE_QUERY_ASM = "/oracle/asm";
static const mp_string REST_ORACLE_TEST = "/oracle/databases/action/testconnection";
static const mp_string REST_ORACLE_QUERYTABLESPACE = "/oracle/tablespace";
static const mp_string REST_ORACLE_QUERY_CLUSTERINFO = "/oracle/clusterinfo";

// appprotect
static const mp_string REST_APPPROTECT_RESOURCE_V1 = "/v1/tasks/resource";
static const mp_string REST_APPPROTECT_DETAIL_V1 = "/v1/tasks/detail";
static const mp_string REST_APPPROTECT_CHECK_V1 = "/v1/tasks/check";
static const mp_string REST_APPPROTECT_CLUSTER_V1 = "/v1/tasks/cluster";
static const mp_string REST_APPPROTECT_CONFIG_V1 = "/v1/tasks/config";
static const mp_string REST_APPPROTECT_DELIVER_JOB_STATUS_V1 = "/v1/tasks/task-status";
static const mp_string REST_APPPROTECT_DETAIL_V2 = "/v2/tasks/detail";
static const mp_string REST_APPPROTECT_ASYNCDETAIL_V2 = "/v2/tasks/asyncdetail";
static const mp_string REST_APPPROTECT_REMOVE_PROTECT_V1 = "/v1/tasks/remove-protect";
static const mp_string REST_APPPROTECT_FINALIZE_CLEAR = "/v1/tasks/finalizeclear";
static const mp_string REST_APPPROTECT_SANCLIENT_JOB = "/v1/tasks/[[:alnum:]]{8}-"
    "[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}([_A-Za-z0-9]*){0,1}/sanclient";
static const mp_string REST_APPPROTECT_WAKEUP_JOB = "/v1/tasks/[[:alnum:]]{8}-"
    "[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}([_A-Za-z0-9]*){0,1}/notify";
static const mp_string REST_APPPROTECT_ABORT_JOB = "/v1/tasks/[[:alnum:]]{8}-"
    "[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}([_A-Za-z0-9]*){0,1}/abort";
static const mp_string REST_APPPROTECT_SANCLIENT_JOB_V1 = "/v1/tasks/[[:alnum:]]{8}-"
    "[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}([_A-Za-z0-9]*){0,1}/queryluninfo";
static const mp_string REST_CLEAN_SANCLIENT_JOB = "/v1/tasks/[[:alnum:]]{8}-"
    "[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}([_A-Za-z0-9]*){0,1}/sanclientclean";
static const mp_string REST_APPPROTECT_GET_ESN = "/v1/tasks/get-esn";

// App
static const mp_string REST_APP_QUERY_DB_INFO = "/app/application";
/***********************REST interface not published*************************************/
// host
static const mp_string REST_HOST_QUERY_AGENT_VERSION = "/host/agent";
static const mp_string REST_HOST_QUERY_FUSION_STORAGE = "/host/fusionstorage";
static const mp_string REST_HOST_QUERY_DISKS = "/host/disks";
static const mp_string REST_HOST_SCAN_DISK = "/host/disks/action/scan";
static const mp_string REST_HOST_QUERY_PARTITIONS = "/host/disks/partitions";
static const mp_string REST_HOST_QUERY_TIMEZONE = "/host/timezone";

// ThirdParty
static const mp_string REST_HOST_THIRDPARTY_QUERY_FILE_INFO = "/host/thirdparty/files";
static const mp_string REST_HOST_THIRDPARTY_EXEC_FILE = "/host/thirdparty/action/excute";
static const mp_string REST_HOST_UPDATE_TRAP_SERVER = "/host/trapserver/update";
static const mp_string REST_HOST_REG_TRAP_SERVER = "/host/trapserver/register";
static const mp_string REST_HOST_UNREG_TRAP_SERVER = "/host/trapserver/unregister";
static const mp_string REST_HOST_VERIFY_SNMP = "/host/snmp/params/action/verify";
static const mp_string REST_HOST_ONLINE = "/host/disks/action/online";
static const mp_string REST_HOST_BATCH_ONLINE = "/host/disks/action/batchonline";
static const mp_string REST_HOST_FREEZE_SCRIPT = "/host/thirdparty/action/freeze";
static const mp_string REST_HOST_UNFREEZE_SCRIPT = "/host/thirdparty/action/unfreeze";
static const mp_string REST_HOST_QUERY_STATUS_SCRIPT = "/host/thirdparty/freezestate";

// Device
// FileSystem
static const mp_string REST_DEVICE_FILESYS_QUERY = "/device/filesystems";
static const mp_string REST_DEVICE_FILESYS_MOUNT = "/device/filesystems/action/mount";
static const mp_string REST_DEVICE_FILESYS_MOUTN_BATCH = "/device/filesystems/action/batchmount";
static const mp_string REST_DEVICE_FILESYS_UNMOUNT = "/device/filesystems/action/unmount";
static const mp_string REST_DEVICE_FILESYS_UMOUNT_BATCH = "/device/filesystems/action/batchunmount";
static const mp_string REST_DEVICE_FILESYS_FREEZE = "/device/filesystems/action/freeze";
static const mp_string REST_DEVICE_FILESYS_UNFREEZE = "/device/filesystems/action/unfreeze";
static const mp_string REST_DEVICE_FILESYS_FREEZESTATUS = "/device/filesystems/freezestate";
static const mp_string REST_DEVICE_DRIVELETTER_DELETE_BATCH = "/device/driveletter/action/batchdel";

// App
static const mp_string REST_APP_FREEZE = "/app/action/freeze";
static const mp_string REST_APP_UNFREEZE = "/app/action/unfreeze";
static const mp_string REST_APP_ENDBACKUP = "/app/action/endbackup";
static const mp_string REST_APP_TRUNCATE_LOG = "/app/action/truncatelog";
static const mp_string REST_APP_QUERY_DB_FREEZESTATE = "/app/freezestate";
static const mp_string REST_APP_UNFREEZEEX = "/app/action/unfreezeex";

// alarm
const static mp_string REST_ALARM = "/v1/internal/alarms/agent";
const static mp_string REST_ALARM_CLEAR = "/v1/internal/alarms/agent/action/clear";

// 错误消息体
static const mp_string REST_PARAM_ERROR_CODE = "errorCode";

// CodeDex误报，Password Management:Hardcoded Password
// 消息头key
static const mp_string HTTPPARAM_DBUSERNAME = "HTTP_DBUSERNAME";
static const mp_string HTTPPARAM_DBPASSWORD = "HTTP_DBPASSWORD";
static const mp_string HTTPPARAM_ASMSERNAME = "HTTP_ASMUSERNAME";
static const mp_string HTTPPARAM_ASMPASSWORD = "HTTP_ASMPASSWORD";
static const mp_string HTTPPARAM_SNMPAUTHPW = "HTTP_AUTHPASSWORD";
static const mp_string HTTPPARAM_SNMPENCRYPW = "HTTP_ENCRYPTPASSWORD";
static const mp_string UNKNOWN = "Unknown";
static const mp_string REMOTE_ADDR = "REMOTE_ADDR";
static const mp_string REQUEST_URI = "REQUEST_URI";
static const mp_string REQUEST_METHOD = "REQUEST_METHOD";
static const mp_string CONTENT_LENGTH = "CONTENT_LENGTH";
static const mp_string QUERY_STRING = "QUERY_STRING";
static const mp_string STATUS = "Status";
static const mp_string CONTENT_TYPE = "CONTENT_TYPE";
static const mp_string HTTP_CONTENT_TYPE = "Content-Type";
static const mp_string CACHE_CONTROL = "Cache-Control";
static const mp_string CONTENT_ENCODING = "Content-Encoding";
static const mp_string UNAME = "HTTP_X_AUTH_USER";
static const mp_string PW = "HTTP_X_AUTH_KEY";
static const mp_string LISTEN_ADDR = "SERVER_ADDR";
static const mp_string CLIENT_CERT_DN = "CLIENT_CERT_DN";

// 如下json消息key定义，agent主进程和so均会用到，提取在这里定义
static const mp_string REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME = "freezeFile";
static const mp_string REST_PARAM_HOST_FREEZE_SCRIPT_PARAM = "freezeParams";
static const mp_string REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME = "unfreezeFile";
static const mp_string REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM = "unfreezeParam";
static const mp_string REST_PARAM_HOST_QUERY_SCRIPT_FILENAME = "queryFile";
static const mp_string REST_PARAM_HOST_QUERY_SCRIPT_PARAM = "queryParam";
static const mp_string REST_PARAM_HOST_FREEZE_TIMEOUT = "freezeTimeOut";
static const mp_string REST_ISUSERDEFINED_SCRIPT = "isUserDefined";
static const mp_string REST_PARAM_MANAGER_SERVER = "managerServerList";
#endif  // __AGENT_REST_INTERFACES_H__
