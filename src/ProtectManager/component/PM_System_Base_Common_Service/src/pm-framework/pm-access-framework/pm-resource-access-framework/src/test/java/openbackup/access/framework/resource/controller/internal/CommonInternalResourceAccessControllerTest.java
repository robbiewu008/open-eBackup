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
package openbackup.access.framework.resource.controller.internal;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;

import openbackup.access.framework.resource.controller.internal.CommonInternalResourceAccessController;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;

import java.util.ArrayList;

@AutoConfigureWebMvc
@AutoConfigureMockMvc
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {CommonInternalResourceAccessController.class})
public class CommonInternalResourceAccessControllerTest {
/**
 * CommonResourceAccessController测试
 *
 */
    @Autowired
    private MockMvc mvc;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private ProtectedResourceMonitorService protectedResourceMonitorService;

    @MockBean
    private CyberEngineResourceService cyberEngineResourceService;

    /**
     * 用例名称：验证资源查询接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：查询符合预期<br/>
     */
    @Test
    public void query_resource_page() throws Exception {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(10);
        pageListResponse.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);
        mvc.perform(get("/v2/internal/resources")
            .param("size", "3")
            .param("page", "0"))
            .andExpect(jsonPath("$.totalCount").exists());
    }

    /**
     * 用例名称：验证根据uuid查询资源接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：查询符合预期<br/>
     */
    @Test
    public void query_resource_page_by_resource_id() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("123");
        Mockito.when(resourceService.getResourceById(false, "123"))
            .thenReturn(java.util.Optional.of(protectedResource));
        mvc.perform(get("/v2/internal/resources/123"))
            .andExpect(MockMvcResultMatchers.status().isOk())
            .andReturn();
    }
    /**
     * 用例名称：验证清理下次备份信息。<br/>
     * 前置条件：无。<br/>
     * check点：清除符合预期<br/>
     */
    @Test
    public void clean_next_backup_success() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("123");
        Mockito.doNothing().when(resourceService).cleanNextBackup("123");
        mvc.perform(put("/v2/internal/resources/{resourceId}/action/clean-next-backup","123"))
                .andExpect(MockMvcResultMatchers.status().isOk())
                .andReturn();
    }

    /**
     * 用例名称：验证根据uuid查询资源下次备份类型和原因。<br/>
     * 前置条件：无。<br/>
     * check点：查询符合预期<br/>
     */
    @Test
    public void query_next_backup_type_and_cause_success() throws Exception {
        NextBackupParams extParams = new NextBackupParams(
                NextBackupChangeCauseEnum.LATEST_FULL_OR_INCREMENTAL_COPY_DELETE_SUCCESS_TO_FULL_LABEL.getLabel());
        Mockito.when(resourceService.queryNextBackupTypeAndCause( "123"))
                .thenReturn(extParams);
        mvc.perform(get("/v2/internal/resources/123/next-backup-type-and-cause"))
                .andExpect(MockMvcResultMatchers.status().isOk())
                .andReturn();
    }
}
