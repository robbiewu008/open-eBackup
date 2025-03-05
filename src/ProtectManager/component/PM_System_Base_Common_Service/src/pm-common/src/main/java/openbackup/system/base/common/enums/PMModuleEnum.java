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
package openbackup.system.base.common.enums;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * 功能模块枚举类
 *
 */
public enum PMModuleEnum {
    /**
     * 复制模块
     */
    REPLICATION_MODULE("Replication",
        Arrays.asList("Replication_NO_RETRY_READ_TIMEOUT", "Replication_NO_RETRY_CONNECT_TIMEOUT")),

    /**
     * 归档模块
     */
    ARCHIVING_MODULE("Archiving", Collections.emptyList()),

    /**
     * 副本过期模块
     */
    COPY_EXPIRATION_MODULE("CopyExpiration", Collections.emptyList()),

    /**
     * 索引模块
     */
    INDEX_MODULE("Index", Collections.emptyList()),

    /**
     * 副本校验模块
     */
    COPY_CHECK_MODULE("CopyCheck", Collections.emptyList()),

    /**
     * VMware插件
     */
    VMWARE_PLUGIN("VmwarePlugin", Collections.singletonList("VMWARE_PLUGIN_RETRY_TIMES")),

    /**
     * 阿里云ApsaraStack插件
     */
    APSARASTACK_PLUGIN("ApsaraStackPlugin",
        Arrays.asList("APSARASTACK_PLUGIN_DOMAINNAME_MAX_LENGTH", "APSARASTACK_PLUGIN_CERT_MAX_BYTE_SIZE",
            "APSARASTACK_PLUGIN_CRL_MAX_BYTE_SIZE")),

    /**
     * CNware插件
     */
    CNWARE_PLUGIN("CnwarePlugin", Arrays.asList("CnwarePlugin_CERT_MAX_BYTE_SIZE", "CnwarePlugin_CRL_MAX_BYTE_SIZE",
        "CnwarePlugin_CNWARE_DOMAINNAME_MAX_LENGTH", "CnwarePlugin_CNWARE_MAX_COUNT")),

    /**
     * FusionCompute插件
     */
    FUSION_COMPUTE("FusionComputePlugin",
        Arrays.asList("FusionCompute_CERT_MAX_BYTE_SIZE", "FusionCompute_CRL_MAX_BYTE_SIZE",
            "FusionCompute_STORAGE_MAX_COUNT")),

    /**
     * HCS华为云插件
     */
    HCS_PLUGIN("HCSPlugin", Arrays.asList("HCSPlugin_STORAGE_MAX_COUNT", "HCSPlugin_PARAM_MAX_LENGTH",
        "HCSPlugin_MAX_SERVERS_SINGLE_PROJECT")),

    /**
     * Hyper-V插件
     */
    HYPER_V_PLUGIN("HypervPlugin", Arrays.asList("HypervPlugin_MAX_NAME_LENGTH", "HypervPlugin_SCVMM_UPPER_LIMIT",
        "HypervPlugin_CLUSTER_UPPER_LIMIT", "HypervPlugin_HOST_UPPER_LIMIT")),

    /**
     * k8s csi插件
     */
    K8S_CSI_PLUGIN("K8sCsiPlugin",
        Arrays.asList("K8sCsiPlugin_DATASET_MAX_COUNT_IN_NAMESPACE", "K8sCsiPlugin_ADVANCED_CONFIG_MAX_SIZE",
            "K8sCsiPlugin_ADVANCED_CONFIG_PARAM_MAX_SIZE", "K8sCsiPlugin_K8S_CLUSTER_MAX_COUNT",
            "K8sCsiPlugin_MAX_JOB_NUMBER")),

    /**
     * k8s flexvolumn插件
     */
    K8S_FLEXVOLUME_PLUGIN("K8sFlexVolumePlugin", Arrays.asList("K8sFlexVolumePlugin_K8S_STORAGE_MAX_COUNT")),

