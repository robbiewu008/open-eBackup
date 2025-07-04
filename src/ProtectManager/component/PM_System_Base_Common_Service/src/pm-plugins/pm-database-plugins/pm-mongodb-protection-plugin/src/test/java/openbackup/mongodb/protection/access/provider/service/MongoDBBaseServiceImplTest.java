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
package openbackup.mongodb.protection.access.provider.service;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.mongodb.protection.access.service.impl.MongoDBBaseServiceImpl;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * MongoDBBaseService 实现类测试类
 *
 */
public class MongoDBBaseServiceImplTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final MongoDBBaseServiceImpl mongoDBBaseServiceImpl = new MongoDBBaseServiceImpl(resourceService,
        agentUnifiedService, protectedEnvironmentService);

    /**
     * 用例场景：校验auth认证信息正确
     * 前置条件：无
     * 检查点：校验auth认证信息正确
     */
    @Test
    public void check_auth_key_and_auth_pwd_success() {
        String username = "aaa";
        String password = "sss";
        mongoDBBaseServiceImpl.checkKeyLength(username, password);
        Assert.assertNotNull(mongoDBBaseServiceImpl);
    }

    /**
     * 用例场景：校验auth key认证信息错误
     * 前置条件：无
     * 检查点：校验auth key认证长度超过32字符
     */
    @Test
    public void throw_lego_exception_when_auth_key_length_over_32() {
        String username = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        String password = "sss";
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBBaseServiceImpl.checkKeyLength(username, password));
    }

    /**
     * 用例场景：校验auth pwd认证信息错误
     * 前置条件：无
     * 检查点：校验auth pwd认证长度超过32字符
     */
    @Test
    public void throw_lego_exception_when_auth_pwd_length_over_32() {
        String username = "sss";
        String password = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBBaseServiceImpl.checkKeyLength(username, password));
    }
}
