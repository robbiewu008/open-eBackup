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
package openbackup.system.base.sdk.hostagent.model;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * LAN-FREE配置页面WWPN信息
 *
 */
@Getter
@Setter
public class AgentLanFreeWwpn {
    /**
     * 客户端wwpn
     */
    @Pattern(regexp = "^[0-9A-Fa-f]{16}$")
    private String wwpn;

    /**
     * 是否客户页面添加的
     */
    private boolean isManualAdd;

    /**
     * 是否选择
     */
    private boolean isChosen;

    /**
     * 运行状态
     */
    @Length(min = 1, max = 64)
    private String runningStatus;
}
