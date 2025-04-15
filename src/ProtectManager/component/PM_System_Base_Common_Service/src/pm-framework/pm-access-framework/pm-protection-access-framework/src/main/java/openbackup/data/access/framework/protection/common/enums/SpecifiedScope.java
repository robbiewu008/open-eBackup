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
package openbackup.data.access.framework.protection.common.enums;

import lombok.Getter;
import openbackup.data.access.framework.copy.mng.enums.CopyTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;

/**
 * 统一转换成天比较
 *
 */
@Getter
public class SpecifiedScope {
    private final TimeUnitEnum timeUnit;

    private final int retentionDuration;

    private final int days;

    public SpecifiedScope(CopyTypeEnum copyType, int retentionDuration) {
        this.timeUnit = copyType.getTimeUnitEnum();
        this.retentionDuration = retentionDuration;
        this.days = copyType.getDays() * retentionDuration;
    }
}
