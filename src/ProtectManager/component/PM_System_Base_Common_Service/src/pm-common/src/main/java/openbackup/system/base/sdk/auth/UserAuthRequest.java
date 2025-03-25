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
package openbackup.system.base.sdk.auth;

import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.enums.UserTypeEnum;

/**
 * 功能描述
 *
 */
@Data
@Slf4j
public class UserAuthRequest {
    private String userName;

    private String password;

    private String userType;

    private boolean isOnlyAuth;

    // 此处由于调用处过多 故做两种初始化适配 如果初始化时没有传入userType 则默认为COMMON
    public UserAuthRequest(String userName, String password, String userType, boolean isOnlyAuth) {
        this.userName = userName;
        this.password = password;
        this.userType = (userType == null) ? UserTypeEnum.COMMON.getValue() : userType; // 设定默认值
        this.isOnlyAuth = isOnlyAuth;
    }

    public UserAuthRequest() {
        this(null, null, null, false);
    }


    // 顺便对set进行防呆处理
    public void setUserType(String userType) {
        if (userType == null) {
            this.userType = UserTypeEnum.COMMON.getValue(); // 默认值
        } else {
            this.userType = userType;
        }
    }
}
