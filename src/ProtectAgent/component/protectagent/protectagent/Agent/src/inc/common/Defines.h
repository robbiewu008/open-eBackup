#ifndef _AGENT_DEFINES_H_
#define _AGENT_DEFINES_H_

#include "common/Types.h"
#include "securec.h"
#include <memory>

static const mp_int32 PER_SENT = 100;  // 百分

static const mp_int32 AGENT_DECIMAL = 10;  // 十进制转换
static const mp_uint32 HTTP_TIME_OUT = 30;     // 30S
static const mp_uint32 HTTP_TIME_OUT_ONE_MIN = 60;     // 60S

static const mp_int32 MAX_FULL_PATH_LEN = 300;
static const mp_int32 MAX_PATH_LEN = 260;
static const mp_int32 MAX_FILE_NAME_LEN = 80;
static const mp_int32 MAX_LINE_SIZE = 2048;
static const mp_int32 MAX_SINGED_INTEGER_VALUE = 2147483647;
static const mp_uint32 MAX_UNSIGNED_INT_VALUE = 4294967295;
static const mp_int32 MAX_HOSTNAME_LENGTH = 260;   // 主机名长度
static const mp_int32 MAX_MAIN_CMD_LENGTH = 1000;  // 系统命令长度
static const mp_int32 MAX_ARRAY_SN_LEN = 20;       // SN最大长度
static const mp_int32 MAX_ERROR_MSG_LEN = 256;
static const mp_int32 MAX_IP_LEN = 64;
static const mp_int32 MIN_VALID_PORT = 1024;
static const mp_int32 MAX_VALID_PORT = 65535;
static const mp_int32 MAX_TASKID_LEN = 60;
static const mp_int32 MAX_STRING_LEN = 512;
static const mp_double MAX_DPP_HBTIMEOUT = 180;  // 连续3分钟没有收到心跳ack则认定超时

static const mp_int32 HOST_TYPE_WINDOWS = 1;   // Windows
static const mp_int32 HOST_TYPE_REDHAT = 2;    // RedHat
static const mp_int32 HOST_TYPE_HP_UX_IA = 3;  // HPUX IA
static const mp_int32 HOST_TYPE_SOLARIS = 4;   // SOLARIS
static const mp_int32 HOST_TYPE_AIX = 5;       // AIX
static const mp_int32 HOST_TYPE_SUSE = 6;      // SUSE
static const mp_int32 HOST_TYPE_ROCKY = 7;     // ROCKY
static const mp_int32 HOST_TYPE_OEL = 8;       // OEL
static const mp_int32 HOST_TYPE_ISOFT = 9;
static const mp_int32 HOST_TYPE_CENTOS = 10;

static const mp_int32 HOST_OS_BIT_64 = 64;  // 64位操作系统
static const mp_int32 HOST_OS_BIT_32 = 32;  // 32位操作系统

// device type
static const mp_int32 DEVICE_TYPE_FILESYS = 0;      // 文件系统
static const mp_int32 DEVICE_TYPE_RAW = 1;          // 裸设备
static const mp_int32 DEVICE_TYPE_ASM_LIB = 2;      // ASMLib磁盘
static const mp_int32 DEVICE_TYPE_ASM_RAW = 3;      // ASM裸设备
static const mp_int32 DEVICE_TYPE_ASM_LINK = 4;     // ASM软链接
static const mp_int32 DEVICE_TYPE_ASM_UDEV = 5;     // ASMOnUdev
static const mp_int32 DEVICE_TYPE_ASM_DISK_ID = 6;  // windows ASM磁盘标识符

// volume type
static const mp_int32 VOLUME_TYPE_SIMPLE = 0;      // 简单卷（无卷管理）
static const mp_int32 VOLUME_TYPE_LINUX_LVM = 1;   // Linux LVM
static const mp_int32 VOLUME_TYPE_LINUX_VXVM = 2;  // Linux VxVM
static const mp_int32 VOLUME_TYPE_AIX_LVM = 3;     // AIX卷管理
static const mp_int32 VOLUME_TYPE_HP_LVM = 4;      // HP卷管理
static const mp_int32 VOLUME_TYPE_UDEV = 5;        // UDEV设备映射

