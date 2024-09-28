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
package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

/**
 * 页面展示ProtectedAgentExtendPo
 *
 */
@Getter
@Setter
public class ProtectedAgentExtend {
    /**
     * CPU占用率百分比 单位：%
     */
    private double cpuRate;

    /**
     * 内存占用率百分比  单位：%
     */
    private double memRate;

    /**
     * 最近一次更新时间 距离1970年相对时间（带时区）
     */
    private long lastUpdateTime;

    /**
     * 是否开启多租户共享
     */
    private Boolean isShared;
}
