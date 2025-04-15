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
const std::string NUTANIX_RESTORE_PRECHECK_LABEL = "virtual_plugin_cnware_restore_precheck_label";
const std::string NUTANIX_VMNAME_CHECK_LABEL = "virtual_plugin_cnware_vmname_check_label";
const std::string NUTANIX_VMNAME_NOTUNIQUE_LABEL = "virtual_plugin_nutanix_vmname_notunique_label";
const std::string NUTANIX_ARCH_CHECK_FAILED_LABEL = "virtual_plugin_nutanix_arch_check_failed_label";
const std::string NUTANIX_STORAGEPOOL_CHECK_LABEL = "virtual_plugin_cnware_storagepool_check_label";
const std::string NUTANIX_STORAGEPOOL_NOTAVAILABLE_LABEL = "virtual_plugin_cnware_storagepool_notavailable_label";
const std::string CNWARE_STORAGEPOOL_FAILED_LABEL = "virtual_plugin_cnware_storagepool_failed_label";
const std::string NUTANIX_CREATE_MACHINE_LABEL = "fc_plugin_create_vm_label";
const std::string NUTANIX_CREATE_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_create_machine_failed_label";
const std::string NUTANIX_POWEROFF_MACHINE_LABEL = "virtual_plugin_cnware_poweroff_machine_label";
const std::string NUTANIX_POWEROFF_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_poweroff_machine_failed_label";
const std::string NUTANIX_POWERON_MACHINE_LABEL = "virtual_plugin_cnware_poweron_machine_label";
const std::string NUTANIX_POWERON_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_poweron_machine_failed_label";
const std::string NUTANIX_DELETE_MACHINE_LABEL = "virtual_plugin_cnware_delete_machine_label";
const std::string CNWARE_DELETE_MACHINE_FAILED_LABEL = "virtual_plugin_cnware_delete_machine_failed_label";
const std::string CNWARE_DETACH_VOLUME_LABEL = "virtual_plugin_cnware_detach_volume_label";
const std::string CNWARE_DETACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_detach_volume_failed_label";
const std::string CNWARE_ATTACH_VOLUME_LABEL = "virtual_plugin_cnware_attach_volume_label";
const std::string CNWARE_ATTACH_VOLUME_FAILED_LABEL = "virtual_plugin_cnware_attach_volume_failed_label";
const std::string NUTANIX_VM_ABNORMAL_LABEL = "virtual_plugin_cnware_vm_abnormal_label";
const std::string CNWARE_BACKUP_CHECK_LABEL = "virtual_plugin_cnware_backup_check_label";
const std::string NUTANIX_HOST_CHECK_FAILED_LABEL = "virtual_plugin_cnware_host_check_failed_label";
const std::string NUTANIX_REPORT_LABEL_START_BACKUP =  "fc_plugin_backup_execute_label";
const std::string NUTANIX_DELETE_VM_FAILED_LABEL = "fc_plugin_delete_vm_failed_label";

/* nutanix error code */
constexpr int64_t CNWARE_VMNAME_CHECK_ERROR = 0x5E025E75;
constexpr int64_t NUTANIX_STORAGEPOOL_NOTAVAILABLE_ERROR = 0x5E025E77;
constexpr int64_t CNWARE_CREATE_MACHINE_FAILED_ERROR = 0x5E025E78;
constexpr int64_t CNWARE_POWEROFF_MACHINE_FAILED_ERROR = 0x5E025E79;
constexpr int64_t CNWARE_POWERON_MACHINE_FAILED_ERROR = 0x5E025E7A;

constexpr int64_t USER_OR_PASSWORD_IS_INVALID = 1677929224;
constexpr int64_t NETWORK_SSL_ERROR = 1677931450;
constexpr int64_t NUTANIX_AUTH_FAILED = 1577213541;
constexpr int64_t NUTANIX_ERR_PARAM = 1677929218;
constexpr int64_t NUTANIX_CONNECT_FAILED = 1677931275;
constexpr int64_t NUTANIX_CERT_FAILED = 1577213574;

const int64_t INIT_CLIENT_FAILED = 1677929218;
const int32_t ACTION_SUCCESS = 0;
const int32_t ACTION_CONTINUE = 100;
const int32_t ACTION_BUSY = 101;
const int32_t ACTION_ERROR = 200;
const int64_t SSL_CONNECT_ERROR = 35;

inline std::map<int64_t, int64_t> nutanixErrorCodeMap = {
    {USER_OR_PASSWORD_IS_INVALID, USER_OR_PASSWORD_IS_INVALID},
    {SSL_CONNECT_ERROR, NETWORK_SSL_ERROR}
};

};
}
#endif // NUTANIX_ERRORCODE_H