    /**
     * Nutanix插件
     */
    NUTANIX_PLUGIN("NutanixPlugin", Arrays.asList("NutanixPlugin_CERT_MAX_BYTE_SIZE", "NutanixPlugin_CRL_MAX_BYTE_SIZE",
        "NutanixPlugin_DOMAIN_NAME_MAX_LENGTH", "NutanixPlugin_MAX_ENV_COUNT")),

    /**
     * OpenStack插件
     */
    OPENSTACK_PLUGIN("OpenStackPlugin",
        Arrays.asList("OpenStackPlugin_MAX_WAIT_TIME", "OpenStackPlugin_MAX_OPENSTACK_COUNT",
            "OpenStackPlugin_MAX_DOMAIN_COUNT", "OpenStackPlugin_CONNECT_TIMEOUT", "OpenStackPlugin_READ_TIMEOUT",
            "OpenStackPlugin_CALL_TIMEOUT")),

    /**
     * 任务模块
     */
    TASK_MODULE("Task",
        Arrays.asList("Task_RUNNING_JOB_LIMIT_COUNT_ONE_NODE", "Task_ANTI_RANSOMWARE_JOB_LIMIT_COUNT_ONE_NODE",
            "Task_backup_job_limit_count", "Task_archive_job_limit_count", "Task_restore_job_limit_count",
            "Task_exercise_job_limit_count", "Task_copy_delete_job_limit_count", "Task_copy_expire_job_limit_count",
            "Task_copy_verify_job_limit_count", "Task_copy_import_job_limit_count",
            "Task_desensitization_import_job_limit_count", "Task_identification_import_job_limit_count",
            "Task_live_mount_job_limit_count", "Task_cancel_live_mount_job_limit_count",
            "Task_agent_install_job_limit_count", "Task_agent_update_job_limit_count",
            "Task_agent_change_app_update_job_limit_count", "Task_resource_scan_job_limit_count",
            "Task_resource_rescan_job_limit_count", "Task_resource_protect_job_limit_count",
            "Task_modify_protect_job_limit_count")),

    /**
     * UBC模块
     */
    UBC_MODULE("UBC",
        Arrays.asList("UBC_ACTIVE_THREAD_POOL_NUM", "UBC_AGENT_HEARTBEAT_TIMEOUT", "UBC_ANNOTATIONS_CONFIG_PATH",
            "UBC_BACKUP_KS_PATH", "UBC_BIND_AGENT_POOL_NUM", "UBC_CACHE_REPOSITORY_DIST_ALG",
            "UBC_CLEAN_THREAD_POOL_NUM", "UBC_DATA_REPOSITORY_DIST_ALG", "UBC_DATABASE_MAX_OVERFLOW",
            "UBC_DATABASE_POOL_PRE_PING", "UBC_DATABASE_POOL_RECYCLE", "UBC_DATABASE_POOL_SIZE",
            "UBC_DATABASE_POOL_TIMEOUT", "UBC_DB_RETRY_INTERVAL", "UBC_DB_RETRY_TIMES", "UBC_DEE_INDEXER_PORT",
            "UBC_DME_ARCHIVE_HOST", "UBC_DME_ARCHIVE_SRV_PORT", "UBC_EXTERNAL_CA_DIR", "UBC_EXTERNAL_CERT_DIR",
            "UBC_EXTERNAL_CNF_DIR", "UBC_EXTERNAL_KEY_DIR", "UBC_FINISHED_TASK_EXPIRE_PERCENT",
            "UBC_FINISHED_TASK_MAX_KEEP_QUANTITY", "UBC_GLOBAL_THREAD_POOL_NUM", "UBC_HTTP_TIME_OUT",
            "UBC_INFRA_DB_HOST", "UBC_INFRA_DB_PORT", "UBC_INFRA_HOST", "UBC_INFRA_HTTP_PORT",
            "UBC_INFRA_HTTP_RETRY_INTERVAL", "UBC_INFRA_HTTP_RETRY_TIMES", "UBC_INFRA_RETRY_INTERVAL",
            "UBC_INFRA_RETRY_INTERVAL_FOR_NETWORK_ERROR", "UBC_INFRA_RETRY_TIMES",
            "UBC_INFRA_RETRY_TIMES_FOR_NETWORK_ERROR", "UBC_INTERNAL_CA_DIR", "UBC_INTERNAL_CERT_DIR",
            "UBC_INTERNAL_CNF_DIR", "UBC_INTERNAL_KEY_DIR", "UBC_LIBKMCV3_SO_PATH", "UBC_LOG_REPOSITORY_DIST_ALG",
            "UBC_MAIN_TASK_EXPIRE_INTERVAL", "UBC_MAIN_TASK_SCHEDULE_TIMEOUT", "UBC_MASTER_KS_PATH",
            "UBC_MAX_CACHE_CONNECTION", "UBC_MAX_HTTP_RETRIES")),

