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

import openbackup.access.framework.resource.controller.AgentResourceController;
import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockHttpServletRequestBuilder;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultHandlers;

import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

/**
 * 功能描述: AgentResourceControllerTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-26
 */
@RunWith(SpringRunner.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@SpringBootTest(classes = {AgentResourceController.class})
public class AgentResourceControllerTest {
    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private AgentUnifiedService agentService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private AgentBusinessService agentBusinessService;
    /**
     * 用例名称：验证通过指定代理主机实时浏览受保护资源的接口正常调用
     * 前置条件：spring mvc框架正常运行。
     * check点：rest接口能正常接收到请求体，并响应正常
     */
    @Test
    public void test_query_resources_by_agent_success() throws Exception {
        when(resourceService.getBasicResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        when(agentService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
                .thenReturn(new PageListResponse<>());
        String agentId = UUIDGenerator.getUUID();
        String url = "/v2/agents/" + agentId + "/resources";
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(url)
                .queryParam("envId", UUIDGenerator.getUUID())
                .queryParam("pageNo", "1")
                .queryParam("pageSize", "10");
        mockMvc.perform(requestBuilder).andExpect(status().isOk()).andDo(MockMvcResultHandlers.print());
    }

    /**
     * 用例名称：代理主机不存在时，浏览受保护环境上的资源失败
     * 前置条件：spring mvc框架正常运行。
     * check点：rest接口能正常接收到请求体，并抛出对应异常
     */
    @Test
    public void test_query_resources_by_agent_failed_when_agent_does_not_exist() throws Exception {
        String agentId = UUIDGenerator.getUUID();
        when(resourceService.getBasicResourceById(agentId)).thenReturn(Optional.empty());
        String url = "/v2/agents/" + agentId + "/resources";
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(url)
                .queryParam("envId", UUIDGenerator.getUUID())
                .queryParam("pageNo", "1")
                .queryParam("pageSize", "10");
        Assert.assertThrows(Exception.class,
                () -> mockMvc.perform(requestBuilder).andExpect(status().isOk()).andDo(MockMvcResultHandlers.print()));
    }

    /**
     * 用例名称：验证通过查询配置信息的接口正常调用
     * 前置条件：spring mvc框架正常运行。
     * check点：rest接口能正常接收到请求体，并响应正常
     */
    @Test
    public void test_query_app_conf_success() throws Exception {
        when(resourceService.getBasicResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        when(agentService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
                .thenReturn(new PageListResponse<>());
        String url = "/v1/agents/config";
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(url)
                .queryParam("subType", ResourceSubTypeEnum.KUBERNETES_STATEFUL_SET.getType())
                .queryParam("hostUuids",UUIDGenerator.getUUID())
                .queryParam("script", "a");
        mockMvc.perform(requestBuilder).andExpect(status().isOk()).andDo(MockMvcResultHandlers.print());
    }
    /**
     * 用例名称：验证传递任务信息的接口正常调用
     * 前置条件：spring mvc框架正常运行。
     * check点：rest接口能正常接收到请求体，并响应正常
     */
    @Test
    public void test_deliver_task_status_success() throws Exception {
        when(resourceService.getBasicResourceById(anyString()))
                .thenReturn(Optional.of(new ProtectedEnvironment()));
        when(agentService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
                .thenReturn(new PageListResponse<>());
        mockMvc.perform(MockMvcRequestBuilders.post("/v1/agents/task/status")
                .contentType(MediaType.APPLICATION_JSON).content(JSONObject.fromObject(new DeliverTaskReq()).toString()))
                .andDo(print()).andExpect(status().is4xxClientError());
    }
}