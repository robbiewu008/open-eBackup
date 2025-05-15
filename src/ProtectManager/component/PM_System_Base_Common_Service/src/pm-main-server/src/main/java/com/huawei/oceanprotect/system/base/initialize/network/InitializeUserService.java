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
package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfoEncoder;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;

import java.util.List;

/**
 * 添加DM端口
 *
 */
public interface InitializeUserService {
    /**
     * 获取用户
     *
     * @param service dm 对象
     * @return 用户列表
     */
    List<UserObjectResponse> getUser(DeviceManagerService service);

    /**
     * 删除指定用户
     *
     * @param authUsername 已认证用户名
     * @param username 用户名
     */
    void delUser(String authUsername, String username);

    /**
     * 删除指定用户
     *
     * @param service dm 对象
     * @param username 用户名
     */
    void delUser(DeviceManagerService service, String username);

    /**
     * 添加指定用户
     *
     * @param service dm 对象
     * @param username 用户名;
     * @return 密码
     */
    String createUserAndSetNeverExpire(DeviceManagerService service, String username);

    /**
     * 设置用户密码永不过期
     *
     * @param addUsername 待添加用户名
     */
    void setNeverExpire(String addUsername);

    /**
     * 创建用户
     *
     * @param authUsername 已认证超级管理员用户名
     * @param addUsername 待创建的用户名
     * @param addUserRoleId 待创建的用户的角色
     * @return 密码
     */
    String createUser(String authUsername, String addUsername, Integer addUserRoleId);

    /**
     * 修改当前用户的登录方式
     *
     * @param targetUsername 待修改的用户名
     */
    void modifyUserLoginMethod(String targetUsername);

    /**
     * 修改当前用户的登录方式
     *
     * @param deviceManagerServiceFactory deviceManagerServiceFactory
     * @param deviceManagerInfoEncoder deviceManagerInfoEncoder
     * @param link link
     * @param pwd pwd
     */
    void modifyUserLoginMethod(DeviceManagerServiceFactory deviceManagerServiceFactory,
        DeviceManagerInfoEncoder deviceManagerInfoEncoder, String link, String pwd);

    /**
     * 修改用户密码
     *
     * @param targetUsername 待修改的用户名
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     */
    void modifyUser(String targetUsername, String oldPassword,
        String newPassword);

    /**
     * 初始化用户名和密码
     *
     * @param authUsername 已认证的用户名
     * @param targetUsername 待修改的用户名
     * @param superAdminPassword 超级管理员的密码
     * @param newPassword 新密码
     */
    void initUserPassword(String authUsername, String targetUsername,
        String superAdminPassword, String newPassword);
}