static const mp_int32 FREEZE_STAT_FREEZED = 0;   // 冻结中
static const mp_int32 FREEZE_STAT_UNFREEZE = 1;  // 解冻
static const mp_int32 FREEZE_STAT_UNKNOWN = 2;   // 未知

// freeze interval
static const mp_int32 FREEZE_MIX_INTERVAL = 0;      // 最小冻结间隔
static const mp_int32 FREEZE_MAX_INTERVAL = 3600;   // 最大冻结间隔
static const mp_int32 FREEZE_DEFAULT_INTERVAL = 5;  // 默认冻结间隔

// logic port type
static const mp_int32 LOGIC_PORT_TYPE_ETH = 1;
static const mp_int32 LOGIC_PORT_TYPE_BOND = 7;
static const mp_int32 LOGIC_PORT_TYPE_VLAN = 8;
static const mp_int32 LOGIC_PORT_TYPE_VIP = 25;
static const mp_int32 LOGIC_PORT_TYPE_SIP = 26;

// appinfo lan type
static const mp_int32 APPINFO_LAN_TYPE_VLAN = 0;
static const mp_int32 APPINFO_LAN_TYPE_VXLAN = 1;

#ifdef WIN32
// windows errorcode define
static const mp_int32 WIN_ERROR_FILE_NOT_FOUND = 2;  // The system cannot find the file specified.

#endif

// 脚本参数
static const mp_string SCRIPTPARAM_INSTNAME = "INSTNAME=";
static const mp_string SCRIPTPARAM_DBNAME = "DBNAME=";
static const mp_string SCRIPTPARAM_DBUSERNAME = "DBUSERNAME=";
static const mp_string SCRIPTPARAM_DBPASSWORD = "DBPASSWORD=";
static const mp_string SCRIPTPARAM_CLUSTERTYPE = "CLUSTERTYPE=";
static const mp_string SCRIPTPARAM_RESGRPNAME = "RESGRPNAME=";
static const mp_string SCRIPTPARAM_DEVGRPNAME = "DEVGRPNAME=";
static const mp_string SQLSERVER_SCRIPTPARAM_TABNAME = "TABLESPACENAME=";
static const mp_string SQLSERVER_SCRIPTPARAM_CLUSTERFLAG = "ISCLUSTER=";
static const mp_string SQLSERVER_SCRIPTPARAM_CHECKTYPE = "CHECKTYPE=";
static const mp_string SCRIPTPARAM_CLUSTERNAME = "CLUSTERNAME=";

// *** BEGIN *** DTS2014071801749 y00275736 20014-07-24
// 该超时时间由10秒增加到60秒，带IO测试时10秒超时冻结操作失败概率很高
// Max wait time for frozen event
static const mp_int32 VSS_TIMEOUT_FREEZE_MSEC = 60000;
// *** END *** DTS2014071801749 y00275736 20014-07-24
// Call QueryStatus every 10 ms while waiting for frozen event
static const mp_int32 VSS_TIMEOUT_EVENT_MSEC = 10;
static const mp_int32 VSS_TIMEOUT_MSEC = 60000;  // 60 * 1000
static const mp_int32 VSS_EXEC_MUTEX_TIMEOUT = 60000;

