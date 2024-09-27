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
package openbackup.data.access.framework.protection.mocks;

import openbackup.system.base.common.constants.TokenBo;

/**
 * token对象模拟
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/9/13
 **/
public class TokenMocker {
    /**
     * 获取模拟token对象
     *
     * @return token对象
     */
    public static TokenBo getMockedTokenBo() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("admin");
        userBo.setId("3434456567");
        long exp = System.currentTimeMillis();
        long created = System.currentTimeMillis();
        TokenBo tokenBo = TokenBo.builder().user(userBo).exp(exp).created(created).build();
        return tokenBo;
    }
}
