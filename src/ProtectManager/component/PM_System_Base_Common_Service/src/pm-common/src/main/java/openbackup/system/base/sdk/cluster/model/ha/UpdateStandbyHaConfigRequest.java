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
package openbackup.system.base.sdk.cluster.model.ha;

import static openbackup.system.base.common.constants.IsmNumberConstant.ONE;
import static openbackup.system.base.common.constants.IsmNumberConstant.THREE;
import static openbackup.system.base.common.constants.IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 修改备节点HA配置请求
 *
 */
@Getter
@Setter
public class UpdateStandbyHaConfigRequest {
    /**
     * 浮动IP地址
     */
    @NotNull(message = "The floatIpAddress cannot be null. ")
    @Size(max = TWO_HUNDRED_FIFTY_SIX, min = ONE, message = "The length of floatIpAddress is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "floatIpAddress is invalid")
    private String floatIpAddress;

    /**
     * 仲裁网关地址
     */
    @NotNull(message = "The gatewayIpList cannot be null. ")
    @Size(max = THREE, min = ONE, message = "The size of gatewayIpList is 1-3")
    private List<String> gatewayIpList;
}