// VSS writer name
static const mp_string VSS_SQLSERVER_WRITER_NAME = "SqlServerWriter";
static const mp_wstring VSS_SQLSERVER_WRITER_NAME_W = L"SqlServerWriter";
static const mp_string VSS_EXCHANGE_WRITER_NAME = "Microsoft Exchange Writer";
static const mp_wstring VSS_EXCHANGE_WRITER_NAME_W = L"Microsoft Exchange Writer";
static const mp_string VSS_FILESYSTEM_WRITER_NAME = "File System Writer";
static const mp_wstring VSS_FILESYSTEM_WRITER_NAME_W = L"File System Writer";
// Events name
static const mp_wstring EVENT_NAME_FROZEN = L"Global\\RDVSSEvent-frozen";
static const mp_wstring EVENT_NAME_THAW = L"Global\\RDVSSEvent-thaw";
static const mp_wstring EVENT_NAME_TIMEOUT = L"Global\\RDVSSEvent-timeout";
static const mp_wstring EVENT_NAME_ENDBACKUP = L"Global\\RDVSSEvent-endbackup";
static const mp_wstring RD_AGENT_SERVICE_NAME_W = L"ReplicationDirector Agent";

static const mp_char IP_SEG_1 = 1;
static const mp_char IP_SEG_2 = 2;
static const mp_char IP_SEG_3 = 3;

static const mp_int32 LENGTH_SEC_DESC = 128;
static const mp_int32 ERR_INFO_SIZE = 256;
static const mp_int32 LENGTH_DISK_DESC = 1024;  // 磁盘集的字符串长度
static const mp_string DEFAULT_RDVSS_LOG_PATH = "C:\\";

// 分隔符
static const mp_string STR_COMMA = ",";  // 字符串中的逗号
static const mp_string STR_SEMICOLON = ";";  // 字符串中的分号，Windows下已此为分隔符，避免与路径中的冒号冲突
static const mp_string STR_COLON = ":";  // 字符串中的冒号
static const mp_string STR_DASH = "-";
static const mp_string STR_PLUS = "+";
static const mp_string STR_VERTICAL_LINE = "|";
static const mp_string STR_DOUBLE_VERTICAL_LINE = "||";
static const mp_string STR_ADDRESS = "&";
static const mp_string STR_DOUBLE_ADDRESS = "&&";
static const mp_string STR_SPACE = " ";
static const mp_string STR_BACKSLASH = "\\";
static const mp_char CHAR_COMMA = ',';
static const mp_char CHAR_SEMICOLON = ';';
static const mp_char CHAR_COLON = ':';
static const mp_char CHAR_VERTICAL_LINE = '|';
static const mp_char CHAR_SLASH = '/';
static const mp_char CHAR_UNDERLINE = '_';
static const mp_string STR_EQUAL = "=";
static const mp_string GET_STR_EQUAL = "%3D";
static const mp_string STR_CODE_WARP = "\n";
static const mp_string STR_QUOTATION_MARK = "\"";
static const mp_string SIGN_FORMAT_STR = "=";
static const mp_string SIGN_IN = "<";
static const mp_string SIGN_OUT = ">";
static const mp_string SIGN_BACKQUOTE = "`";
static const mp_string SIGN_EXCLAMATION = "!";
static const mp_string SIGN_DOLLAR = "$";
static const mp_string SIGN_UNDERLINE = "_";

// 日志文件名称
static const mp_string AGENT_LOG_NAME = "rdagent.log";
static const mp_string ROOT_EXEC_LOG_NAME = "rootexec.log";
static const mp_string CRYPTO_LOG_NAME = "crypto.log";
static const mp_string RD_VSS_LOG_NAME = "rdvss.log";
static const mp_string MONITOR_LOG_NAME = "monitor.log";
static const mp_string WIN_SERVICE_LOG_NAME = "winservice.log";
static const mp_string XML_CFG_LOG_NAME = "xmlcfg.log";
static const mp_string AGENT_CLI_LOG_NAME = "agentcli.log";
static const mp_string GET_INPUT_LOG_NAME = "getinput.log";
static const mp_string SCRIPT_SIGN_LOG_NAME = "scriptsign.log";
static const mp_string DATA_MIGRA_LOG_NAME = "datamigra.log";
static const mp_string AGENT_LOG_ZIP_NAME = "AGENTLOG";
static const mp_string SHOW_STATISTICS = "showStatistics.log";
static const mp_string UPGRADE_KERNEL = "upgradeKernel.log";
static const mp_string XBSA_CLIENT_LOG_NAME = "xbsa_client.log";
static const mp_string AGENT_PUSH_MODEFILE_PATH = "AgentInstallMode.log";
// 可执行程序名称
static const mp_string AGENT_EXEC_NAME = "rdagent";
static const mp_string ROOT_EXEC_NAME = "rootexec";
static const mp_string CRYPT_TOOL_NAME = "crypto";
static const mp_string MONITOR_EXEC_NAME = "monitor";
static const mp_string NGINX_EXEC_NAME = "rdnginx";
static const mp_string WIN_SERVICE_EXEC_NAME = "winservice";
static const mp_string XML_CFG_EXEC_NAME = "xmlcfg";
static const mp_string GET_INPUT_EXEC_NAME = "getinput";
static const mp_string OM_DPP_EXEC_NAME = "dataprocess";
static const mp_string SERVICE_TYPE = "serviceType";

