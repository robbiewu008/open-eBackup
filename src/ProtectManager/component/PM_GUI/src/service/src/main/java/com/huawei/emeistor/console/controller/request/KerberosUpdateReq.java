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
package com.huawei.emeistor.console.controller.request;

import lombok.Data;

import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * KerberosUpdateReq
 *
 */
@Data
public class KerberosUpdateReq {
    @Size(min = 1, max = 64, message = "The length of name is 1-64.")
    @Pattern(regexp = "^[a-zA-Z0-9]{1,64}$")
    private String name;

    @Size(max = 2048, message = "The length of password is 0-2048.")
    private String password;

    @Size(min = 1, max = 64, message = "The length of principal name is 1-64.")
    private String principalName;

    private String createModel;
}
