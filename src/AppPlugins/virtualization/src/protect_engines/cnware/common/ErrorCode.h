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
#ifndef CNWARE_ERRORCODE_H
#define CNWARE_ERRORCODE_H

namespace CNwarePlugin {

namespace CNwareErrorCode {

/* cnware restore label */
const std::string CNWARE_RESTORE_PRECHECK_LABEL = "virtual_plugin_cnware_restore_precheck_label";
const std::string CNWARE_VMNAME_CHECK_LABEL = "virtual_plugin_cnware_vmname_check_label";
const std::string CNWARE_VMNAME_NOTUNIQUE_LABEL = "virtual_plugin_cnware_vmname_notunique_label";
const std::string CNWARE_ARCH_CHECK_LABEL = "virtual_plugin_cnware_arch_check_label";
const std::string CNWARE_ARCH_CHECK_FAILED_LABEL = "virtual_plugin_cnware_arch_check_failed_label";
const std::string CNWARE_STORAGEPOOL_CHECK_LABEL = "virtual_plugin_cnware_storagepool_check_label";
const std::string CNWARE_STORAGEPOOL_NOTAVAILABLE_LABEL = "virtual_plugin_cnware_storagepool_notavailable_label";
const std::string CNWARE_STORAGEPOOL_FAILED_LABEL = "virtual_plugin_cnware_storagepool_failed_label";
const std::string CNWARE_CREATE_MACHINE_LABEL = "virtual_plugin_cnware_create_machine_label";
const std::string CNWARE_CREATE_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_create_machine_failed_label";
const std::string CNWARE_POWEROFF_MACHINE_LABEL = "virtual_plugin_cnware_poweroff_machine_label";
const std::string CNWARE_POWEROFF_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_poweroff_machine_failed_label";
const std::string CNWARE_POWERON_MACHINE_LABEL = "virtual_plugin_cnware_poweron_machine_label";
const std::string CNWARE_POWERON_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_poweron_machine_failed_label";
const std::string CNWARE_DELETE_MACHINE_LABEL = "virtual_plugin_cnware_delete_machine_label";
const std::string CNWARE_DELETE_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_delete_machine_failed_label";
const std::string CNWARE_CREATE_VOLUME_LABEL = "virtual_plugin_cnware_create_volume_label";
const std::string CNWARE_CREATE_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_create_volume_failed_label";
const std::string CNWARE_DETACH_VOLUME_LABEL = "virtual_plugin_cnware_detach_volume_label";
const std::string CNWARE_DETACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_detach_volume_failed_label";
const std::string CNWARE_ATTACH_VOLUME_LABEL = "virtual_plugin_cnware_attach_volume_label";
const std::string CNWARE_ATTACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_attach_volume_failed_label";
const std::string CNWARE_NOT_SUPPORT_BLOCK_LUN_LABEL = "virtual_plugin_cnware_unsupport_blocklun_label";
const std::string CNWARE_VM_ABNORMAL_LABEL = "virtual_plugin_cnware_vm_abnormal_label";
const std::string CNWARE_VM_MIGRATE_LABEL = "virtual_plugin_cnware_vm_migrate_label";
const std::string CNWARE_VM_MIGRATE_FAILED_LABEL = "virtual_plugin_cnware_vm_migrate_failed_label";
const std::string CNWARE_ADD_STORAGE_LABEL = "virtual_plugin_cnware_add_storage_label";
const std::string CNWARE_ADD_STORAGE_FAILED_LABEL = "virtual_plugin_cnware_add_storage_failed_label";
const std::string CNWARE_SCAN_STORAGE_FAILED_LABEL = "virtual_plugin_cnware_scan_storage_failed_label";
const std::string CNWARE_CREATE_STORAGE_POOL_LABEL = "virtual_plugin_cnware_create_storage_pool_label";
const std::string CNWARE_CREATE_STORAGE_POOL_FAILED_LABEL = "virtual_plugin_cnware_create_storage_pool_failed_label";
const std::string CNWARE_REFRESH_STORAGE_POOL_FAILED_LABEL = "virtual_plugin_cnware_refresh_storage_pool_failed_label";
const std::string CNWARE_BACKUP_CHECK_LABEL = "virtual_plugin_cnware_backup_check_label";
const std::string CNWARE_HOST_CHECK_FAILED_LABEL = "virtual_plugin_cnware_host_check_failed_label";
const std::string CNWARE_HOST_CPU_LIMIT_LABEL = "virtual_plugin_cnware_host_cpu_limit_label";
const std::string CNWARE_CHECK_CPU_USAGE_FAILED_LABEL = "virtual_plugin_cnware_cpu_usage_limit_label";
const std::string CNWARE_CHECK_MEMORY_USAGE_FAILED_LABEL = "virtual_plugin_cnware_memory_usage_limit_label";
const std::string CNWARE_CHECK_STORAGE_USAGE_FAILED_LABEL = "virtual_plugin_cnware_storage_usage_limit_label";
const std::string CNWARE_NAS_PREALLOCATION_WARNING_LABEL = "virtual_plugin_cnware_nas_preallocation_warning_label";
const std::string CNWARE_VOLUME_STATUS_UNNORMAL_LABEL = "virtual_plugin_cnware_volume_status_unnormal_label";
const std::string CNWARE_STORAGE_STATUS_UNNORMAL_LABEL = "virtual_plugin_cnware_storage_status_unnormal_label";
const std::string CNWARE_RESOURCE_NOT_FIND_LABEL = "virtual_plugin_cnware_resource_not_find_label";

/* cnware restore error code */
constexpr int64_t CNWARE_VMNAME_CHECK_ERROR = 0x5E025E75;
constexpr int64_t CNWARE_ARCH_CHECK_FAILED_ERROR = 0x5E025E76;
constexpr int64_t CNWARE_STORAGEPOOL_NOTAVAILABLE_ERROR = 0x5E025E77;
constexpr int64_t CNWARE_CREATE_MACHINE_FAILED_ERROR = 0x5E025E78;
constexpr int64_t CNWARE_POWEROFF_MACHINE_FAILED_ERROR = 0x5E025E79;
constexpr int64_t CNWARE_POWERON_MACHINE_FAILED_ERROR = 0x5E025E7A;
constexpr int64_t CNWARE_CREATE_VOLUME_FAILED_ERROR = 0x5E025E7B;
constexpr int64_t CNWARE_DETACH_VOLUME_FAILED_ERROR = 0x5E025E7C;
constexpr int64_t CNWARE_ATTACH_VOLUME_FAILED_ERROR = 0x5E025E7D;

constexpr int64_t CNWARE_AUTH_FAILED = 1577213541;
constexpr int64_t CNWARE_ERR_PARAM = 1677929218;
constexpr int64_t CNWARE_CONNECT_FAILED = 1677931275;
constexpr int64_t CNWARE_CERT_FAILED = 1577213574;
constexpr int64_t CNWARE_DISK_NOT_FOUND = 1677931451;
constexpr int64_t ALLOW_COMMON_ERROR = 1677929228;
};
}
#endif // CNWARE_ERRORCODE_H