// nginx作为参数名
static const mp_string NGINX_AS_PARAM_NAME = "nginx";
// 检查IPV4 IPV6格式参数
static const mp_string IPTYPE_AS_PARAM_NAME = "IP4ORIPV6";
static const mp_string IP6_AS_PARAM_NAME = "IPV6";

// 配置文件
static const mp_string AGENT_XML_CONF = "agent_cfg.xml";
static const mp_string XBSA_JSON_CONF = "xbsa.json";
static const mp_string ALARMINFO_XML_CONF = "alarm_info.xml";
static const mp_string AGENT_PLG_CONF = "pluginmgr.xml";
static const mp_string AGENT_NGINX_CONF_FILE = "nginx.conf";
static const mp_string AGENT_SCRIPT_SIGN = "script.sig";

static const mp_string HDRS_CA_NAME = "Huawei_Cloud_CA.cer";
static const mp_string HDRS_HA_KEY_NAME = "HostAgentUser.key";
static const mp_string HDRS_HA_CERT_NAME = "HostAgentUser.pem";

// system参数
static const mp_string WORKING_USER_PASSWORD = "working_user_passward";
// nginx优化参数
static const mp_string CFG_SSL_KEY_PASSWORD = "ssl_key_password";
static const mp_string SSL_PASSWORD_TEMP_FILE = "nginxPassword.tmp";
static const mp_string NGINX_START = "startnginx";
static const mp_string NGINX_RELOAD = "reloadnginx";
static const mp_string AGENTCLI_UNIX = "agentcli";
// 脚本文件
static const mp_string SCRIPT_QUERY_INITIATOR = "initialtor.sh";
static const mp_string SCRIPT_ACTION_DB2 = "Db2_sample.sh";
// 运行参数依赖文件
static const mp_string CFG_CLIENT_CONF_FILE = "client.conf";
static const mp_string CFG_RUNNING_PARAM = "testcfg.tmp";
static const mp_string CMD_RUNNING_UPGRADE_ERROR_MSG = "errormsg.log";
#ifndef WIN32
static const mp_string START_SCRIPT = "agent_start.sh";
static const mp_string STOP_SCRIPT = "agent_stop.sh";
static const mp_string UPGRADE_SCRIPT = "upgrade.sh";
static const mp_string AGENT_CHECKIP = "checkIpType.sh";
static const mp_string ZIP_SUFFIX = ".tar.gz";
static const mp_string RELOAD_NGINX_SCRIPT = "agent_reload_nginx.sh";
#else
static const mp_string START_SCRIPT = "agent_start.bat";
static const mp_string STOP_SCRIPT = "agent_stop.bat";
static const mp_string PROCESS_START_SCRIPT = "process_start.bat";
static const mp_string PROCESS_STOP_SCRIPT = "process_stop.bat";
static const mp_string UPGRADE_SCRIPT = "upgrade.bat";
static const mp_string UPGRADE_CHECK_SCRIPT = "upgrade_check.bat";
static const mp_string UPGRADE_PREPARE_SCRIPT = "upgrade_prepare.bat";
static const mp_string UPGRADE_CALLER_SCRIPT = "upgrade_caller.bat";
static const mp_string PUSH_UPDATE_CERT = "push_update_cert.bat";
static const mp_string MODIFY_CHECK_SCRIPT = "modify_check.bat";
static const mp_string MODIFY_PREPARE_SCRIPT = "modify_prepare.bat";
static const mp_string MODIFY_CALLER_SCRIPT = "modify_caller.bat";
static const mp_string AGENT_CHECKIP = "checkIpType.bat";
static const mp_string ZIP_SUFFIX = ".zip";
static const mp_string RELOAD_NGINX_SCRIPT = "agent_reload_nginx.bat";
static const mp_string SCRIPT_CIFS_OPERATION = "cifsoperation.bat";
static const mp_string SCRIPT_DATATURBO_MOUNT = "dataturbo_mount.bat";
// nginx优化参数
static const mp_string AGENTCLI_EXE = "agentcli.exe";
static const mp_string SCRIPT_CHECK_AND_CREATE_DATATURBO_LINK = "CreateDataturbolink.exe";
static const mp_string SCRIPT_DATATURBO_UMOUNT = "DataturboUmount.exe";

