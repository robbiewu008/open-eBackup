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
package openbackup.data.access.framework.livemount.controller.policy.request;

import lombok.Data;
import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 创建更新策略
 *
 */
@Data
public class CreatePolicyRequest {
    @NotNull
    @Pattern(regexp = "[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    @Size(min = 1, max = 64)
    private String name;

    @NotNull
    private CopyDataSelection copyDataSelectionPolicy;

    @NotNull
    private RetentionType retentionPolicy;

    private Integer retentionValue;

    private RetentionUnit retentionUnit;

    @NotNull
    private ScheduledType schedulePolicy;

    private Integer scheduleInterval;

    private ScheduledUnit scheduleIntervalUnit;

    private String scheduleStartTime;
}
