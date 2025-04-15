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

/**
 * ResourceProtectionJobSteps
 *
 */
public enum ProtectionModifyJobStepEnum {
    PROTECTION_MODIFY_START("job_log_resource_protection_modify_prepare_label"),
    PROTECTION_MODIFY_EXECUTING_SUCCESS("job_log_resource_protection_modify_execute_success_label"),
    PROTECTION_MODIFY_EXECUTING_FAILED("job_log_resource_protection_modify_execute_failed_label"),
    PROTECTION_REMOVE_SUCCESS("job_log_resource_protection_remove_success_label"),
    PROTECTION_REMOVE_FAILED("job_log_resource_protection_remove_failed_label"),
    PROTECTION_CREATE_SUCCESS("job_log_resource_protect_execute_success_label"),
    PROTECTION_CREATE_FAILED("job_log_resource_protect_execute_failed_label"),
    PROTECTION_MODIFY_FAILED("job_log_resource_protection_modify_failed_label"),
    PROTECTION_MODIFY_FINISH("job_log_resource_protection_modify_finish_label");

    private final String value;

    ProtectionModifyJobStepEnum(String value) {
        this.value = value;
    }

    /**
     * 获取类型字符串
     *
     * @return 类型字符串
     */
    public String getValue() {
        return value;
    }
}