#endif

// agent升级校验完整性签名文件
static const mp_string UPGRADE_SIGNATURE_FILE = "upgrade_signature.sign";

// 目录名称
static const mp_string AGENT_BIN_DIR = "bin";
static const mp_string AGENT_LOG_DIR = "log";
static const mp_string AGENT_TMP_DIR = "tmp";
static const mp_string XBSA_LOG_DIR = "log";
static const mp_string XBSA_CONF_DIR = "conf";
static const mp_string XBSA_LIB_DIR = "lib";
static const mp_string XBSA_NGINX_DIR = "ngnix";
#ifdef WIN32
static const mp_string AGENT_DEFAULT_INSTALL_DIR = "C:";
static const mp_string AGENT_SBIN_DIR = "bin";
static const mp_string AGENT_SLOG_DIR = "log";
static const mp_string AGENT_STMP_DIR = "tmp";
#else
static const mp_string AGENT_DEFAULT_INSTALL_DIR = "/opt";
static const mp_string AGENT_SBIN_DIR = "sbin";
static const mp_string AGENT_SLOG_DIR = "slog";
static const mp_string AGENT_STMP_DIR = "stmp";
#endif // WIN32
static const mp_string AGENT_CONF_DIR = "conf";
static const mp_string AGENT_LIB_DIR = "lib";
static const mp_string AGENT_PLUGIN_DIR = "plugins";
static const mp_string AGENT_PLUGIN_INSTALL_DIR = "Plugins";
static const mp_string AGENT_EBKUSER_DIR = "ebk_user";
static const mp_string AGENT_THIRDPARTY_DIR = "thirdparty";
static const mp_string AGENT_DB = "db";
static const mp_string AGENT_NGINX = "nginx";
static const mp_string AGENT_NGINX_LOGS = "logs";
static const mp_string AGENT_NGINX_CONF = "conf";
static const mp_string AGENT_USER_DEFINED_DIR = "rd_user";
static const mp_string AGENT_SAMPLE_DIR = "sample";
static const mp_string AGENT_SAMPLE_SCRIPT = "0";
static const mp_string AGENT_USER_DEFINED_SCRIPT = "1";
static const mp_string AGENT_UPGRADE = "upgrade";
static const mp_string AGENT_VDDK_DIR = "vddk";


// 一些公用符号定义
static const mp_string NODE_COLON = "\n";     // 传递脚本参数使用换行符\n作为分割符号
static const mp_string NODE_SEMICOLON = ";";  // 字符串中的分号
static const mp_string NODE_COMMA = ",";      // 字符串中的逗号

// windows服务相关
static const mp_string MONITOR_SERVICE_NAME = "RdMonitor";
static const mp_string AGENT_SERVICE_NAME = "RdAgent";
static const mp_string NGINX_SERVICE_NAME = "RdNginx";
static const mp_string INSTALL_OPERATOR = "install";
static const mp_string UNINSTALL_OPERATOR = "uninstall";
static const mp_string CHANGE_USER_OPERATOR = "changeUser";
static const mp_string RUN_OPERATOR = "run";

