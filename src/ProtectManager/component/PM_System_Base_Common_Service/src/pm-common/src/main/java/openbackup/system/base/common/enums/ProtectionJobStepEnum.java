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
public enum ProtectionJobStepEnum {
    PROTECTION_START("job_log_resource_protect_prepare_label"),
    PROTECTION_EXECUTING_SUCCESS("job_log_resource_protect_execute_success_label"),
    PROTECTION_EXECUTING_FAILED("job_log_resource_protect_execute_failed_label"),
    PROTECTION_FAILED("job_log_resource_protect_failed_label"),
    PROTECTION_FINISH("job_log_resource_protect_finish_label"),
    PROTECTION_START_MANUAL_FAILED("job_log_start_manual_failed_label"),
    PROTECTION_START_MANUAL_SUCCESS("job_log_start_manual_success_label");

    private final String value;

    ProtectionJobStepEnum(String value) {
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
