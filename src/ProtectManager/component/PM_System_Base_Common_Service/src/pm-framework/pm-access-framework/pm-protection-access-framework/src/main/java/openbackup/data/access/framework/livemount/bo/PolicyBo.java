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
package openbackup.data.access.framework.livemount.bo;

import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;

import lombok.Data;

/**
 * 更新策略业务对象
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Data
public class PolicyBo {
    private String name;

    private CopyDataSelection copyDataSelectionPolicy;

    private RetentionType retentionPolicy;

    private Integer retentionValue;

    private RetentionUnit retentionUnit;

    private ScheduledType schedulePolicy;

    private Integer scheduleInterval;

    private ScheduledUnit scheduleIntervalUnit;

    private String scheduleStartTime;
}
