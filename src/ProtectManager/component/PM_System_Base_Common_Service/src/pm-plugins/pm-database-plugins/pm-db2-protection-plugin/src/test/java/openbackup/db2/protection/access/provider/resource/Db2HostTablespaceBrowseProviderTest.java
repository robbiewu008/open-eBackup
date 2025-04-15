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
package openbackup.db2.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * {@link Db2HostTablespaceBrowseProvider} 测试类
 *
 */
public class Db2HostTablespaceBrowseProviderTest {
    private final Db2TablespaceService db2TablespaceService = PowerMockito.mock(Db2TablespaceService.class);

    private final Db2HostTablespaceBrowseProvider provider = new Db2HostTablespaceBrowseProvider(db2TablespaceService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_tablespace_browse_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：浏览db2表空间
     * 前置条件：无
     * 检查点：浏览成功
     */
    @Test
    public void execute_db2_tablespace_browse_success() {
        PowerMockito.when(db2TablespaceService.querySingleTablespace(any(), any()))
            .thenReturn(mockTablespace());
        PageListResponse<ProtectedResource> response = provider.browse(mockEnv(), mockConditions());
        Assert.assertEquals(1, response.getTotalCount());
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        return environment;
    }

    private BrowseEnvironmentResourceConditions mockConditions() {
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setPageNo(0);
        conditions.setPageSize(100);
        conditions.setResourceType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        return conditions;
    }

    private PageListResponse<ProtectedResource> mockTablespace() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        ProtectedResource tablespace = new ProtectedResource();
        tablespace.setName("test");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(tablespace);
        response.setRecords(list);
        return response;
    }
}