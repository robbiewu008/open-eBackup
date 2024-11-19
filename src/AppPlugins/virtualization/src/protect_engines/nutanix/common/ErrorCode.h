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
#ifndef NUTANIX_ERRORCODE_H
#define NUTANIX_ERRORCODE_H

namespace NutanixPlugin {

namespace NutanixErrorCode {

/* cnware restore label */
const std::string NUTANIX_RESTORE_PRECHECK_LABEL = "virtual_plugin_nutanix_restore_precheck_label";
const std::string NUTANIX_VMNAME_CHECK_LABEL = "virtual_plugin_nutanix_vmname_check_label";
const std::string NUTANIX_VMNAME_NOTUNIQUE_LABEL = "virtual_plugin_nutanix_vmname_notunique_label";
const std::string NUTANIX_ARCH_CHECK_FAILED_LABEL = "virtual_plugin_nutanix_arch_check_failed_label";
const std::string NUTANIX_STORAGEPOOL_CHECK_LABEL = "virtual_plugin_nutanix_storagepool_check_label";
const std::string NUTANIX_STORAGEPOOL_NOTAVAILABLE_LABEL = "virtual_plugin_nutanix_storagepool_notavailable_label";
const std::string CNWARE_STORAGEPOOL_FAILED_LABEL = "virtual_plugin_cnware_storagepool_failed_label";
const std::string NUTANIX_CREATE_MACHINE_LABEL = "virtual_plugin_nutanix_create_machine_label";
const std::string NUTANIX_CREATE_MACHINE_FAILED_LABEL = "virtual_plugin_nutanix_create_machine_failed_label";
const std::string NUTANIX_POWEROFF_MACHINE_LABEL = "virtual_plugin_nutanix_poweroff_machine_label";
const std::string NUTANIX_POWEROFF_MACHINE_FAILED_LABEL = "virtual_plugin_nutanix_poweroff_machine_failed_label";
const std::string NUTANIX_POWERON_MACHINE_LABEL = "virtual_plugin_nutanix_poweron_machine_label";
const std::string NUTANIX_POWERON_MACHINE_FAILED_LABEL = "virtual_plugin_nutanix_poweron_machine_failed_label";
const std::string CNWARE_DELETE_MACHINE_LABEL = "virtual_plugin_cnware_delete_machine_label";
const std::string CNWARE_DELETE_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_delete_machine_failed_label";
const std::string CNWARE_DETACH_VOLUME_LABEL = "virtual_plugin_cnware_detach_volume_label";
const std::string CNWARE_DETACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_detach_volume_failed_label";
const std::string CNWARE_ATTACH_VOLUME_LABEL = "virtual_plugin_cnware_attach_volume_label";
const std::string CNWARE_ATTACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_attach_volume_failed_label";
const std::string NUTANIX_VM_ABNORMAL_LABEL = "virtual_plugin_nutanix_vm_abnormal_label";
const std::string CNWARE_BACKUP_CHECK_LABEL = "virtual_plugin_cnware_backup_check_label";
const std::string NUTANIX_HOST_CHECK_FAILED_LABEL = "virtual_plugin_nutanix_host_check_failed_label";

/* cnware restore error code */
constexpr int64_t CNWARE_VMNAME_CHECK_ERROR = 0x5E025E75;
constexpr int64_t NUTANIX_STORAGEPOOL_NOTAVAILABLE_ERROR = 0x5E025E77;
constexpr int64_t CNWARE_CREATE_MACHINE_FAILED_ERROR = 0x5E025E78;
constexpr int64_t CNWARE_POWEROFF_MACHINE_FAILED_ERROR = 0x5E025E79;
constexpr int64_t CNWARE_POWERON_MACHINE_FAILED_ERROR = 0x5E025E7A;

constexpr int64_t CNWARE_AUTH_FAILED = 1577213541;
constexpr int64_t CNWARE_ERR_PARAM = 1677929218;
constexpr int64_t CNWARE_CONNECT_FAILED = 1677931275;
constexpr int64_t CNWARE_CERT_FAILED = 1577213574;

const int64_t INIT_CLIENT_FAILED = 1677929218;
const int32_t ACTION_SUCCESS = 0;
const int32_t ACTION_CONTINUE = 100;
const int32_t ACTION_BUSY = 101;
const int32_t ACTION_ERROR = 200;

};
}
#endif // NUTANIX_ERRORCODE_H