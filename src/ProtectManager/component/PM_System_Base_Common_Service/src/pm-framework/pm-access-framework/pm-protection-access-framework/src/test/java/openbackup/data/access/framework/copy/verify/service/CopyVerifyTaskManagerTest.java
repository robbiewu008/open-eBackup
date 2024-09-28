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
package openbackup.data.access.framework.copy.verify.service;

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.data.access.framework.agent.AgentSelectorManager;
import openbackup.data.access.framework.copy.verify.constant.CopyVerifyJobLabelConstant;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.CopyVerifyTaskMocker;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.restore.service.RestoreResourceService;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.ReflectUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import io.jsonwebtoken.lang.Maps;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.lang.reflect.Field;
import java.util.Collections;
import java.util.Optional;
import java.util.UUID;

/**
 * CopyVerifyTaskManager类的单元测试集合
 *
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class CopyVerifyTaskManagerTest {
    private final CopyVerifyService copyVerifyService = Mockito.mock(CopyVerifyService.class);

    private final JobLogRecorder jobLogRecorder = Mockito.mock(JobLogRecorder.class);

    private final AgentSelectorManager agentSelectorManager = Mockito.mock(AgentSelectorManager.class);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final SanClientService sanClientService = Mockito.mock(SanClientService.class);

    @Mock
    private JobService jobService;

    @Mock
    private IVpcService vpcService;

    @Mock
    private DeployTypeService deployTypeService;


    @Mock
    private RestoreResourceService restoreResourceService;

    private ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final CopyVerifyTaskManager copyVerifyTaskManager = new CopyVerifyTaskManager(copyVerifyService,
        jobLogRecorder, agentSelectorManager, providerManager, resourceService);

    @Before
    public void init(){
        CommonAgentService commonAgentService = Mockito.mock(CommonAgentService.class);
        copyVerifyTaskManager.setCommonAgentService(commonAgentService);
    }

    /**
     * 用例名称：验证副本校验任务初始化成功<br/>
     * 前置条件：无<br/>
     * check点：返回的jobId符合预期<br/>
     */
    @Test
    public void should_init_success_when_init() {
        // given
        String requestId = UUID.randomUUID().toString();
        String mockCopyId = UUID.randomUUID().toString();
        given(copyVerifyService.getCopyDetail((mockCopyId))).willReturn(CopyMocker.mockHcsCopy());
        given(agentSelectorManager.selectAgentsByCopy(any())).willReturn(
            Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999)));
        given(copyVerifyService.createJob(any())).willReturn(requestId);
        given(jobService.isJobPresent(anyString())).willReturn(true);
        // when
        String jobId = copyVerifyTaskManager.init(mockCopyId, null);
        // then
        Assert.assertEquals(jobId, requestId);
    }

    /**
     * 用例名称：验证副本校验任务初始化成功<br/>
     * 前置条件：接口下发了agents<br/>
     * check点：返回的jobId符合预期<br/>
     */
    @Test
    public void should_init_success_if_agents_are_selected_when_init() {
        given(jobService.isJobPresent(anyString())).willReturn(true);
        String requestId = UUID.randomUUID().toString();
        String mockCopyId = UUID.randomUUID().toString();
        given(copyVerifyService.getCopyDetail((mockCopyId))).willReturn(CopyMocker.mockHcsCopy());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        agent.setUuid("agent_01");
        agent.setEndpoint("1.1.1.1");
        agent.setPort(1234);
        agent.setOsType("linux");
        given(resourceService.getResourceById(any())).willReturn(Optional.of(agent));
        given(copyVerifyService.createJob(any())).willReturn(requestId);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        // when
        String jobId = copyVerifyTaskManager.init(mockCopyId, "agent_01");
        // then
        Assert.assertEquals(jobId, requestId);
    }

    /**
     * 用例名称：验证副本校验任务执行成功<br/>
     * 前置条件：无<br/>
     * check点：相关函数调用次数符合预期<br/>
     */
    @Test
    public void should_successful_when_execute() throws IllegalAccessException {
        copyVerifyTaskManager.setSanClientService(sanClientService);
        Mockito.when(deployTypeService.isCyberEngine()).thenReturn(false);
        Whitebox.setInternalState(copyVerifyTaskManager, "deployTypeService", deployTypeService);
        Whitebox.setInternalState(copyVerifyTaskManager, "restoreResourceService", restoreResourceService);
        Mockito.when(restoreResourceService.queryEndpoints(anyMap(),anyString(),anyString(),any())).thenReturn(Collections.singletonList(new Endpoint()));
        // given
        given(jobService.isJobPresent(anyString())).willReturn(true);
        String requestId = UUID.randomUUID().toString();
        String taskId = UUID.randomUUID().toString();
        given(copyVerifyService.lockResource(anyString(), anyString(), any())).willReturn(true);
        CopyVerifyTask copyVerifyTask = new CopyVerifyTask();
        copyVerifyTask.setRequestId(requestId);
        copyVerifyTask.setTaskId(taskId);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setSubType(ResourceSubTypeEnum.FILESET.getType());
        copyVerifyTask.setTargetEnv(taskEnvironment);
        given(copyVerifyService.getCopyDetail(Mockito.any())).willReturn(CopyMocker.mockHcsCopy());
        // when
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        Field opServiceHelperField = ReflectUtil.field(CopyVerifyTaskManager.class, "opServiceHelper", OpServiceHelper.class);
        opServiceHelperField.setAccessible(true);
        opServiceHelperField.set(copyVerifyTaskManager, opServiceHelper);
        copyVerifyTaskManager.execute(copyVerifyTask);
        // then
        verify(copyVerifyService, times(1)).start(any());
        verify(copyVerifyService, times(1)).modifyJobProgressRange(any());
        verify(jobLogRecorder, times(1)).recordJobStep(eq(requestId), eq(JobLogLevelEnum.INFO),
            eq(CopyVerifyJobLabelConstant.COPY_CHECK_INIT), isNull());
        verify(jobLogRecorder, times(1)).recordJobStep(eq(requestId), eq(JobLogLevelEnum.INFO),
            eq(CopyVerifyJobLabelConstant.COPY_CHECK_START), isNull());
    }

    /**
     * 用例名称：任务结果成功时，正确执行任务成功逻辑<br/>
     * 前置条件：无<br/>
     * check点：相关函数调用次数符合预期<br/>
     */
    @Test
    public void should_successful_when_complete_given_task_is_main_task_and_status_is_success() {
        // given
        given(jobService.isJobPresent(anyString())).willReturn(true);
        String requestId = UUID.randomUUID().toString();
        String copyId = UUID.randomUUID().toString();
        CopyVerifyTask copyVerifyTask = CopyVerifyTaskMocker.mockTask(requestId, copyId);
        TaskCompleteMessageBo completeMessageBo = new TaskCompleteMessageBo();
        completeMessageBo.setJobRequestId(requestId);
        completeMessageBo.setJobId(requestId);
        completeMessageBo.setJobStatus(DmeJobStatusEnum.SUCCESS.getTypeName());
        // when
        copyVerifyTaskManager.complete(completeMessageBo, copyVerifyTask);
        // then
        verify(jobLogRecorder, times(1)).recordJobStep(eq(requestId), any(),
            eq(CopyVerifyJobLabelConstant.COPY_CHECK_COMPLETE), any());
        verify(copyVerifyService, times(1)).completeJob(eq(requestId), any());
        verify(copyVerifyService, never()).executeRestoreTask(requestId);
    }

    /**
     * 用例名称：任务结果失败并且副本未损坏时，正确执行任务失败逻辑<br/>
     * 前置条件：无<br/>
     * check点：相关函数调用次数符合预期<br/>
     */
    @Test
    public void should_failed_when_complete_given_task_status_is_not_success_and_copy_not_damaged() {
        // given
        given(jobService.isJobPresent(anyString())).willReturn(true);
        String requestId = UUID.randomUUID().toString();
        String copyId = UUID.randomUUID().toString();
        CopyVerifyTask copyVerifyTask = CopyVerifyTaskMocker.mockTask(requestId, copyId);
        TaskCompleteMessageBo completeMessageBo = new TaskCompleteMessageBo();
        completeMessageBo.setJobRequestId(requestId);
        completeMessageBo.setJobId(requestId);
        completeMessageBo.setJobStatus(DmeJobStatusEnum.FAIL.getTypeName());
        completeMessageBo.setExtendsInfo(Maps.of(TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED, "false").build());
        Copy copy = new Copy();
        copy.setUuid(copyId);
        given(copyRestApi.queryCopyByID(Mockito.anyString())).willReturn(copy);
        given(copyRestApi.queryLatestBackupCopy(Mockito.any(), Mockito.any(), Mockito.any()))
                .willReturn(copy);
        // when
        copyVerifyTaskManager.complete(completeMessageBo, copyVerifyTask);
        // then
        verify(jobLogRecorder, times(1)).recordJobStep(eq(requestId), any(),
            eq(CopyVerifyJobLabelConstant.COPY_CHECK_COMPLETE), any());
        verify(copyVerifyService, times(1)).completeJob(eq(requestId), any());
        verify(copyVerifyService, times(1)).unlockResource(requestId, requestId, true);
        verify(copyVerifyService, never()).modifyCopyStatus(eq(copyId), eq(CopyStatus.INVALID));
        verify(resourceService, times(0)).modifyNextBackup(any());
    }

    /**
     * 用例名称：任务结果失败并且副本损坏时，正确执行任务失败逻辑<br/>
     * 前置条件：无<br/>
     * check点：相关函数调用次数符合预期<br/>
     */
    @Test
    public void should_failed_when_complete_given_task_status_is_not_success_and_copy_damaged() {
        // given
        given(jobService.isJobPresent(anyString())).willReturn(true);
        String requestId = UUID.randomUUID().toString();
        String copyId = UUID.randomUUID().toString();
        CopyVerifyTask copyVerifyTask = CopyVerifyTaskMocker.mockTask(requestId, copyId);
        TaskCompleteMessageBo completeMessageBo = new TaskCompleteMessageBo();
        completeMessageBo.setJobRequestId(requestId);
        completeMessageBo.setJobId(requestId);
        completeMessageBo.setJobStatus(DmeJobStatusEnum.FAIL.getTypeName());
        completeMessageBo.setExtendsInfo(Maps.of(TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED, "true").build());
        Copy copy = new Copy();
        copy.setUuid(copyId);
        given(copyRestApi.queryLatestBackupCopy(Mockito.any(), Mockito.any(), Mockito.any()))
                .willReturn(copy);
        given(copyRestApi.queryCopyByID(Mockito.anyString())).willReturn(copy);
        // when
        copyVerifyTaskManager.complete(completeMessageBo, copyVerifyTask);
        // then
        verify(copyVerifyService, times(1)).modifyCopyStatus(eq(copyId), eq(CopyStatus.INVALID));
    }
}
