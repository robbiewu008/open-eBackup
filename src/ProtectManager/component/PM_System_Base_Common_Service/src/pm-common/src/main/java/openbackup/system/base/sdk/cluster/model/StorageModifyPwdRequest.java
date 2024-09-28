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

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * Modify dorado passwd request
 *
 */
@Data
public class StorageModifyPwdRequest {
    private static final int MIN_ACCOUNT_LEN = 1;

    private static final int MAX_ACCOUNT_LEN = 256;

    // 用户ID，必选
    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String userId;

    // 新密码，必选
    @NotNull
    @Length(min = MIN_ACCOUNT_LEN, max = MAX_ACCOUNT_LEN)
    private String newPassword;

    // 存储的esn
    @Size(min = 1, max = 256)
    private String esn;
}
