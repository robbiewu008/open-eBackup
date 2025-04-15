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
package openbackup.data.access.framework.protection.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionExecuteDto;
import openbackup.data.protection.access.provider.sdk.protection.service.ProjectedObjectService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.enums.GroupTypeEnum;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.quartz.SchedulerException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.TimeUnit;

/**
 * 保护服务实现
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ProjectedObjectService.class, TokenBo.class})
public class ProjectedObjectServiceTest {
    @Mock
    private JobService jobService;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private ResourceSetApi resourceSetApi;

    @Mock
    private ProtectedObjectMapper protectedObjectMapper;

    @Mock
    private SchedulerService scheduleService;

    @Mock
    private ProjectedTaskService projectedTaskService;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ResourceGroupRepository resourceGroupRepository;

    @Mock
    private ProtectObjectRestApi protectObjectRestApi;

    @InjectMocks
    private ProjectedObjectServiceImpl projectedObjectService;

    @Before
    public void setUp() throws SchedulerException {
        PowerMockito.when(jobService.createJob(any())).thenReturn("jobId");
        SlaDto sla = new SlaDto();
        sla.setPolicyList(new ArrayList<>());
        PowerMockito.when(slaQueryService.querySlaById(any())).thenReturn(sla);
        PowerMockito.when(scheduleService.startScheduler(any())).thenReturn("scheduleId");
        PowerMockito.when(projectedTaskService.saveBatch(any())).thenReturn(true);
        PowerMockito.when(protectedObjectMapper.insertProtectedObject(any())).thenReturn(1);
        PowerMockito.when(resourceGroupRepository.updateStatusById(any(), any())).thenReturn(1);
        PowerMockito.when(resourceService.batchUpdateStatusById(any(), anyInt())).thenReturn(1);
        PowerMockito.doNothing().when(protectObjectRestApi).deleteProtectedObjects(any());
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = new TokenBo();
        tokenBo.setUser(new TokenBo.UserBo());
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);

    }


    /**
     * 测试场景：资源组创建保护
     * 前置条件：无
     * 检查点：执行成功
     */
    @Test
    public void create_group_projected_object_success() throws Exception {
        String jobId = projectedObjectService.createGroupProjectedObject(mockExecuteDto(), (userId) -> {});
        ThreadPoolTool.getPool().awaitTermination(5, TimeUnit.SECONDS);
        Assert.assertEquals(jobId, "jobId");
    }

    /**
     * 测试场景：资源组修改时同步修改保护
     * 前置条件：无
     * 检查点：执行成功
     */
    @Test
    public void update_group_projected_object_success() throws Exception {
        String jobId = projectedObjectService.changeGroupProjectedObject(mockExecuteDto(), Arrays.asList("1", "2", "3"));
        ThreadPoolTool.getPool().awaitTermination(5, TimeUnit.SECONDS);
        Assert.assertEquals(jobId, "jobId");
    }

    private ProtectionExecuteDto mockExecuteDto() {
        ProtectionExecuteDto dto = new ProtectionExecuteDto();
        dto.setExtParameters(new JSONObject());
        dto.setSubResources(new ArrayList<>());
        dto.setResource(mockGroupDto());
        dto.setSlaId("slaId");
        return dto;
    }

    private ResourceGroupDto mockGroupDto() {
        ResourceGroupDto dto = new ResourceGroupDto();
        dto.setGroupType(GroupTypeEnum.RULE.getValue());
        dto.setUuid("groupId");
        dto.setName("groupName");
        dto.setSourceSubType("sourceSubType");
        dto.setSourceType("sourceType");
        return dto;
    }

}
