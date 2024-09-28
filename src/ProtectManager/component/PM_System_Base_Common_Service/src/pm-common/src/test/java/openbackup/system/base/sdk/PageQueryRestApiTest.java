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
package openbackup.system.base.sdk;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.VirtualResourceSchema;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static org.mockito.ArgumentMatchers.*;

/**
 * PageQueryRestApi
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {PageQueryRestApi.class})
public class PageQueryRestApiTest {
    @MockBean
    private ResourceRestApi resourceRestApi;

    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    private static final String GUID = "uuid";

    private static final String DATA_STORE_ID = "datastore-2279";

    private static final String DATA_STORE_ID2 = "datastore-2280";

    private static final String VM_IP_MOK = "127.128.129.00";

    private static final String TEST_RESOURCE_ID = "resourceid-123456";

    private static final String LINK_StATUS = "1";

    @Test
    public void get_page_query_rest_api_object_success() {
        Assert.assertNotNull(PageQueryRestApi.get(resourceRestApi::queryVMResources));
    }

    @Test
    public void query_one_method_when_parameter_is_one_success() {
        PowerMockito.when(resourceRestApi.queryVMResources(anyInt(), anyInt(), anyString(), anyList()))
                .thenReturn(getBasePageVirtualResourceSchema());
        VirtualResourceSchema mountedVMWareResource = PageQueryRestApi.get(resourceRestApi::queryVMResources)
                .queryOne(new JSONObject().set(GUID, TEST_RESOURCE_ID));
        Assert.assertNotNull(PageQueryRestApi.get(resourceRestApi::queryVMResources));
        Assert.assertEquals(DATA_STORE_ID, mountedVMWareResource.getMoId());
        Assert.assertEquals(VM_IP_MOK, mountedVMWareResource.getVmIp());
        Assert.assertEquals(LINK_StATUS, mountedVMWareResource.getLinkStatus());
    }

    @Test
    public void query_one_method_when_parameter_is_two_success() {
        PowerMockito.when(resourceRestApi.queryVMResources(anyInt(), anyInt(), anyString(), anyList()))
                .thenReturn(getBasePageVirtualResourceSchema());
        VirtualResourceSchema mountedVMWareResource = PageQueryRestApi.get(resourceRestApi::queryVMResources)
                .queryOne(new JSONObject().set(GUID, TEST_RESOURCE_ID), Collections.singletonList("-display_timestamp"));
        Assert.assertNotNull(mountedVMWareResource);
        Assert.assertEquals(DATA_STORE_ID, mountedVMWareResource.getMoId());
        Assert.assertEquals(VM_IP_MOK, mountedVMWareResource.getVmIp());
        Assert.assertEquals(LINK_StATUS, mountedVMWareResource.getLinkStatus());
    }

    @Test
    public void should_return_LegoCheckedException_when_query_one_strict_is_true() {
        PowerMockito.when(resourceRestApi.queryVMResources(anyInt(), anyInt(), anyString(), anyList()))
                .thenReturn(getBasePageVirtualResourceSchema());
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Multiple resources found");
        PageQueryRestApi.get(resourceRestApi::queryVMResources)
                .queryOne(new JSONObject().set(GUID, TEST_RESOURCE_ID), true);
    }

    @Test
    public void count_method_when_parameter_is_two_success() {
        PowerMockito.when(resourceRestApi.queryVMResources(anyInt(), anyInt(), anyString(), anyList()))
                .thenReturn(getBasePageVirtualResourceSchema());
        long count = PageQueryRestApi.get(resourceRestApi::queryVMResources)
                .count(new JSONObject().set("uuid", TEST_RESOURCE_ID));
        Assert.assertEquals(2, count);
    }

    private BasePage<VirtualResourceSchema> getBasePageVirtualResourceSchema() {
        BasePage<VirtualResourceSchema> virtualResourceSchemaBasePage = new BasePage<>();
        List<VirtualResourceSchema> virtualResourceSchemas = mockDataStoresInfo();
        virtualResourceSchemaBasePage.setTotal(2);
        virtualResourceSchemaBasePage.setItems(virtualResourceSchemas);
        return virtualResourceSchemaBasePage;
    }

    private List<VirtualResourceSchema> mockDataStoresInfo() {
        List<VirtualResourceSchema> dataStores = new ArrayList<>();
        VirtualResourceSchema virtualResourceSchema1 = new VirtualResourceSchema();
        virtualResourceSchema1.setMoId(DATA_STORE_ID);
        virtualResourceSchema1.setVmIp(VM_IP_MOK);
        virtualResourceSchema1.setLinkStatus(LINK_StATUS);
        virtualResourceSchema1.setName(DATA_STORE_ID);
        VirtualResourceSchema virtualResourceSchema2 = new VirtualResourceSchema();
        virtualResourceSchema2.setMoId(DATA_STORE_ID2);
        virtualResourceSchema2.setLinkStatus(LINK_StATUS);
        virtualResourceSchema2.setVmIp(VM_IP_MOK);
        virtualResourceSchema2.setName(DATA_STORE_ID2);
        dataStores.add(virtualResourceSchema1);
        dataStores.add(virtualResourceSchema2);
        return dataStores;
    }
}
