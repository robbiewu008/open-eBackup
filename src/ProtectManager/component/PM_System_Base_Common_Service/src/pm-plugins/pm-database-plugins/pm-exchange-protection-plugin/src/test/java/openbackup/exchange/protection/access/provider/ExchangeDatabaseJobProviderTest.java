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
package openbackup.exchange.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedResourceServiceImpl;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.jupiter.api.BeforeEach;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * ExchangeDatabaseJobProvider Test
 *
 * @author w30032137
 * @since 2024-06-17
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ExchangeDatabaseJobProvider.class})
public class ExchangeDatabaseJobProviderTest {
    private final ResourceService resourceService = Mockito.mock(ProtectedResourceServiceImpl.class);

    private ExchangeDatabaseJobProvider providerTest = new ExchangeDatabaseJobProvider(resourceService);

    @BeforeEach
    void setUp() {
        providerTest = new ExchangeDatabaseJobProvider(resourceService);
    }

    /**
     * 用例场景：测试资源能否执行
     * 前置条件：无
     * 检查点：只对exchange database生效
     */
    @Test
    public void testApplicable() {
        Assert.assertTrue(providerTest.applicable(ResourceSubTypeEnum.EXCHANGE_DATABASE.getType()));
        Assert.assertFalse(providerTest.applicable(ResourceSubTypeEnum.EXCHANGE_GROUP.getType()));
        Assert.assertFalse(providerTest.applicable(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType()));
        Assert.assertFalse(providerTest.applicable(ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType()));
    }

    /**
     * 用例场景：测试job扩展信息中塞入isGroup字段成功。
     * 前置条件：无
     * 检查点：数据库属于dag
     */
    @Test
    public void testIntercept() {
        Job insertJob = getJobInfo();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("isGroup", "1");
        protectedEnvironment.setExtendInfo(extendInfo);
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(protectedEnvironment);
        when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        providerTest.intercept(insertJob);
        JSONObject extendStr = JSONObject.fromObject(insertJob.getExtendStr());
        String isGroup = (String) extendStr.get("isGroup");
        Assert.assertEquals("1", isGroup);
    }

    private Job getJobInfo() {
        Job insertJob = new Job();
        insertJob.setType(JobTypeEnum.BACKUP.getValue());
        insertJob.setSourceSubType(ResourceSubTypeEnum.EXCHANGE_DATABASE.getType());
        String extend = "{\"endpoint\": \"\",\n" + "    \"port\": 0}";
        insertJob.setExtendStr(extend);
        return insertJob;
    }
}