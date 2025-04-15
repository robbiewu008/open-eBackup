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
package openbackup.tdsql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述 TdsqlClusterGroupProviderTest
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlClusterGroupProviderTest {
    @Mock
    private TdsqlService mockTdsqlService;

    @Mock
    private ResourceService mockResourceService;

    private TdsqlClusterGroupProvider tdsqlClusterGroupProvider;

    @Before
    public void setUp() {
        tdsqlClusterGroupProvider = new TdsqlClusterGroupProvider(mockTdsqlService, mockResourceService);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：subType为TDSQL-clusterGroup
     * 检查点：过滤成功
     */
    @Test
    public void test_applicable_success() {
        final ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        boolean result = tdsqlClusterGroupProvider.applicable(object);
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：subType不为TDSQL-clusterGroup
     * 检查点：过滤成功
     */
    @Test
    public void test_applicable_failed() {
        final ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setSubType("object");
        boolean result = tdsqlClusterGroupProvider.applicable(object);
        Assert.assertFalse(result);
    }

    @Test
    public void test_supplyDependency() {
        final ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        protectedResource.setUuid("test1");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setName("name1");
        protectedEnvironment.setEndpoint("endpoint1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PowerMockito.when(
                mockResourceService.queryDependencyResources(Mockito.anyBoolean(), Mockito.any(), Mockito.any()))
            .thenReturn(list);
        Assert.assertTrue(tdsqlClusterGroupProvider.supplyDependency(protectedResource));
    }

    /**
     * 用例场景：注册/更新实例信息时对实例信息进行检查
     * 前置条件：实例信息正确
     * 检查点：环境信息进行检查无异常
     */
    @Test
    public void test_before_create_success() {
        when(mockTdsqlService.checkGroupInfo(any(), any())).thenReturn(true);
        when(mockTdsqlService.checkDataNodeIsMatchAgent(any(), any())).thenReturn(true);
        ProtectedEnvironment environment = getEnvironment();
        environment.setEndpoint("9.9.9.9");
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(environment);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(getProtectedResourcePageListResponse());

        ProtectedEnvironment protectedEnvironment = getEnvironment();
        tdsqlClusterGroupProvider.beforeCreate(protectedEnvironment);
        tdsqlClusterGroupProvider.beforeUpdate(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getUuid(), "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
    }

    private PageListResponse<ProtectedResource> getProtectedResourcePageListResponse() {
        final ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1878ba2e-be98-4004-890a-ee18cc0e1422");
        protectedResource.setVersion("9.11.6");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId("123");
        HashMap<String, String> map = new HashMap<>();
        map.put("version", "9.11.6");
        protectedResource.setExtendInfo(map);
        protectedResource.setProtectedObject(protectedObject);
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource record = getEnvironment();
        records.add(record);
        protectedResourcePageListResponse.setRecords(records);
        return protectedResourcePageListResponse;
    }

    private ProtectedEnvironment getEnvironment() {
        String json = "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"9e68a8f1-7ad4-3eef-a808-dce3b2062120\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.190\\\",\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"},{\\\"ip\\\":\\\"8.40.168.192\\\",\\\"parentUuid\\\":\\\"c75146f7-7e2a-41d6-b110-28d0e22245ee\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }
}