// 进程pid文件
static const mp_string AGENT_PID = "rdagent.pid";
static const mp_string NGINX_PID = "nginx.pid";
static const mp_string MONITOR_PID = "monitor.pid";

// host SN文件名
static const mp_string HOSTSN_FILE = "HostSN";
// host SN文件夹名称
#ifndef WIN32
static const mp_string HOSTSN_DIR = "/etc/HostSN/";
#else
static const mp_string HOSTSN_DIR = "C:\\Users\\Default\\";
#endif
// 附件相关内部json对象key
static const mp_string REST_PARAM_ATTACHMENT_NAME = "attachmentName";  // 附件名称
static const mp_string REST_PARAM_ATTACHMENT_PATH = "attachmentPath";  // 附件完整路径，包括文件名称
static const mp_string REST_PARAM_LOG_COLLECT_STATUS = "status";       // 日志收集状态
static const mp_string REST_PARAM_LOG_COLLECT_INIT = "notstart";
static const mp_string REST_PARAM_LOG_COLLECT_COMPLETED = "completed";
static const mp_string REST_PARAM_LOG_COLLECT_COLLECTING = "collecting";
static const mp_string REST_PARAM_LOG_COLLECT_FAILED = "failed";

// 鉴权类型
static const mp_string AGENT_PASSWORD_AUTH = "1";     // 密码鉴权
static const mp_string AGENT_CERTIFICATE_AUTH = "2";  // 证书认证

static const mp_string AGENT_CERT_DN_KEY = "CN=";
static const mp_string AGENT_CERT_DN_VALUE = "BCManager eBackup Client Cert";

// DataProcess
static const mp_string DATAPROCESS_LOGNAME = "dataprocess";
static const mp_string DATAPROCESS_XML_CONF = "dataprocess_cfg.xml";
static const mp_string DATAPROCESS_PID_FILE = "dataprocess";
static const mp_string LOCAL_IPADDR = "127.0.0.1";

static const mp_int32 DPPMESSAGE_TYPE = 1;
static const mp_int32 REQMESSAGE_TYPE = 2;

// external plugin
static const mp_string EXTERNAL_PLUGIN_PATH = "Plugins";
#ifdef WIN32
static const mp_string EXTERNAL_PLUGIN_STOP_SCRIPT = "stop.bat";
static const mp_string EXTERNAL_PLUGIN_START_SCRIPT = "start.bat";
#else
static const mp_string EXTERNAL_PLUGIN_START_SCRIPT = "start.sh";
static const mp_string EXTERNAL_PLUGIN_STOP_SCRIPT = "stop.sh";
#endif

// base64 decode
static const mp_string BASE64_STR = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const mp_int32 BASE64_NUM6 = 6;
static const mp_int32 BASE64_NUM8 = 8;
static const mp_int32 BASE64_NUM64 = 64;
static const mp_int32 BASE64_NUM256 = 256;

// HBA state
static const mp_string HBA_STATUS_ONLINE = "27";
static const mp_string HBA_STATUS_OFFLINE = "28";

const mp_string INTERNAL_CA_FILE = "/opt/logpath/infrastructure/cert/internal/ca/ca.crt.pem";
const mp_string INTERNAL_CERT_FILE = "/opt/logpath/infrastructure/cert/internal/internal.crt.pem";
const mp_string INTERNAL_KEY_FILE = "/opt/logpath/infrastructure/cert/internal/internal.pem";
const mp_string INTERNAL_KEY_PASSWD_FILE = "/opt/logpath/infrastructure/cert/internal/internal_cert";

#ifdef REST_PUBLISH
static const mp_string AGENT_ROOT_PATH = "/home/rdadmin/Agent/";
#elif SANCLIENT_AGENT
static const mp_string AGENT_ROOT_PATH = "/opt/DataBackup/SanClient/ProtectClient-E";
#else
static const mp_string AGENT_ROOT_PATH = "/opt/DataBackup/ProtectClient/ProtectClient-E";
#endif

