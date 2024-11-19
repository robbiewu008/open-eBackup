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

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 内部组件密码info类
 *
 */
@Data
public class ClusterComponentPwdInfo {
    /**
     * 内部组件密码的Key值
     */
    @NotNull
    @Size(min = 1, max = 256)
    private String passwordField;

    /**
     * 内部组件密码的Value值
     * 根据秘钥生成规则，需包含字母、数字、特殊字符，长度在8-18位
     */
    @NotNull
    @Pattern(regexp = "^(?=.*[a-zA-Z])(?=.*\\d)(?=.*[~!@#$%^&*()_+`\\-={}|\\[\\]:;\"'<>,.?\\\\/ ]).{8,18}$")
    private String passwordValue;
}