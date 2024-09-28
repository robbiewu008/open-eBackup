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
package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.access.client.sdk.api.framework.dee.DeeCopiesManagementRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.copy.mng.service.DeeCopyService;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.repository.tapelibrary.common.util.JsonUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.Codec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = DeeCopyService.class)
public class DeeCopyServiceTest {

    @Autowired
    private DeeCopyService deeCopyService;

    @MockBean
    private JobService jobService;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private DeeCopiesManagementRestApi deeCopiesManagementRestApi;

    /**
     * 用例名称：执行快照备份
     * 前置条件：无
     * check点：调用执行快照
     */
    @Test
    public void list_available_time_ranges_success() {
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId("requestId");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId("resourceId");
        backupObject.setProtectedObject(protectedObject);

        PolicyBo policyBo = new PolicyBo();
        RetentionBo retention = new RetentionBo();
        retention.setDurationUnit("y");
        retention.setRetentionDuration(1);
        policyBo.setRetention(retention);
        RMap rMap = Mockito.mock(RMap.class);
        rMap.put(CopyConstants.POLICY, JsonUtil.toJsonString(policyBo));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("rootUUID");
        protectedResource.setName("resourceName");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_ID, "System_Vstore");
        extendInfo.put(CopyResourcePropertiesConstant.PROTECTED_RESOURCE_FILESYSTEM_ID, "10001");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optionalProtectedResource = Optional.of(protectedResource);

        PowerMockito.when(resourceService.getResourceById("resourceId")).thenReturn(optionalProtectedResource);
        deeCopyService.hyperDetectBackup(backupObject);
        Mockito.verify(jobService, Mockito.times(1)).updateJob(Mockito.any(), Mockito.any());
    }
}
