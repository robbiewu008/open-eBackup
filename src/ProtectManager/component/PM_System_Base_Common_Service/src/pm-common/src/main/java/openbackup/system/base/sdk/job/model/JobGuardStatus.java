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
package openbackup.system.base.sdk.job.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;

import java.util.Locale;

/**
 * Job Guard Status
 *
 * @author l00272247
 * @since 2021-03-12
 */
public enum JobGuardStatus {
    /**
     * DISCARD
     */
    DISCARD,
    /**
     * cancelled
     */
    CANCELLED,
    /**
     * pending
     */
    PENDING,
    /**
     * running
     */
    RUNNING;

    /**
     * get job guard status by name
     *
     * @param name name
     * @return status
     */
    @JsonCreator
    public static JobGuardStatus get(String name) {
        return EnumUtil.get(JobGuardStatus.class, JobGuardStatus::name, name.toUpperCase(Locale.ENGLISH));
    }
}