    /**
     * 报表模块
     */
    REPORT_MODULE("Report",
        Arrays.asList("Report_pm-resource-manager.url", "Report_CORE_POOL_SIZE", "Report_MAXIMUM_POOL_SIZE",
            "Report_KEEP_ALIVE_TIME", "Report_WORK_QUEUE_SIZE", "Report_MAX_RETRY_TIMES",
            "Report_MAX_REPORT_FILE_COUNT", "Report_MAX_EMAIL_SIZE", "Report_MAX_REPORT_FILE_SIZE",
            "Report_TIME_INTERVAL", "Report_CREATE_TIMEOUT_INTERVAL", "Report_REPORT_FOLDER_LOCK_WAIT_TIME")),

    /**
     * 标签模块
     */
    TAG_MODULE("Tag", Arrays.asList("Tag_MAXIMUM_LABEL_SIZE", "Tag_MAXIMUM_LABEL_RESOURCE_SIZE")),

    /**
     * 日志导出模块
     */
    LOG_EXPORT_MODULE("LogExport",
        Arrays.asList("LogExport_NUMBER_OF_EXPORT_FILE_STATUS", "LogExport_NUMBER_OF_EXPORT_FILE_TYPE",
            "LogExport_MAX_LENGTH_OF_EXPORT_FILE_ID", "LogExport_INITIAL_DELAY", "LogExport_FIXED_RATE",
            "LogExport_TIME_OUT_LOOP_TIME", "LogExport_MAX_LENGTH_OF_FILE_NAME", "LogExport_MAX_LENGTH_OF_NODE_NAME",
            "LogExport_TWO_HOURS", "LogExport_EXPIRE_TIME", "LogExport_PER_CONTROLLER_DOWNLOAD_THREAD_NUM",
            "LogExport_REDIS_LOCK_TIME", "LogExport_REDIS_LOCK_TIMEOUT", "LogExport_MAX_AGENT_LOG_EXPORT_NUM",
            "LogExport_exportfile.instances")),

    /**
     * 集群模块
     */
    CLUSTER_MODULE("Cluster",
        Arrays.asList("Cluster_FIXED_RATE", "Cluster_HEARTBEAT_FIXED_RATE", "Cluster_CLEAR_FIXED_RATE",
            "Cluster_STATUS_FIXED_RATE", "Cluster_OFF_LINE_TIME", "Cluster_UPDATE_MEMBER_DEVICE_SECRET_RATE",
            "Cluster_FIXED_DELAY", "Cluster_FIXED_CLEAN_RATE", "Cluster_HEARTBEAT_LOSS_SECOND",
            "Cluster_HEARTBEAT_CHECK_INTERVAL", "Cluster_CLUSTER_STATUS_CACHE_EXPIRE_TIME", "Cluster_INITIAL_DELAY")),

