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
package openbackup.system.base.sdk.auth.api;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.sdk.auth.UserInfo;
import openbackup.system.base.sdk.auth.UserLoginResponse;

import java.util.Optional;

/**
 * User Auth 本地调用API接口定义
 *
 */
public interface AuthNativeApi {
    /**
     * 根据用户名获取用户信息
     *
     * @param username 用户名
     * @return TokenBo.UserInfo 用户信息
     */
    TokenBo.UserInfo queryUserInfoByName(String username);

    /**
     * 根据用户id获取用户信息
     *
     * @param userId 用户id
     * @return 用户信息
     */
    Optional<TokenBo.UserInfo> queryUserInfoById(String userId);

    /**
     * 获取对应用户的token
     *
     * @param userInfo 用户信息
     * @return UserLoginResponse 用户登录响应
     */
    UserLoginResponse genToken(UserInfo userInfo);

    /**
     * 生成sysadmin的Token
     *
     * @return 生成sysadmin的Token
     */
    String generateSysadminToken();

    /**
     * 生成cluster_admin的Token
     *
     * @return 生成cluster_admin的Token
     */
    String generateClusterAdminToken();

    /**
     * 生成cluster_admin的Token
     *
     * @param expire 过期时间
     * @return 生成cluster_admin的Token
     */
    String generateClusterAdminToken(int expire);


    /**
     * 根据userid获取token
     *
     * @param userId userId
     * @return token
     */
    UserLoginResponse genToken(String userId);

    /**
     * 根据用户id获取token，用户id为null时，获取cluster_admin的token
     *
     * @param userId 用户id
     * @return token token
     */
    String genTokenByUserId(String userId);
}
