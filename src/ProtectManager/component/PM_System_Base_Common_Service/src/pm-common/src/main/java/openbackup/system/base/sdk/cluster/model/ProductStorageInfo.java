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

import lombok.Data;
import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * common模块ProductStorage类
 *
 */
@Data
public class ProductStorageInfo {
    String id;

    @NotNull
    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE,
        message = "The length of ip is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "The ip is invalid")
    String ip;

    @NotNull(message = "The port cannot be null")
    @Max(IsmConstant.PORT_MAX)
    @Min(IsmConstant.PORT_MIN)
    int port;

    String status;

    @NotNull
    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE,
        message = "The length of esn is 1-256 characters")
    String esn;

    String wwn;

    String type;

    String deviceName;

    String createTime;

    @NotNull(message = "The username cannot be null. ")
    @NotEmpty(message = "The username cannot be empty. ")
    @NotBlank(message = "The username cannot be blank. ")
    @Size(min = IsmNumberConstant.FIVE, max = IsmNumberConstant.SIXTY_FOUR)
    String userName;

    @NotNull
    String password;

    @NotNull
    String deviceId;

    @NotNull(message = "The resourceUserId cannot be null. ")
    @NotEmpty(message = "The resourceUserId cannot be empty. ")
    @NotBlank(message = "The resourceUserId cannot be blank. ")
    @Size(min = IsmNumberConstant.FIVE, max = IsmNumberConstant.SIXTY_FOUR)
    String resourceUserId;

    String managementIps;

    @NotNull
    @Min(0)
    @Max(1)
    int authType;

    boolean isVerifyCert;

    @NotNull
    boolean isLocal;

    @NotNull
    boolean isAuthValid;
}