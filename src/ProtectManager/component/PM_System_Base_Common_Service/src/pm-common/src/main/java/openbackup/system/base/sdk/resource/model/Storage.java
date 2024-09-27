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
package openbackup.system.base.sdk.resource.model;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 存储的信息
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/19
 */
@Data
public class Storage {
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "ip is invalid, not ipv4 or ipv6.")
    private String ip;

    @Max(IsmConstant.PORT_MAX)
    private int port;

    // 存储用户名
    @NotNull(message = "The username cannot be null. ")
    private String username;

    // 存储密码
    @NotNull(message = "The password cannot be null. ")
    private String password;

    // 存储类型：0 DoradoV6，1 NetApp
    private int type;
}
