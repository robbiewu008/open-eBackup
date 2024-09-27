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
package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.constants.IsmNumberConstant;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 组件信息
 *
 * @author x30046484
 * @since 2023-05-17
 */
@Getter
@Setter
public class ComponentIpInfo {
    @NotNull
    @Size(min = 1, max = 256)
    private String componentName;

    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE, message = "The length of ip is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "cluster ip is invalid, not ipv4 or ipv6.")
    private String ip;
}
