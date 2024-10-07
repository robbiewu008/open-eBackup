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
#ifndef MODULE_DEFINES_H
#define MODULE_DEFINES_H

#include "define/Types.h"
#include <memory>

#ifndef _AGENT_DEFINES_H_

namespace Module {

static const int AGENT_DECIMAL = 10;  // 十进制转换
static const int MAX_FULL_PATH_LEN = 300;
static const int MAX_PATH_LEN = 260;
static const int MAX_FILE_NAME_LEN = 80;
static const int MAX_LINE_SIZE = 2048;
static const int MAX_SINGED_INTEGER_VALUE = 2147483647;
static const int MAX_UNSIGNED_INT_VALUE = 4294967295;
static const int MAX_HOSTNAME_LENGTH = 260;   // 主机名长度
static const int MAX_MAIN_CMD_LENGTH = 1000;  // 系统命令长度
static const int MAX_ARRAY_SN_LEN = 20;       // SN最大长度
static const int MAX_ERROR_MSG_LEN = 256;
static const int MAX_IP_LEN = 64;
static const int MIN_VALID_PORT = 1024;
static const int MAX_VALID_PORT = 65535;
static const int MAX_TASKID_LEN = 60;
static const int MAX_STRING_LEN = 512;
static const double MAX_DPP_HBTIMEOUT = 180;  // 连续3分钟没有收到心跳ack则认定超时

static const int HOST_TYPE_WINDOWS = 1;   // Windows
static const int HOST_TYPE_REDHAT = 2;    // RedHat
static const int HOST_TYPE_HP_UX_IA = 3;  // HPUX IA
static const int HOST_TYPE_SOLARIS = 4;   // SOLARIS
static const int HOST_TYPE_AIX = 5;       // AIX
static const int HOST_TYPE_SUSE = 6;      // SUSE
static const int HOST_TYPE_ROCKY = 7;     // ROCKY
static const int HOST_TYPE_OEL = 8;       // OEL
static const int HOST_TYPE_ISOFT = 9;
static const int HOST_TYPE_CENTOS = 10;

static const int HOST_OS_BIT_64 = 64;  // 64位操作系统
static const int HOST_OS_BIT_32 = 32;  // 32位操作系统

// device type
static const int DEVICE_TYPE_FILESYS = 0;      // 文件系统
static const int DEVICE_TYPE_RAW = 1;          // 裸设备
static const int DEVICE_TYPE_ASM_LIB = 2;      // ASMLib磁盘
static const int DEVICE_TYPE_ASM_RAW = 3;      // ASM裸设备
static const int DEVICE_TYPE_ASM_LINK = 4;     // ASM软链接
static const int DEVICE_TYPE_ASM_UDEV = 5;     // ASMOnUdev
static const int DEVICE_TYPE_ASM_DISK_ID = 6;  // windows ASM磁盘标识符

// volume type
static const int VOLUME_TYPE_SIMPLE = 0;      // 简单卷（无卷管理）
static const int VOLUME_TYPE_LINUX_LVM = 1;   // Linux LVM
static const int VOLUME_TYPE_LINUX_VXVM = 2;  // Linux VxVM
static const int VOLUME_TYPE_AIX_LVM = 3;     // AIX卷管理
static const int VOLUME_TYPE_HP_LVM = 4;      // HP卷管理
static const int VOLUME_TYPE_UDEV = 5;        // UDEV设备映射

static const int FREEZE_STAT_FREEZED = 0;   // 冻结中
static const int FREEZE_STAT_UNFREEZE = 1;  // 解冻
static const int FREEZE_STAT_UNKNOWN = 2;   // 未知

// freeze interval
static const int FREEZE_MIX_INTERVAL = 0;      // 最小冻结间隔
static const int FREEZE_MAX_INTERVAL = 3600;   // 最大冻结间隔
static const int FREEZE_DEFAULT_INTERVAL = 5;  // 默认冻结间隔

#ifdef WIN32
// windows errorcode define
static const int WIN_ERROR_FILE_NOT_FOUND = 2;  // The system cannot find the file specified.
#define strerror_r(errno,buf,len) strerror_s(buf,len,errno)   // 适配windows


#include <winsock2.h>
#if BYTE_ORDER == LITTLE_ENDIAN

	#define htobe16(x) htons(x)
	#define htole16(x) (x)
	#define be16toh(x) ntohs(x)
	#define le16toh(x) (x)

