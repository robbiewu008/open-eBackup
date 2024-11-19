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
package openbackup.mongodb.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * MongoDBInstanceResourceProviderTest
 *
 */
@RunWith(PowerMockRunner.class)
public class MongoDBInstanceResourceProviderTest {
    @Mock
    private ResourceService resourceService;

    @InjectMocks
    private MongoDBInstanceResourceProvider provider;

    @Test
    public void test_applicable() {
        ProtectedResource resource = mockResource();
        Assert.assertTrue(provider.applicable(resource));
    }

    @Test
    public void test_supplyDependency() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setName("name1");
        protectedEnvironment.setEndpoint("endpoint1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PowerMockito.when(resourceService.queryDependencyResources(Mockito.anyBoolean(), Mockito.any(), Mockito.any()))
                .thenReturn(list);
        Assert.assertTrue(provider.supplyDependency(mockResource()));
    }

    private ProtectedResource mockResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.MONGODB_SINGLE.getType());
        protectedResource.setUuid("test1");
        return protectedResource;
    }
}
