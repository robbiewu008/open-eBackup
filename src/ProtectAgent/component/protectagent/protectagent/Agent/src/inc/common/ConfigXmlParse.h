#ifndef __AGENT_CONFIG_XML_PARSE_H__
#define __AGENT_CONFIG_XML_PARSE_H__

#include <map>
#include <vector>
#include "tinyxml2.h"
#include "common/Types.h"
#include "common/CMpThread.h"

// config items from agent.cfg.xml
static const mp_string CFG_SYSTEM_SECTION = "System";
static const mp_string CFG_DRM_IP = "drm_ip";
static const mp_string CFG_DRM_PORT = "drm_port";
static const mp_string CFG_DRM_PERIOD = "drm_period";
static const mp_string CFG_AGENT_IP = "agent_ip";
static const mp_string CFG_PORT = "port";
static const mp_string CFG_LOG_LEVEL = "log_level";
static const mp_string CFG_LOG_COUNT = "log_count";
static const mp_string CFG_LOG_MAX_SIZE = "log_max_size";
static const mp_string CFG_LOG_KEEP_TIME = "log_keep_time";
static const mp_string CFG_LOG_CACHE_THRESHOLD = "log_cache_threshold";
static const mp_string CFG_USER_NAME = "name";
static const mp_string CFG_SALT_VALUE = "sl";
static const mp_string CFG_HASH_VALUE = "hash";
static const mp_string CFG_WIN_VERISON_VALUE = "win_version";
static const mp_string CFG_WIN_SYSTEM_DISK_VALUE = "win_system_disk";
static const mp_string CFG_DOMAIN_NAME_VALUE   = "domain_name";
static const mp_string CFG_DOMAIN_NAME_VALUE_DME   = "domain_name_dme";
static const mp_string CFG_SECURE_CHANNEL      = "secure_channel";
static const mp_string CFG_ALGORITHM_SUITE     = "algorithm_suite";
static const mp_string CFG_CLIENT_VERSION = "client_version";
static const mp_string CFG_VERSION_TIME_STAMP = "version_time_stamp";
static const mp_string CFG_CURL_CONNECTIVITY_INTERVAL = "curl_connectivity_interval";
static const mp_string CFG_CURL_CONNECTIVITY_TIMEOUT = "curl_connectivity_timeout";
static const mp_string CFG_HEARTBEAT_TO_PM_INTERVAL = "heartbeat_to_pm_interval";
static const mp_string CFG_RESOURCE_USAGE_TO_PM_INTERVAL = "resourse_usage_to_pm_interval";
static const mp_string CFG_IS_AUTO_SYNCHRONIZE_HOST_NAME = "is_auto_synchronize_host_name";
static const mp_string CFG_RESOURCE_REST_PRINT = "resourse_rest_print";
static const mp_string CFG_MONITOR_SECTION = "Monitor";
static const mp_string CFG_RETRY_TIME = "retry_time";
static const mp_string CFG_MONITOR_INTERVAL = "monitor_interval";
static const mp_string CFG_RDAGENT_SECTION = "rdagent";
static const mp_string CFG_NGINX_SECTION = "nginx";
static const mp_string CFG_RESOURCE_SECTION = "resource";
static const mp_string CFG_THREAD_COUNT = "monitor_interval";
static const mp_string CFG_HANDLE_COUNT = "handle_count";
static const mp_string CFG_PM_SIZE = "pm_size";
static const mp_string CFG_VM_SIZE = "vm_size";
static const mp_string CFG_CPU_USAGE = "cpu_usage";
static const mp_string CFG_TMP_FILE_SIZE = "tmpfile_size";
static const mp_string CFG_SNMP_SECTION = "SNMP";
static const mp_string CFG_PRIVATE_PASSWOD = "private_password";
static const mp_string CFG_PRIVATE_PROTOCOL = "private_protocol";
static const mp_string CFG_AUTH_PASSWORD = "auth_password";
static const mp_string CFG_AUTH_PROTOCOL = "auth_protocol";
static const mp_string CFG_SECURITY_NAME = "security_name";
static const mp_string CFG_CONTEXT_NAME = "context_name";
static const mp_string CFG_ENGINE_ID = "engine_id";
static const mp_string CFG_SECURITY_LEVEL = "security_level";
static const mp_string CFG_SECURITY_MODEL = "security_model";
static const mp_string CFG_FREEZEINTERVALTIME = "freeze_interval_time";
static const mp_string CFG_AUTH_TYPE = "auth_type";
static const mp_string CFG_SIGN_IGNORE = "sign_ignore";
static const mp_string CFG_BACKUP_SECTION = "Backup";
static const mp_string CFG_LINK_ADMINNODE_NIC = "link_ebk_server_nic";
static const mp_string CFG_IF_KEEP_VMSNAP_ON_FAIL = "if_keep_vmsnap_on_fail";
static const mp_string CFG_ADMINNODE_IP = "ebk_server_ip";
static const mp_string CFG_IAM_PORT = "ebk_server_auth_port";
static const mp_string CFG_ADMINNODE_PORT = "ebk_server_port";
static const mp_string CFG_ADMINNODE_USER = "ebk_server_user";
static const mp_string CFG_ADMINNODE_PWD = "ebk_server_pwd";
static const mp_string CFG_BACKUP_ROLE = "backup_role";
static const mp_string CFG_CHECK_CONN_TIME = "check_conn_time";
static const mp_string CFG_CHECK_CONN_THREAD_NUM = "check_conn_thread_num";
static const mp_string CFG_CHECK_VSPHERE_CONN_TIME = "check_vsphere_conn_time";
static const mp_uchar CFG_DEFAULT_SECURITY_LEVEL = 3;
static const mp_string CFG_PROGRESS_INTERVAL   = "progress_interval";
static const mp_string CFG_ARCHIVE_THRESHOLD = "archive_threshold";
static const mp_string CFG_ARCHIVE_THREAD_TIMEOUT = "archive_thread_timeout";
static const mp_string CFG_MAX_CHUNK_NUMBER = "max_chunk_number";
static const mp_string CFG_ALLOCATED_BLOCK_CHUNKSIZE = "allocated_block_chunksize";
static const mp_string CFG_DATAPROCESS_SECTION = "DataProcess";
static const mp_string CFG_CHUNK_SIZE = "chunkSize";
static const mp_string CFG_IS_ENABLE_SSL = "isEnableSSL";
static const mp_string CFG_IS_ENABLE_ADVANCED = "isEnableAdvanced";
static const mp_string CFG_BACKUP_SEGMENT_SIZE = "backup_segment_block_size";
static const mp_string CFG_RESTORE_SEGMENT_SIZE = "restore_segment_block_size";
static const mp_string CFG_TASKPOOL_WORKER_COUNT = "taskpoll_worker_count";
static const mp_string CFG_DEE_GID = "dee_gid";
static const mp_string CFG_THREAD_TIMEOUT = "thread_timeout";
static const mp_string CFG_THREAD_SLEEP_MILLISECONDS = "thread_sleep_milliseconds";
static const mp_string CFG_VDDKAPI_TIMEOUT = "vddkapi_timeout";
static const mp_string CFG_CHECK_CERT_CYCLE = "check_cert_cycle";
static const mp_string CFG_WARING_TIME = "warning_time";
static const mp_string CFG_IS_DPC_COMPUTE_NODE = "is_dpc_compute_node";

