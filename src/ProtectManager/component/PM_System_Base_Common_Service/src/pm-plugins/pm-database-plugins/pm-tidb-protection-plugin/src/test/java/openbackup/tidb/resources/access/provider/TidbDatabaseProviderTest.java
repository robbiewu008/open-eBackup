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
package openbackup.tidb.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * TidbDatabaseProviderTest
 *
 * @author w00426202
 * @since 2023-07-15
 */
@RunWith(PowerMockRunner.class)
public class TidbDatabaseProviderTest {
    @Mock
    private TidbService tidbService;

    private TidbDatabaseProvider tidbDatabaseProvider;

    @Mock
    private JsonSchemaValidator jsonSchemaValidator;

    @Mock
    private ResourceService resourceService;

    @Before
    public void setUp() {
        tidbDatabaseProvider = new TidbDatabaseProvider(tidbService, jsonSchemaValidator,resourceService);
    }

    /**
     * 用例场景：测试类型判断
     * 前置条件：
     * 检查点：返回false
     */
    @Test
    public void test_applicable() {
        ProtectedResource resource = new ProtectedResource();
        Assert.assertFalse(tidbDatabaseProvider.applicable(resource));
    }

    @Test
    @Ignore
    public void test_beforeCreate() throws IOException {
        ProtectedResource protectedResource = getDatabaseReqFromJsonFile();
        PowerMockito.doNothing().when(resourceService).updateSubResource(any(), any());
        tidbDatabaseProvider.beforeCreate(protectedResource);
    }

    @Test
    public void test_beforeUpdate() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("tidb");
        Map<String, String> map = new HashMap<>();
        map.put(TidbConstants.TIUP_PATH, "/temp/tidb");
        protectedResource.setExtendInfo(map);
        protectedResource.setUuid("123");
        protectedResource.setParentUuid("456");


        ProtectedResource src = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TidbConstants.TIUP_PATH, "/temp/tidb");
        src.setExtendInfo(extendInfo);

        ProtectedResource target = new ProtectedResource();
        Map<String, String> targetExtendInfo = new HashMap<>();
        target.setExtendInfo(targetExtendInfo);

        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(src);
        PowerMockito.doNothing().when(tidbService).checkResourceStatus(any());
        PowerMockito.doNothing().when(tidbService).checkDuplicateResource(anyList(), any());

        PowerMockito.when(tidbService.getAgentResource(any())).thenReturn(target);
        tidbDatabaseProvider.beforeCreate(protectedResource);
    }

    public ProtectedEnvironment getDatabaseReqFromJsonFile() throws IOException {
        URL resource = Thread.currentThread().getContextClassLoader().getResource("Database-req.json");
        File file = new File(Objects.requireNonNull(resource).getPath());
        String masterReqStr = FileUtils.readFileToString(file, Charset.defaultCharset());
        ProtectedEnvironment ProtectedEnvironment = JsonUtil.read(masterReqStr, ProtectedEnvironment.class);
        return ProtectedEnvironment;
    }
}
