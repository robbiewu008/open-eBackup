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
package openbackup.system.base.sdk.devicemanager.request;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * pacific 初始化网络时传的参数
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Getter
@Setter
public class IpInfo {
    // ip地址/掩码
    @NotNull(message = "The ipAddress cannot be null")
    @Length(min = 1, max = 64)
    private String ipAddress;

    // 端口名称
    @NotNull(message = "The ifaceName cannot be null")
    @Length(min = 1, max = 64)
    private String ifaceName;
}
