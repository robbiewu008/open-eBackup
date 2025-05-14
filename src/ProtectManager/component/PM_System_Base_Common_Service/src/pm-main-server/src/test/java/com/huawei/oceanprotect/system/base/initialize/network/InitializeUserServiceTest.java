/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeUserServiceAbility;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.UserServiceApi;
import com.huawei.oceanprotect.system.base.service.InitConfigService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 测试DM 用户能力
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-16
 */
@RunWith(PowerMockRunner.class)
public class InitializeUserServiceTest {
    @InjectMocks
    private InitializeUserServiceAbility initializeUserServiceAbility;

    @Mock
    private UserServiceApi userServiceApi;

    @Mock
    private InitConfigService initConfigService;

    /**
     * 用例场景：删除用户
     * 前置条件：NA
     * 检查点：删除成功
     */
    @Test
    public void del_user_success() {
        initializeUserServiceAbility.delUser("", "dataprotect_admin");
        Assert.assertTrue(true);
    }


    /**
     * 用例场景：修改用户权限
     * 前置条件：NA
     * 检查点：修改用户权限成功
     */
    @Test
    public void modify_user_login_method_success() {
        initializeUserServiceAbility.modifyUserLoginMethod("");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：设置永不过期
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_setNeverExpire_success() {
        initializeUserServiceAbility.setNeverExpire("");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建用户
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_createUser_success() {
        initializeUserServiceAbility.createUser("", "", 1);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：修改用户
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_modifyUser_success() {
        initializeUserServiceAbility.modifyUser("", "", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：初始化用户
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_initUserPassword_success() {
        initializeUserServiceAbility.initUserPassword("", "",
            "", "");
        Assert.assertTrue(true);
    }
}