// Vmfs_related
const mp_string VMFS_MOUNT_PATH = "/mnt/VMFS/";

//
const mp_string INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR = "Split dir and file for instantly mount";

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
    static const mp_string PATH_SEPARATOR = "\\";
    #define IS_DIR_SEP_CHAR(x) ('\\' == (x) || '/' == (x))
    static const mp_string LIB_SUFFIX = ".dll";
    static const mp_char PATH_SEPCH = ';';
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

#define AGENT_IMPORT
#define AGENT_STDCALL
static const mp_string PATH_SEPARATOR = "/";
static const mp_string DOUBLE_SLASH = "//";
static const mp_string PARENT_PATH_SEPARATOR = "/..";
#define IS_DIR_SEP_CHAR(x) ('/' == (x))
static const mp_string LIB_SUFFIX = ".so";
static const mp_char PATH_SEPCH = ':';
#define GETCH getch()
#define ITOA(val, buf, size, radix) itoa(val, buf, radix)
#endif

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

#define IS_SPACE(x) (x == ' ' || x == '\t')
#define strempty(str) (0 == str || 0 == (*str))

#ifdef WIN32
static const mp_string MOBILITY_DEVICE_NAME = "\\\\.\\IoMirrorDev";
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
static const mp_string MOBILITY_DEVICE_NAME = "/dev/im_ctldev";
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
static const mp_string LINUX_DRIVER_CONF = "/etc/huawei/im.conf";
static const mp_string BOOT_SAVED_BITMAP = "/boot/hwsnap_bitmap";
// linux driver启动的配置文件
static const mp_string LINUX_DRIVER_BOOT_CONF = "/boot/huawei/im.conf";

#endif

// 安全函数snprintf_s需要用CHECK_FAIL进行判断，只有-1时表示出错
#define CHECK_FAIL(Call)                                                                                               \
    do {                                                                                                               \
        mp_int32 iCheckFailRet = Call;                                                                                 \
        if (MP_FAILED == iCheckFailRet) {                                                                              \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckFailRet);                                    \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    } while (0)

#define CHECK_NOT_OK(Call)                                                                                             \
    do {                                                                                                               \
        mp_int32 iCheckNotOkRet = Call;                                                                                \
        if (EOK != iCheckNotOkRet) {                                                                                   \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);                                   \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    } while (0)

#define CHECK_FAIL_NOLOG(Call)                                                                                         \
    do {                                                                                                               \
        mp_int32 iCheckFailRet = Call;                                                                                 \
        if (MP_FAILED == iCheckFailRet) {                                                                              \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    } while (0)

// 将调用返回码返回
#define CHECK_FAIL_EX(Call)                                                                                            \
    do {                                                                                                               \
        mp_int32 iCheckNotOkRet = Call;                                                                                \
        if (MP_SUCCESS != iCheckNotOkRet) {                                                                            \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);                                   \
            return iCheckNotOkRet;                                                                                     \
        }                                                                                                              \
    } while (0)

// 判断指针是否为空
#define CHECK_POINTER_NULL(Ptr)                                                                                        \
    do {                                                                                                               \
        if (Ptr == nullptr) {                                                                                          \
            COMMLOG(OS_LOG_ERROR, "%s Pointer is NULL.", #Ptr);                                                        \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    } while (0)
// 打印宏名称
#define MACR(x) #x

// 数据库冻结解冻状态，多个模块公用
enum DB_STATUS {
    DB_FREEZE = 0,
    DB_UNFREEZE,
    DB_UNKNOWN,
    DB_FREEZING,
    DB_UNFREEZING,
    DB_BUTT  // 枚举结束
};
#if defined(LINUX) || defined(WIN32)
// 定义模板实现C++11支持make_unique(C++14提供支持)
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

#endif  // _AGENT_DEFINES_H_
