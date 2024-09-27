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
package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.openstack.adapter.service.OpenStackUserManager;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.user.entity.UserInfoEntity;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import javax.servlet.http.HttpServletRequest;

/**
 * {@link OpenStackUserManager} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-27
 */
public class OpenStackUserManagerTest {
    private final UserService userService = Mockito.mock(UserService.class);
    private final HttpServletRequest request = Mockito.mock(HttpServletRequest.class);
    private final OpenStackUserManager userManager = new OpenStackUserManager(userService, request);

    /**
     * 用例场景：如果查询用户为null，则抛出异常
     * 前置条件：用户不存在
     * 检查点：  内置用户必须存在
     */
    @Test
    public void should_throwLegoCheckedException_when_obtainUserId_given_noneExistUser() {
        Mockito.when(userService.getUserInfoByName(anyString())).thenReturn(null);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class, userManager::obtainUserId);
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.UNKNOWN_USER);
        assertThat(exception.getMessage()).isEqualTo("user not exists.");
    }

    /**
     * 用例场景：如果内置用户存在，则返回对应用户id
     * 前置条件：用户存在
     * 检查点：  能够获取到内置用户的id
     */
    @Test
    public void should_returnUserId_when_obtainUserId_given_existUser() {
        String mockUserId = "user_id";
        UserInfoEntity user = new UserInfoEntity();
        user.setUuid(mockUserId);
        Mockito.when(userService.getUserInfoByName(anyString())).thenReturn(user);
        String userId = userManager.obtainUserId();
        assertThat(userId).isEqualTo(mockUserId);
    }
}