// section Security
static const mp_string CFG_SECURITY_SECTION = "Security";
static const mp_string CFG_ALARM_ID = "alarm_id";
static const mp_string CFG_PM_CA_INFO = "pm_ca_info";
static const mp_string CFG_AGENT_CA_INFO = "agent_ca_info";
static const mp_string CFG_SSL_CERT = "ssl_cert";
static const mp_string CFG_SSL_KEY = "ssl_key";
static const mp_string CFG_SSL_CRL = "ssl_crl";
static const mp_string CFG_CERT = "cert";
static const mp_string CFG_DISABLE_SAFE_RANDOM_NUMBER = "disable_safe_random_number";

static const mp_string CFG_TASK_EXPIRE_DAYS = "task_expire_days";
static const mp_string CFG_API_INVOKING_TIME_INTERVAL = "api_invoking_time_interval";
static const mp_string CFG_HOSTAGENT_SYSTEM_VIRT = "system_virt";
static const mp_string CFG_PROGRESS_QUERY_TIMEINTERVAL = "progress_query_time_interval";
static const mp_string CFG_DWS_TMP_FILE_MAX_SIZE = "dws_tmp_file_max_size";
static const mp_string CFG_DWS_HOST_MAX_NUM = "dws_host_max_num";
static const mp_string CFG_XBSA_SESSION_TIMEOUT_TIME = "xbsa_session_timeout_time";
static const mp_string CFG_DWS_CREATE_SNAPSHOT_TIMEOUT = "create_snapshot_timeout";
static const mp_string CFG_DWS_MAX_OBJS_FOR_SNAPSHOT = "max_objs_for_snapshot";
static const mp_string CFG_XBSA_PROVIDER = "xbsa_provider";
static const mp_string CFG_THRIFT_SECTION = "Thrift";
static const mp_string CFG_THRIFT_SERVER_PORT = "thrift_server_port";
static const mp_string CFG_THRIFT_MAX_THREAD_SIZE = "thrift_max_thread_size";
static const mp_string CFG_THRIFT_CLIENT_SEND_TIMEOUT = "thrift_clent_send_timeout";
static const mp_string CFG_THRIFT_PLUGIN_HEARTBEAT_TIMEOUT = "thrift_plugin_heartbeat_timeout";
static const mp_string CFG_FRAME_THRIFT_SECTION = "Frame";
static const mp_string CFG_FRAME_THRIFT_SERVER_START_PORT = "thrift_server_start_port";
static const mp_string CFG_FRAME_THRIFT_SERVER_END_PORT = "thrift_server_end_port";
static const mp_string CFG_PLUGIN_START_PORT = "external_plugin_start_port";
static const mp_string CFG_PLUGIN_END_PORT = "external_plugin_end_port";
static const mp_string MAIN_JOB_CNT_MAX = "main_job_cnt_max";
static const mp_string MAX_PLUGIN_CONNECTION_COUNT = "max_plugin_connection_count";
static const mp_string THRIFT_TIME_OUT = "thrift_time_out";
static const mp_string SUB_JOB_CNT_MAX = "sub_job_cnt_max";
static const mp_string MAIN_JOB_CNT_MAX_INNER = "main_job_cnt_max_inner";
static const mp_string SUB_JOB_CNT_MAX_INNER = "sub_job_cnt_max_inner";
static const mp_string CFG_MOUNT_SECTION = "Mount";
static const mp_string CFG_LINK_HOST_PORT = "link_host_port";
static const mp_string CFG_CHECK_LINK_STATUS_TIMEOUT = "check_link_status_timeout";
static const mp_string CFG_EIP = "eip";
static const mp_string CFG_WIN_MOUNT_PUBLIC_PATH = "win_mount_public_path";
static const mp_string CFG_GENERAL_MOUNT_OPTION = "general_mount_option";
static const mp_string CFG_LINKENCRY_MOUNT_OPTION = "linkencyption_mount_option";
static const mp_string CFG_GENERAL_MOUNT_PROTOCOL = "general_mount_protocol";
static const mp_string CFG_AIX_GENERAL_MOUNT_OPTION = "aix_general_mount_option";
static const mp_string CFG_HOST_STORAGE_MAP = "agent_storage_relation";
static const mp_string CFG_CLEAR_MOUNT_POINTS_SECTION = "ClearMountPoints";
static const mp_string CFG_CLEAR_LOOP_INTERVAL = "job_execute_interval";
static const mp_string CFG_MOUNT_POINT_CHANGE_TIMEOUT = "mount_point_change_timeout";
static const mp_string CFG_READ_FILE_SIZE = "read_file_size";
static const mp_string CFG_READ_FILE_RESPONSE_SIZE = "read_file_response_size";
static const mp_string CFG_DEPLOY_TYPE = "deploy_type";
static const mp_string CFG_BACKUP_SCENE = "backup_scene";
static const mp_string CFG_SOLARIS_GENERAL_MOUNT_OPTION = "solaris_general_mount_option";
static const mp_string CFG_BACKUP_ESN = "backup_esn";
static const mp_string CFG_INNER_ESN = "inner_esn";              // config items from xbsa.xml
static const mp_string CFG_OPENSTACK_METADATA_SERVER_IP = "openstack_metadata_server_ip";
static const mp_string CFG_OPENSTACK_METADATA_SERVER_PORT = "openstack_metadata_server_port";
static const mp_string CFG_DATAPROCESS_USE_AIO_BACKUP = "use_aio_backup";
static const mp_string CFG_DATAPROCESS_USE_AIO_RESTORE = "use_aio_restore";