    /**
     * agent模块
     */
    AGENT_MODULE("Agent",
        Arrays.asList("Agent_REGISTER_EXPIRE", "Agent_REGISTER_MAX_NUM", "Agent_REGISTER_CHECK_MAX_NUMS",
            "Agent_MAX_AGENT_DIR_NUM", "Agent_FIFTEEN_MINUTES", "Agent_LOG_COLLECT_TIME_OUT",
            "Agent_LOG_STATUS_SLEEP_TIME", "Agent_LOG_MAX_SIZE", "Agent_LOG_TIMEOUT", "Agent_MAX_FILE_NAME_LENGTH",
            "Agent_agentDealExpireTime", "Agent_TWENTY_MINUTES", "Agent_CORE_POOL_SIZE", "Agent_MAXIMUM_POOL_SIZE",
            "Agent_KEEP_ALIVE_TIME", "Agent_WORK_QUEUE_SIZE", "Agent_SOCKET_TIMEOUT", "Agent_RETRY_TIME",
            "Agent_SESSION_TIMEOUT")),

    /**
     * 告警模块
     */
    ALARM_MODULE("Alarm",
        Arrays.asList("Alarm_FIXED_RATE", "Alarm_FAIL_LIMIT", "Alarm_SKIP_EXEC_COUNT", "Alarm_VALUE_1000",
            "Alarm_VALUE_1000000", "Alarm_MAX_NUM", "Alarm_LOOP_TIME", "Alarm_TRY_LOCK_TIME",
            "Alarm_MIN_LENGTH_OF_SECURITY_NAME_V2C", "Alarm_MAX_LENGTH_OF_USERNAME", "Alarm_MAX_LENGTH_OF_PASSWORD",
            "Alarm_MAX_LENGTH_OF_UPLOAD_PATH", "Alarm_DEFAULT_RESER_DATE")),

    /**
     * 初始化模块
     */
    INIT_MODULE("Initialization", Arrays.asList("Initialization_INIT_OVER_TIME", "Initialization_MODIFY_OVER_TIME",
        "Initialization_FAILURE_THRESHOLD", "Initialization_PERIOD", "Initialization_MAX_RETRY_COUNT",
        "Initialization_DEFAULT_WAIT_SECOND")),

    /**
     * SFTP模块
     */
    SFTP_MODULE("SFTP",
        Arrays.asList("SFTP_USER_COUNT_LIMIT", "SFTP_SFTP_SWITCH_WAIT_TIME", "SFTP_SFTP_LOCK_WAIT_TIMES",
            "SFTP_SFTP_PWD_MAX_LENGTH", "SFTP_SFTP_PWD_MIN_LENGTH", "SFTP_RETRY_DELAY_TIME", "SFTP_RETRY_NUMBER_TIME",
            "SFTP_MAX_RETRY_COUNT", "SFTP_FIRST_WAIT_TIME", "SFTP_AVERAGE_WAITING_TIME", "SFTP_KEEP_ALIVE",
            "SFTP_AWAIT_TERMINATION")),

    /**
     * 管理数据备份模块
     */
    DATA_BACKUP_MANAGEMENT_MODULE("DataBackupManagement",
        Arrays.asList("DataBackupManagement_RETRY_TIME_INTERVAL", "DataBackupManagement_MAX_FILE_LENGTH",
            "DataBackupManagement_INPUT_BACK_UP_FILE_SIZE"));

    private final String moduleName;

    private final List<String> relatedKeys;

    PMModuleEnum(String moduleName, List<String> relatedKeys) {
        this.moduleName = moduleName;
        this.relatedKeys = relatedKeys;
    }

    public String getModuleName() {
        return moduleName;
    }

    public List<String> getRelatedKeys() {
        return relatedKeys;
    }

    /**
     * 获取所有模块名称的集合
     *
     * @return 获取所有模块类型的集合
     */
    public static Set<String> getAllModuleNames() {
        Set<String> moduleNames = new HashSet<>();
        for (PMModuleEnum module : values()) {
            moduleNames.add(module.getModuleName());
        }
        return moduleNames;
    }

    /**
     * 获取所有相关的key集合（去重）
     *
     * @return 获取所有模块白名单的集合
     */
    public static Set<String> getAllRelatedKeys() {
        Set<String> relatedKeysSet = new HashSet<>();
        for (PMModuleEnum module : values()) {
            relatedKeysSet.addAll(module.getRelatedKeys());
        }
        return relatedKeysSet;
    }
}


