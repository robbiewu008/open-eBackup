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
package openbackup.access.framework.resource.util;

import openbackup.access.framework.resource.util.ResourceUtil;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Objects;

/**
 * 描述
 *
 */
public class ResourceUtilTest {
    @Test
    public void set_source_type_success() {
        String sourceType = "source_type";
        ProtectedResource root = new ProtectedResource();
        root.setDependencies(new HashMap<>());
        List<ProtectedResource> children = root.getDependencies().computeIfAbsent("children", e -> new ArrayList<>());
        ProtectedResource r1 = new ProtectedResource();
        r1.setUuid("1");
        children.add(r1);
        ProtectedResource r2 = new ProtectedResource();
        children.add(r2);
        ResourceUtil.supplySourceTypeWhenUuidNull(root, sourceType);
        Assert.assertEquals(root.getSourceType(), sourceType);
        root.getDependencies().forEach((k, v) -> {
            v.forEach(elem -> {
                if (Objects.nonNull(elem.getUuid())) {
                    Assert.assertTrue(Objects.isNull(elem.getSourceType()));
                } else {
                    Assert.assertEquals(elem.getSourceType(), sourceType);
                }
            });
        });
    }

    @Test
    public void combine_should_success_return_when_not_same_type() {
        ProtectedResource source = new ProtectedResource();
        source.setName("source");
        source.setUuid("uuid");
        ProtectedEnvironment target = new ProtectedEnvironment();
        target.setName("target");
        target.setLocation("targetLocation");
        ProtectedResource res = ResourceUtil.combineProtectedResource(source, target);
        Assert.assertEquals(res.getClass(), ProtectedEnvironment.class);
        ProtectedEnvironment resEnv = (ProtectedEnvironment) res;
        Assert.assertEquals(resEnv.getName(), "source");
        Assert.assertEquals(resEnv.getLocation(), "targetLocation");

        ProtectedResource convertRes = ResourceUtil.combineProtectedResource(target, source);
        Assert.assertEquals(convertRes.getName(), "target");
    }


    /**
     * 用例名称：验证merge函数功能是否正确。<br/>
     * 前置条件：源数据准备完成。<br/>
     * check点：合并结果正确。<br/>
     */
    @Test
    public void test_merge() {
        Assert.assertNull(ResourceUtil.merge(ProtectedResource.class, null, null, true));
        ProtectedResource resource0 = new ProtectedResource();
        Authentication authentication0 = new Authentication();
        authentication0.setAuthKey("0");
        resource0.setAuth(authentication0);
        Assert.assertEquals(
            authentication0.getAuthKey(),
            ResourceUtil.merge(ProtectedResource.class, null, resource0, true).getAuth().getAuthKey());

        resource0.setUuid("uuid");
        ProtectedResource resource1 = new ProtectedResource();
        Authentication authentication1 = new Authentication();
        authentication1.setAuthKey("1");
        resource1.setAuth(authentication1);

        ProtectedResource resource2 = ResourceUtil.merge(ProtectedResource.class, resource0, resource1, true);
        Assert.assertEquals(authentication1.getAuthKey(), resource2.getAuth().getAuthKey());
        Assert.assertEquals("uuid", resource2.getUuid());

        ProtectedEnvironment environment0 = new ProtectedEnvironment();
        environment0.setLinkStatus("online");
        resource0.setEnvironment(environment0);
        resource0.setExtendInfoByKey("field0", "value0");

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment0.setLinkStatus("offline");
        resource1.setEnvironment(environment1);
        resource1.setExtendInfoByKey("field0", "value1");
        resource1.setExtendInfoByKey("field1", "data");

        ProtectedResource resource3 = ResourceUtil.merge(ProtectedResource.class, resource0, resource1, true);
        Assert.assertEquals(authentication1.getAuthKey(), resource3.getAuth().getAuthKey());
        Assert.assertEquals("uuid", resource3.getUuid());
        Assert.assertEquals("offline", resource3.getEnvironment().getLinkStatus());
        Assert.assertEquals("value1", resource3.getExtendInfoByKey("field0"));
        Assert.assertEquals("data", resource3.getExtendInfoByKey("field1"));
    }

    /**
     * 用例名称：验证convertStorageType函数功能是否正确。<br/>
     * 前置条件：源数据准备完成。<br/>
     * check点：类型转换正确。<br/>
     */
    @Test
    public void test_convertStorageType() {
        Assert.assertEquals("OceanStor Pacific", ResourceUtil.convertStorageType("Pacific"));
        Assert.assertEquals("OceanStor Dorado", ResourceUtil.convertStorageType("Dorado"));
        Assert.assertEquals("OceanProtect", ResourceUtil.convertStorageType("CyberEngine OceanProtect"));
        Assert.assertEquals("test", ResourceUtil.convertStorageType("test"));
    }
}
