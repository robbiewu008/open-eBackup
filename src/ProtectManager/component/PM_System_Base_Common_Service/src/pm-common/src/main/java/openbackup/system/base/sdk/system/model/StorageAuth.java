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
package openbackup.system.base.sdk.system.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotEmpty;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-14
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class StorageAuth {
    @NotEmpty
    @Length(max = 32, min = 1, message = "The length of description is 1 ~ 32")
    private String username;

    @NotEmpty
    @Length(max = 16, min = 1, message = "The length of description is 1 ~ 16")
    private String password;
}
