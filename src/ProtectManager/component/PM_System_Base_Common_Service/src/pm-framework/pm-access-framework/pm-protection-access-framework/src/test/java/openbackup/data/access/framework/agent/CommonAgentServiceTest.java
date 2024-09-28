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
package openbackup.data.access.framework.agent;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;

import openbackup.data.access.framework.agent.CommonAgentServiceImpl;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;
import com.huawei.oceanprotect.job.sdk.JobService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * CommonAgentService测试类
 *
 */
public class CommonAgentServiceTest {
    private final String subNetFixedIpKey = "subNetFixedIp";
    private CommonAgentService commonAgentService;

    @Before
    public void init() {
        ResourceExtendInfoService resourceExtendInfoService = Mockito.mock(ResourceExtendInfoService.class);
        MemberClusterService memberClusterService = Mockito.mock(MemberClusterService.class);
        JobService jobService = Mockito.mock(JobService.class);
        StorageUnitService storageUnitService = Mockito.mock(StorageUnitService.class);
        commonAgentService = new CommonAgentServiceImpl(resourceExtendInfoService, memberClusterService, jobService,
            storageUnitService);

        Map<String, List<ProtectedResourceExtendInfo>> extendInfoMap = new HashMap<>();
        List<ProtectedResourceExtendInfo> extendInfoList = new ArrayList<>();
        extendInfoMap.put("1", extendInfoList);

        ProtectedResourceExtendInfo protectedResourceExtendInfo = new ProtectedResourceExtendInfo();
        protectedResourceExtendInfo.setUuid("1");
        protectedResourceExtendInfo.setKey(subNetFixedIpKey);
        protectedResourceExtendInfo.setValue("1.1.1.1");
        extendInfoList.add(protectedResourceExtendInfo);

        Mockito.when(resourceExtendInfoService.queryExtendInfoByResourceIds(Mockito.any())).thenReturn(extendInfoMap);
    }

    /**
     * 用例名称：添加agent信息成功<br/>
     * 前置条件：无<br/>
     * check点：agent扩展信息符合预期<br/>
     */
    @Test
    public void supply_agent_info_success() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId("1");
        List<Endpoint> endpoints = new ArrayList<>(Collections.singletonList(endpoint));
        commonAgentService.supplyAgentCommonInfo(endpoints);

        Assert.assertEquals(endpoints.get(0).getAdvanceParams().get(subNetFixedIpKey), "1.1.1.1");
    }
}