	#define htobe32(x) htonl(x)
	#define htole32(x) (x)
	#define be32toh(x) ntohl(x)
	#define le32toh(x) (x)

	#define htobe64(x) htonll(x)
	#define htole64(x) (x)
	#define be64toh(x) ntohll(x)
	#define le64toh(x) (x)

#elif BYTE_ORDER == BIG_ENDIAN


	#define htobe16(x) (x)
	#define htole16(x) __builtin_bswap16(x)
	#define be16toh(x) (x)
	#define le16toh(x) __builtin_bswap16(x)

	#define htobe32(x) (x)
	#define htole32(x) __builtin_bswap32(x)
	#define be32toh(x) (x)
	#define le32toh(x) __builtin_bswap32(x)
	#define htobe64(x) (x)
	#define htole64(x) __builtin_bswap64(x)
	#define be64toh(x) (x)
	#define le64toh(x) __builtin_bswap64(x)
#endif
#endif

// 脚本参数
static const std::string SCRIPTPARAM_INSTNAME = "INSTNAME=";
static const std::string SCRIPTPARAM_DBNAME = "DBNAME=";
static const std::string SCRIPTPARAM_DBUSERNAME = "DBUSERNAME=";
static const std::string SCRIPTPARAM_DBPASSWORD = "DBPASSWORD=";
static const std::string SCRIPTPARAM_CLUSTERTYPE = "CLUSTERTYPE=";
static const std::string SCRIPTPARAM_RESGRPNAME = "RESGRPNAME=";
static const std::string SCRIPTPARAM_DEVGRPNAME = "DEVGRPNAME=";
static const std::string SQLSERVER_SCRIPTPARAM_TABNAME = "TABLESPACENAME=";
static const std::string SQLSERVER_SCRIPTPARAM_CLUSTERFLAG = "ISCLUSTER=";
static const std::string SQLSERVER_SCRIPTPARAM_CHECKTYPE = "CHECKTYPE=";
static const std::string SCRIPTPARAM_CLUSTERNAME = "CLUSTERNAME=";

// *** BEGIN *** DTS2014071801749 y00275736 20014-07-24
// 该超时时间由10秒增加到60秒，带IO测试时10秒超时冻结操作失败概率很高
// Max wait time for frozen event
static const int VSS_TIMEOUT_FREEZE_MSEC = 60000;
// *** END *** DTS2014071801749 y00275736 20014-07-24
// Call QueryStatus every 10 ms while waiting for frozen event
static const int VSS_TIMEOUT_EVENT_MSEC = 10;
static const int VSS_TIMEOUT_MSEC = 60000;  // 60 * 1000
static const int VSS_EXEC_MUTEX_TIMEOUT = 60000;

// VSS writer name
static const std::string VSS_SQLSERVER_WRITER_NAME = "SqlServerWriter";
static const std::wstring VSS_SQLSERVER_WRITER_NAME_W = L"SqlServerWriter";
static const std::string VSS_EXCHANGE_WRITER_NAME = "Microsoft Exchange Writer";
static const std::wstring VSS_EXCHANGE_WRITER_NAME_W = L"Microsoft Exchange Writer";
static const std::string VSS_FILESYSTEM_WRITER_NAME = "File System Writer";
static const std::wstring VSS_FILESYSTEM_WRITER_NAME_W = L"File System Writer";
// Events name
static const std::wstring EVENT_NAME_FROZEN = L"Global\\RDVSSEvent-frozen";
static const std::wstring EVENT_NAME_THAW = L"Global\\RDVSSEvent-thaw";
static const std::wstring EVENT_NAME_TIMEOUT = L"Global\\RDVSSEvent-timeout";
static const std::wstring EVENT_NAME_ENDBACKUP = L"Global\\RDVSSEvent-endbackup";
static const std::wstring RD_AGENT_SERVICE_NAME_W = L"ReplicationDirector Agent";

static const char IP_SEG_1 = 1;
static const char IP_SEG_2 = 2;
static const char IP_SEG_3 = 3;

static const int LENGTH_SEC_DESC = 128;
static const int ERR_INFO_SIZE = 256;
static const int LENGTH_DISK_DESC = 1024;  // 磁盘集的字符串长度
static const std::string DEFAULT_RDVSS_LOG_PATH = "C:\\";

// 分隔符
static const std::string STR_COMMA = ",";  // 字符串中的逗号
static const std::string STR_SEMICOLON = ";";  // 字符串中的分号，Windows下已此为分隔符，避免与路径中的冒号冲突
static const std::string STR_COLON = ":";  // 字符串中的冒号
static const std::string STR_DASH = "-";
static const std::string STR_PLUS = "+";
static const std::string STR_VERTICAL_LINE = "|";
static const std::string STR_DOUBLE_VERTICAL_LINE = "||";
static const std::string STR_ADDRESS = "&";
static const std::string STR_DOUBLE_ADDRESS = "&&";
static const std::string STR_SPACE = " ";
static const char CHAR_COMMA = ',';
static const char CHAR_SEMICOLON = ';';
static const char CHAR_COLON = ':';
static const char CHAR_VERTICAL_LINE = '|';
static const char CHAR_SLASH = '/';
static const std::string STR_EQUAL = "=";
static const std::string GET_STR_EQUAL = "%3D";
static const std::string STR_CODE_WARP = "\n";
static const std::string STR_QUOTATION_MARK = "\"";
static const std::string SIGN_FORMAT_STR = "=";
static const std::string SIGN_IN = "<";
static const std::string SIGN_OUT = ">";
static const std::string SIGN_BACKQUOTE = "`";
static const std::string SIGN_EXCLAMATION = "!";
static const std::string SIGN_DOLLAR = "$";
static const std::string STR_BACKSLASH = "\\";

// 日志文件名称
static const std::string AGENT_LOG_NAME = "rdagent.log";
static const std::string ROOT_EXEC_LOG_NAME = "rootexec.log";
static const std::string CRYPTO_LOG_NAME = "crypto.log";
static const std::string RD_VSS_LOG_NAME = "rdvss.log";
static const std::string MONITOR_LOG_NAME = "monitor.log";
static const std::string WIN_SERVICE_LOG_NAME = "winservice.log";
static const std::string XML_CFG_LOG_NAME = "xmlcfg.log";
static const std::string AGENT_CLI_LOG_NAME = "agentcli.log";
static const std::string GET_INPUT_LOG_NAME = "getinput.log";
static const std::string SCRIPT_SIGN_LOG_NAME = "scriptsign.log";
static const std::string DATA_MIGRA_LOG_NAME = "datamigra.log";
static const std::string AGENT_LOG_ZIP_NAME = "sysinfo";
static const std::string SHOW_STATISTICS = "showStatistics.log";
static const std::string UPGRADE_KERNEL = "upgradeKernel.log";
static const std::string XBSA_CLIENT_LOG_NAME = "xbsa_client.log";
static const std::string TMP = "Tmp_";
static const std::string DOT = ".";

// 可执行程序名称
static const std::string AGENT_EXEC_NAME = "rdagent";
static const std::string ROOT_EXEC_NAME = "rootexec";
static const std::string CRYPT_TOOL_NAME = "crypto";
static const std::string MONITOR_EXEC_NAME = "monitor";
static const std::string NGINX_EXEC_NAME = "rdnginx";
static const std::string WIN_SERVICE_EXEC_NAME = "winservice";
static const std::string XML_CFG_EXEC_NAME = "xmlcfg";
static const std::string GET_INPUT_EXEC_NAME = "getinput";
static const std::string OM_DPP_EXEC_NAME = "dataprocess";
static const std::string SERVICE_TYPE = "serviceType";

// nginx作为参数名
static const std::string NGINX_AS_PARAM_NAME = "nginx";
// 检查IPV4 IPV6格式参数
static const std::string IPTYPE_AS_PARAM_NAME = "IP4ORIPV6";
static const std::string IP6_AS_PARAM_NAME = "IPV6";

// 配置文件
static const std::string AGENT_XML_CONF = "agent_cfg.xml";
static const std::string ALARMINFO_XML_CONF = "alarm_info.xml";
static const std::string AGENT_PLG_CONF = "pluginmgr.xml";
static const std::string AGENT_NGINX_CONF_FILE = "nginx.conf";
static const std::string AGENT_SCRIPT_SIGN = "script.sig";

static const std::string HDRS_CA_NAME = "Huawei_Cloud_CA.cer";
static const std::string HDRS_HA_KEY_NAME = "HostAgentUser.key";
static const std::string HDRS_HA_CERT_NAME = "HostAgentUser.pem";

// nginx优化参数
static const std::string CFG_SSL_KEY_PASSWORD = "ssl_key_password";
static const std::string SSL_PASSWORD_TEMP_FILE = "nginxPassword.tmp";
static const std::string NGINX_START = "startnginx";
static const std::string NGINX_RELOAD = "reloadnginx";
static const std::string AGENTCLI_UNIX = "agentcli";
// 脚本文件
static const std::string SCRIPT_QUERY_INITIATOR = "initialtor.sh";
static const std::string SCRIPT_ACTION_DB2 = "Db2_sample.sh";
// 运行参数依赖文件
static const std::string CFG_RUNNING_PARAM = "testcfg.tmp";
static const std::string CMD_RUNNING_UPGRADE_ERROR_MSG = "errormsg.log";
#ifndef WIN32
static const std::string START_SCRIPT = "agent_start.sh";
static const std::string STOP_SCRIPT = "agent_stop.sh";
static const std::string UPGRADE_SCRIPT = "upgrade.sh";
static const std::string AGENT_CHECKIP = "checkIpType.sh";
static const std::string ZIP_SUFFIX = ".tar.gz";
static const std::string RELOAD_NGINX_SCRIPT = "agent_reload_nginx.sh";
#else
static const std::string START_SCRIPT = "process_start.bat";
static const std::string STOP_SCRIPT = "process_stop.bat";
static const std::string UPGRADE_SCRIPT = "upgrade.bat";
static const std::string AGENT_CHECKIP = "checkIpType.bat";
static const std::string ZIP_SUFFIX = ".zip";

// nginx优化参数
static const std::string AGENTCLI_EXE = "agentcli.exe";
#endif

// 目录名称
static const std::string AGENT_TMP_DIR = "tmp";
static const std::string AGENT_STMP_DIR = "stmp";
static const std::string AGENT_CONF_DIR = "conf";
static const std::string AGENT_BIN_DIR = "bin";
static const std::string AGENT_SBIN_DIR = "sbin";
static const std::string AGENT_LIB_DIR = "lib";
static const std::string AGENT_PLUGIN_DIR = "plugins";
static const std::string AGENT_PLUGIN_INSTALL_DIR = "Plugins";
static const std::string AGENT_PROTECT_CLIENT_E_DIR = "ProtectClient-E";
static const std::string AGENT_LOG_DIR = "log";
static const std::string AGENT_SLOG_DIR = "slog";
static const std::string AGENT_EBKUSER_DIR = "ebk_user";
static const std::string AGENT_THIRDPARTY_DIR = "thirdparty";
static const std::string AGENT_DB = "db";
static const std::string AGENT_NGINX = "nginx";
static const std::string AGENT_NGINX_LOGS = "logs";
static const std::string AGENT_NGINX_CONF = "conf";
static const std::string AGENT_USER_DEFINED_DIR = "rd_user";
static const std::string AGENT_SAMPLE_DIR = "sample";
static const std::string AGENT_SAMPLE_SCRIPT = "0";
static const std::string AGENT_USER_DEFINED_SCRIPT = "1";

// 一些公用符号定义
static const std::string NODE_COLON = "\n";     // 传递脚本参数使用换行符\n作为分割符号
static const std::string NODE_SEMICOLON = ";";  // 字符串中的分号

// windows服务相关
static const std::string MONITOR_SERVICE_NAME = "RdMonitor";
static const std::string AGENT_SERVICE_NAME = "RdAgent";
static const std::string NGINX_SERVICE_NAME = "RdNginx";
static const std::string INSTALL_OPERATOR = "install";
static const std::string UNINSTALL_OPERATOR = "uninstall";
static const std::string RUN_OPERATOR = "run";

// 进程pid文件
static const std::string AGENT_PID = "rdagent.pid";
static const std::string NGINX_PID = "nginx.pid";
static const std::string MONITOR_PID = "monitor.pid";

// host SN文件名
static const std::string HOSTSN_FILE = "HostSN";
// host SN文件夹名称
#ifndef WIN32
static const std::string HOSTSN_DIR = "/etc/HostSN/";
#else
static const std::string HOSTSN_DIR = "C:/Users/Default/";
#endif
// 附件相关内部json对象key
static const std::string REST_PARAM_ATTACHMENT_NAME = "attachmentName";  // 附件名称
static const std::string REST_PARAM_ATTACHMENT_PATH = "attachmentPath";  // 附件完整路径，包括文件名称

// 鉴权类型
static const std::string AGENT_PASSWORD_AUTH = "1";     // 密码鉴权
static const std::string AGENT_CERTIFICATE_AUTH = "2";  // 证书认证

static const std::string AGENT_CERT_DN_KEY = "CN=";
static const std::string AGENT_CERT_DN_VALUE = "BCManager eBackup Client Cert";

// DataProcess
static const std::string DATAPROCESS_LOGNAME = "dataprocess";
static const std::string DATAPROCESS_XML_CONF = "dataprocess_cfg.xml";
static const std::string DATAPROCESS_PID_FILE = "dataprocess";
static const std::string LOCAL_IPADDR = "127.0.0.1";

static const int DPPMESSAGE_TYPE = 1;
static const int REQMESSAGE_TYPE = 2;

// external plugin
static const std::string EXTERNAL_PLUGIN_PATH = "Plugins";
static const std::string EXTERNAL_PLUGIN_START_SCRIPT = "start.sh";
static const std::string EXTERNAL_PLUGIN_STOP_SCRIPT = "stop.sh";

#ifdef REST_PUBLISH
static const std::string AGENT_ROOT_PATH = "/home/rdadmin/Agent/";
#endif
// DataProcessType
enum dataProcessService { OCEAN_MOBILITY_SERVICE = 1, OCEAN_VMWARENATIVE_SERVICE = 2, INVALID_SERVICE };

#ifdef WIN32
// 解决静态链接fcgi找不到符号的问题，提示找不到__imp_FXGXxxxxx符号，
// 而__imp_符号是动态库导入库的符号前缀，是在使用DLLIMPORT的时候生成的.
#define DLLAPI
#ifdef AGENT_DLL_EXPORTS  // specified in vs
#define AGENT_API __declspec(dllexport)
#else
#define AGENT_API __declspec(dllimport)
#endif
#define AGENT_EXPORT __declspec(dllexport)
#define AGENT_IMPORT __declspec(dllimport)
#define AGENT_STDCALL __stdcall
static const std::string PATH_SEPARATOR = "\\";
#define IS_DIR_SEP_CHAR(x) ('\\' == (x) || '/' == (x))
static const std::string LIB_SUFFIX = ".dll";
static const char PATH_SEPCH = ';';
#define GETCH _getch()
#define ITOA(val, buf, size, radix) itoa(val, buf, radix)
#define I64ITOA(val, buf, size, radix) _i64toa(val, buf, radix)
#else  // WIN32
#define AGENT_API

#ifdef LINUX
#define AGENT_EXPORT __attribute__((visibility("default")))
#else
#define AGENT_EXPORT
#endif

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

#define AGENT_IMPORT
#define AGENT_STDCALL
static const std::string PATH_SEPARATOR = "/";
#define IS_DIR_SEP_CHAR(x) ('/' == (x))
static const std::string LIB_SUFFIX = ".so";
static const char PATH_SEPCH = ':';
#define GETCH getch()
#define ITOA(val, buf, size, radix) itoa(val, buf, radix)
#endif

#define IS_SPACE(x) (x == ' ' || x == '\t')
#define strempty(str) (0 == str || 0 == (*str))

#ifdef WIN32
static const std::string MOBILITY_DEVICE_NAME = "\\\\.\\IoMirrorDev";
#define OM_IOCTL_START CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_VERIFY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_MODIFY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_STOP CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_VOL_ADD CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_VOL_DELETE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_NOTIFY_CHANGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_PAUSE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_RESUME CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_VOL_MOD CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define OM_IOCTL_GET_STATISTICS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#else
static const std::string MOBILITY_DEVICE_NAME = "/dev/im_ctldev";
#define IM_CTL_MAGIC 'k'
#define OM_IOCTL_START _IOWR(IM_CTL_MAGIC, 1, om_drv_protect_strategy_t)
#define OM_IOCTL_STOP _IO(IM_CTL_MAGIC, 2)
#define OM_IOCTL_MODIFY _IOWR(IM_CTL_MAGIC, 3, om_drv_protect_strategy_t)
#define OM_IOCTL_PAUSE _IOW(IM_CTL_MAGIC, 4, WaitFlushQueue)
#define OM_IOCTL_RESUME _IO(IM_CTL_MAGIC, 5)
#define OM_IOCTL_VERIFY _IOWR(IM_CTL_MAGIC, 12, om_drv_protect_strategy_t)
#define OM_IOCTL_VOL_ADD _IOW(IM_CTL_MAGIC, 7, om_drv_protect_vol_t)
#define OM_IOCTL_VOL_DELETE _IOW(IM_CTL_MAGIC, 8, om_drv_protect_vol_t)
#define OM_IOCTL_NOTIFY_CHANGE
#define OM_IM_CTL_MOD_VOLUME _IOW(IM_CTL_MAGIC, 27, ProtectVol)
#define OM_IOCTL_GET_STATISTICS _IOR(IM_CTL_MAGIC, 29, StatisticsInfo)
#define OM_IOCTL_GET_KERNEL_ALARM _IOR(IM_CTL_MAGIC, 30, GetAlarm)
#define OM_IOCTL_SET_TOKEN_ID _IOW(IM_CTL_MAGIC, 31, SetTokenID)

/*
/etc/huawei/im.conf
保存结构体struct im_config_pg，是保护时对应的信息，放到/etc目录下，系统启动时可以通过文件系统方式加载，
不能放到/boot目录，dirver启动时/boot目录未加载
/boot/hwsnap_bitmap
保存关机时卷的bitmap信息，关机时agent创建bitmap文件，把偏移发给driver，driver在关机前会把内存中的bitmap
通过磁盘偏移的方式写入文件中，只支持简单卷，只能放到/boot目录
/boot/huawei/im.conf
保存结构体struct im_config_variable，关机时agent创建配置文件，把偏移发给driver，driver在关机前会把内存中
结构体通过磁盘偏移的方式写入文件中，只支持简单卷，只能放到/boot目录
ps.因为/boot/hwsnap_bitmap需要和windows保持一致，/boot/huawei/im.conf的内容放到/boot/hwsnap_bitmap
不合适
*/
// linux driver配置文件
static const std::string LINUX_DRIVER_CONF = "/etc/huawei/im.conf";
static const std::string BOOT_SAVED_BITMAP = "/boot/hwsnap_bitmap";
// linux driver启动的配置文件
static const std::string LINUX_DRIVER_BOOT_CONF = "/boot/huawei/im.conf";
#endif

// 安全函数snprintf_s需要用CHECK_FAIL进行判断，只有-1时表示出错
#define MODULE_CHECK_FAIL(Call)                                                                                               \
    {                                                                                                                  \
        int iCheckFailRet = Call;                                                                                      \
        if (FAILED == iCheckFailRet) {                                                                                 \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckFailRet);                                    \
            return FAILED;                                                                                             \
        }                                                                                                              \
    }

