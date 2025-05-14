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
package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.enums.AddressFamily;

/**
 * 修改逻辑端口参数
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/25
 */
@Getter
@Setter
public class ModifyLogicPortDto {
    private String id;

    private String name;

    private String ip;

    private String mask;

    private String gateWay;

    private AddressFamily addressFamily;

    private Boolean isFailOver;

    private PortRole role;

    private String failoverGroupId;
}
