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
package openbackup.system.base.sdk.cluster.request;

import lombok.Data;
import openbackup.system.base.common.constants.IsmNumberConstant;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 功能描述
 *
 */
@Data
public class BackupTaskRequest {
    @NotNull
    private Long imagesId;

    @NotNull
    @Size(max = 256)
    private String desc;

    @NotNull
    @Min(value = IsmNumberConstant.ZERO)
    @Max(value = IsmNumberConstant.ONE)
    private int backupType;

    @Size(max = 16)
    private String password;

    @NotNull
    @Size(max = 1024)
    @Pattern(regexp = "^\\d{13}$")
    private String backupPath;
}
