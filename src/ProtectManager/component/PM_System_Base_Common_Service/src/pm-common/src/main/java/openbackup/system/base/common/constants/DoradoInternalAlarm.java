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
package openbackup.system.base.common.constants;

import lombok.Data;

/**
 * dorado告警对象
 *
 */
@Data
public class DoradoInternalAlarm extends LegoInternalAlarm {
    /**
     * 清除时间
     */
    private long clearTime;

    /**
     * 恢复时间
     */
    private long recoverTime;

    private String name;

    private String suggestion;

    private String description;

    private String detail;

    private String alarmObjType;

    private String location;
}
