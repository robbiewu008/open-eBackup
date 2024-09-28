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
package openbackup.database.base.plugin.utils;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.List;

/**
 * 数据库扫描公共工具测试类
 *
 */
public class DatabaseScannerUtilsTest {
    private final String envId = UUIDGenerator.getUUID();

    private ResourceService resourceService;

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
    }

    /**
     * 用例名称：数据库扫描查询环境下面实例不为空
     * 前置条件：实例注册正常
     * 检查点：成功获取环境下面存在的实例
     */
    @Test
    public void get_instances_by_env_id_when_instances_is_exist() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(getQueryResult("exist"));
        List<ProtectedResource> instancesByEnvironment = DatabaseScannerUtils.getInstancesByEnvironment(envId,
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), resourceService);
        Assert.assertEquals(1, instancesByEnvironment.size());
    }

    /**
     * 用例名称：数据库扫描查询环境下面实例为空
     * 前置条件：实例注册正常
     * 检查点：环境下面没有实例或者获取实例失败
     */
    @Test
    public void get_instances_by_env_id_when_instances_is_empty() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(getQueryResult(""));
        List<ProtectedResource> instancesByEnvironment = DatabaseScannerUtils.getInstancesByEnvironment(envId,
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), resourceService);
        Assert.assertEquals(0, instancesByEnvironment.size());
    }

    private PageListResponse<ProtectedResource> getQueryResult(String flag) {
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        if (!flag.isEmpty()) {
            result.setTotalCount(1);
            ProtectedResource instance = new ProtectedResource();
            result.setRecords(Collections.singletonList(instance));
        }
        return result;
    }
}
