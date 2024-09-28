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
package openbackup.system.base.sdk.storage.model;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Pattern;

/**
 * 存储信息
 *
 */
@Data
public class ProductStorageReq {
    @Length(max = 32)
    private String userName;

    @Length(max = 32)
    private String password;

    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "ip is invalid, not ipv4 or ipv6.")
    private String ip;

    @Max(IsmConstant.PORT_MAX)
    private int port;
}
