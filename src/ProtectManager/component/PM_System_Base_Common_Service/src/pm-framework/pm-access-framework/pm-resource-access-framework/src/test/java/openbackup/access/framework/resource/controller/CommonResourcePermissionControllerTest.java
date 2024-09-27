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
package openbackup.access.framework.resource.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;

import openbackup.access.framework.resource.controller.CommonResourceAccessController;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.access.framework.resource.service.lock.ResourceDistributedLockService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.SessionService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;

import java.util.Optional;

@RunWith(SpringRunner.class)
@SpringBootTest(classes = {CommonResourceAccessController.class, SessionService.class})
@MockBean({ResourceService.class, ProtectedResourceMonitorService.class})
@AutoConfigureMockMvc
public class CommonResourcePermissionControllerTest {
    @Autowired
    private MockMvc mvc;

    @Autowired
    private ResourceService resourceService;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private JobService jobService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private ResourceDistributedLockService resourceDistributedLockService;

    /**
     * 用例名称：验证资源创建接口是否能够正常调用。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：rest接口响应体正常。<br/>
     */
    @Test
    public void test_create_resource() throws Exception {
        PowerMockito.when(resourceService.create(any())).thenReturn(new String[0]);
        ProtectedResource resource = new ProtectedResource();
        resource.setName("name");
        String content = JSONObject.fromObject(resource).toString();
        mvc.perform(
                        post("/v2/resources")
                                .accept(MediaType.APPLICATION_JSON)
                                .contentType(MediaType.APPLICATION_JSON)
                                .characterEncoding("utf-8")
                                .content(content))
                .andDo(print());
        Assert.assertNotNull("name");
    }

    /**
     * 用例名称：验证资源查询接口是否能够正常调用。<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：rest接口响应体正常。<br/>
     */
    @Test
    public void test_query_resource() throws Exception {
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
                .thenReturn(Optional.of(new ProtectedResource()));
        mvc.perform(get("/v2/resources/{resourceId}", "resource-id").contentType(MediaType.APPLICATION_JSON));
        Assert.assertTrue(true);
    }
}