#define MODULE_CHECK_NOT_OK(Call)                                                                                             \
    {                                                                                                                  \
        int iCheckNotOkRet = Call;                                                                                     \
        if (EOK != iCheckNotOkRet) {                                                                                   \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);                                   \
            return FAILED;                                                                                             \
        }                                                                                                              \
    }

#define MODULE_CHECK_FAIL_NOLOG(Call)                                                                                         \
    {                                                                                                                  \
        int iCheckFailRet = Call;                                                                                      \
        if (FAILED == iCheckFailRet) {                                                                                 \
            return FAILED;                                                                                             \
        }                                                                                                              \
    }

// 将调用返回码返回
#define MODULE_CHECK_FAIL_EX(Call)                                                                                            \
    {                                                                                                                  \
        int iCheckNotOkRet = Call;                                                                                     \
        if (SUCCESS != iCheckNotOkRet) {                                                                               \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);                                   \
            return iCheckNotOkRet;                                                                                     \
        }                                                                                                              \
    }                                                                                                                  

// 判断指针是否为空
#define MODULE_CHECK_POINTER_NULL(Ptr)                                                                                        \
    {                                                                                                                  \
        if (Ptr == nullptr) {                                                                                          \
            COMMLOG(OS_LOG_ERROR, "%s Pointer is NULL.", #Ptr);                                                        \
            return FAILED;                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \


// 数据库冻结解冻状态，多个模块公用
enum DB_STATUS {
    DB_FREEZE = 0,
    DB_UNFREEZE,
    DB_UNKNOWN,
    DB_FREEZING,
    DB_UNFREEZING,
    DB_BUTT  // 枚举结束
};

#if defined(LINUX)
// 定义模板实现C++11支持make_unique(C++14提供支持)
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

}

using mp_string = std::string;
using mp_int32 = int;
using mp_uint32 = unsigned int;
#define BLOCK_SIZE 4194304
#define DORADO_BLOCK_SIZE  (((uint64_t)256)*1024)
#define DIRTYRANGE_BATCH_LEN     (((uint64_t)8)*1024*1024*1024)  //10G

#endif  // _AGENT_DEFINES_H_

#endif  // MODULE_DEFINES_H
