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
import openbackup.system.base.common.constants.FaultEnum.AlarmSeverity;

/**
 * Lego告警
 *
 */
@Data
public class LegoInternalEvent {
    // 告警ID
    private String eventId;

    // 告警级别
    private AlarmSeverity eventLevel = AlarmSeverity.INVALID;

    // 告警产生时间
    private long eventTime;

    // 告警流水号
    private long eventSequence = -IsmNumberConstant.ONE;

    // 对应Lego ManagedObject对象的MOID属性
    private String moId = String.valueOf(-IsmNumberConstant.ONE);

    // 网元显示名称
    private String moName;

    // 网元IP
    private String moIP;

    // 告警参数
    private String[] eventParam;

    // lego错误码
    private String legoErrorCode;

    // 告警产生对象
    private String sourceId;

    // 对象类型：资源、业务对象
    private String sourceType;

    // 操作是否成功
    private Boolean isSuccess;

    // 制造用户ID
    private String userId;

    // 告警关联资源ID
    private String resourceId;

    // DME场景下的Token信息
    private String dmeToken;
}
