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
package openbackup.access.framework.resource.service.proxy;

import openbackup.access.framework.resource.service.proxy.ProxyFactory;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.util.Arrays;

/**
 * Proxy Factory Test
 *
 */
public class ProxyFactoryTest {
    /**
     * 用例场景：验证代理工厂逻辑正确性
     * 前置条件：代理工厂及原始对象准备完成
     * 检查点： 1.针对getter场景取值正确； 2.针对setter场景设值正确。
     */
    @Test
    public void test_create() {
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setType("type1");
        resource1.setExtendInfoByKey("key1", "value1");
        Authentication authentication1 = new Authentication();
        authentication1.setAuthKey("admin");
        resource1.setAuth(authentication1);

        ProtectedResource resource2 = new ProtectedResource();
        resource2.setType("type2");
        resource2.setSubType("sub-type2");
        resource2.setExtendInfoByKey("key2", "value2");
        Authentication authentication2 = new Authentication();
        authentication2.setAuthKey("root");
        authentication2.setAuthPwd("xxx");
        resource2.setAuth(authentication2);

        ProtectedResource resource = ProxyFactory.get(ProtectedResource.class).create(
            Arrays.asList(resource1, resource2, null));

        JsonUtil.json(resource);

        Assertions.assertNull(resource.getExtendInfoByKey("missing"));
        Assertions.assertEquals("type1", resource.getType());
        Assertions.assertEquals("sub-type2", resource.getSubType());
        Assertions.assertEquals("value1", resource.getExtendInfoByKey("key1"));
        Assertions.assertEquals("value2", resource.getExtendInfoByKey("key2"));

        Authentication authentication = resource.getAuth();
        Assertions.assertEquals("admin", authentication.getAuthKey());
        Assertions.assertEquals("xxx", authentication.getAuthPwd());

        resource.setType("type");
        resource.setExtendInfoByKey("key1", "value");
        resource.setExtendInfoByKey("key2", "value");
        Assertions.assertEquals("type", resource.getType());
        Assertions.assertEquals("type", resource1.getType());
        Assertions.assertEquals("type2", resource2.getType());

        Assertions.assertEquals("value", resource.getExtendInfoByKey("key1"));
        Assertions.assertEquals("value", resource.getExtendInfoByKey("key2"));
        Assertions.assertEquals("value", resource1.getExtendInfoByKey("key1"));
        Assertions.assertEquals("value", resource1.getExtendInfoByKey("key2"));
        Assertions.assertNull(resource2.getExtendInfoByKey("key1"));
        Assertions.assertEquals("value2", resource2.getExtendInfoByKey("key2"));

        authentication.setAuthKey("sys");
        Assertions.assertEquals("sys", authentication.getAuthKey());
        Assertions.assertEquals("sys", authentication1.getAuthKey());
        Assertions.assertEquals("root", authentication2.getAuthKey());
    }
}