// job frame
static const mp_string CFG_JOB_FRAM_SECTION = "JobFrame";
static const mp_string CFG_GET_JOB_BASE_INTERVAL = "get_job_base_interval";
static const mp_string CFG_GET_JOB_INC_INTERVAL = "get_job_inc_interval";
static const mp_string CFG_GET_JOB_INTERVAL_MAX_ADJUST_TIMES = "get_job_interval_max_adjust_times";

static const mp_string XBSACFG_APP_TYPE_SECTION = "Apptype";
static const mp_string XBSACFG_APP_TYPE = "app_type";
static const mp_string XBSACFG_STORAGE_TYPE_SECTION = "StorageType";
static const mp_string XBSACFG_LOG = "log";
static const mp_string XBSACFG_LOG_PATH = "log_path";
static const mp_string XBSACFG_DATA = "data";
static const mp_string XBSACFG_DATA_PATH = "data_path";

typedef std::map<mp_string, mp_string> NodeValue;

class AGENT_API CConfigXmlParser {
public:
    static CConfigXmlParser& GetInstance();

    ~CConfigXmlParser()
    {
        CMpThread::DestroyLock(&m_cfgValueMutexLock);
        if (m_pDoc != NULL) {
            delete m_pDoc;
            m_pDoc = NULL;
        }
    }

