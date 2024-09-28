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
package openbackup.system.base.sdk.anti.model;

import lombok.Data;

import java.util.List;

/**
 * AirGap策略信息对象返回
 *
 */
@Data
public class AirGapPolicyInfoRsp {
    private String id;

    /**
     * 策略名称
     */
    private String name;

    /**
     * 描述
     */
    private String description;

    /**
     * 策略频率
     */
    private String triggerCycle;

    /**
     * 策略周期
     */
    private String triggerWeekFreq;

    /**
     * 策略时间窗
     */
    private List<AirGapPolicyWindowInfoRsp> airGapPolicyWindows;

    /**
     * 关联设备数量
     */
    private Integer deviceCount;
}
