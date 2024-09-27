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
package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 修改dorado用户名请求体
 *
 * @author t00482481
 * @since 2020-07-31
 */
@Data
public class DoradoModifyPwdRequest {
    @JsonProperty("ID")
    private String id;

    @JsonProperty("OLDPASSWORD")
    private String oldPassword;

    @JsonProperty("PASSWORD")
    private String password;

    /**
     * 修改密碼请求构造器
     *
     * @param userName    用户名
     * @param oldPassword 旧密码
     * @param password    新密码
     */
    public DoradoModifyPwdRequest(String userName, String oldPassword, String password) {
        this.id = userName;
        this.oldPassword = oldPassword;
        this.password = password;
    }
}