    mp_int32 Init(const mp_string& strCfgFilePath);
    mp_bool IsModified();
    mp_int32 GetValueString(const mp_string& strSection, const mp_string& strKey, mp_string& strValue);
    mp_int32 GetValueBool(const mp_string& strSection, const mp_string& strKey, mp_bool& bValue);
    mp_int32 GetValueInt32(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue);
    mp_int32 GetValueInt64(const mp_string& strSection, const mp_string& strKey, mp_int64& lValue);
    mp_int32 GetValueUint64(const mp_string& strSection, const mp_string& strKey, mp_uint64& lValue);
    mp_int32 GetValueFloat(const mp_string& strSection, const mp_string& strKey, mp_float& fValue);
    mp_int32 GetValueString(
        const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
        mp_string& strValue);
    mp_int32 GetValueBool(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
        mp_bool& bValue);
    mp_int32 GetValueInt32(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
        mp_int32& iValue);
    mp_int32 GetValueInt64(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
        mp_int64& lValue);
    mp_int32 GetValueFloat(const mp_string& strParentSection, const mp_string& strChildSection,
        const mp_string& strKey, mp_float& fValue);

    mp_int32 ModifyConfigFile(const mp_string& tmpFile);
    mp_int32 SetValue(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue);
    mp_int32 SetValue(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
        const mp_string &strValue);

private:
    inline mp_bool IsInited();

    CConfigXmlParser()
    {
        CMpThread::InitLock(&m_cfgValueMutexLock);
        m_pDoc = NULL;
        m_lastTime = 0;
    }
    mp_int32 Load();
    mp_void ParseNodeValue(tinyxml2::XMLElement* pCfgSec, NodeValue& nodeValue);
    tinyxml2::XMLElement* GetChildElement(tinyxml2::XMLElement* pParentElement, const mp_string& strSection);
    mp_int32 UpdateModifyTime();

private:
    static CConfigXmlParser m_instance;
    mp_string m_strCfgFilePath;
    mp_time m_lastTime;
    thread_lock_t m_cfgValueMutexLock;
    tinyxml2::XMLDocument* m_pDoc;
};

#endif  // __AGENT_CONFIG_XML_PARSE_H__
