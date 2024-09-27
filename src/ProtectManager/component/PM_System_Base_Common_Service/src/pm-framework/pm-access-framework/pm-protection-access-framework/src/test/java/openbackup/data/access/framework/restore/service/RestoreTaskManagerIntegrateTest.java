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
package openbackup.data.access.framework.restore.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.KmcHelperMocker;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.protection.mocks.MockRestoreInterceptorProvider;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.service.DeployTypeService;

import com.huawei.oceanprotect.system.base.user.service.UserService;

import org.assertj.core.util.Maps;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.retry.annotation.EnableRetry;
import org.springframework.test.context.TestPropertySource;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Collections;
import java.util.UUID;

/**
 * RestoreTaskManager类集成测试，主要验证Retry & Recover机制
 *
 * @since 2022-09-02
 */
@EnableRetry
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {RestoreTaskManager.class})
@PrepareForTest(value = {RestoreTaskManager.class, KmcHelper.class})
@MockBean(classes = {RestoreValidateService.class, CopyVerifyTaskManager.class, DeployTypeService.class})
@TestPropertySource(properties = {"spring.config.location = classpath:application-test.yaml"})
public class RestoreTaskManagerIntegrateTest extends KmcHelperMocker {
    @Autowired
    private RestoreTaskManager restoreTaskManager;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private RestoreTaskService restoreTaskService;

    @MockBean
    private JobLogRecorder jobLogRecorder;

    @MockBean
    private RestoreResourceService restoreResourceService;

    @MockBean
    private UserService userService;

    @MockBean
    private SanClientService sanClientService;

    @MockBean
    private JobService jobService;

    @MockBean
    private CommonAgentService commonAgentService;

    @MockBean
    private IVpcService iVpcService;

    /**
     * 用例场景：测试restoreTaskService complete函数。
     * 前置条件：updateCopyStatus更新副本抛出RetryableException。构造complet失败超时。
     * 检查点：重试三次，recoverStart函数被调用，无异常抛出。
     */
    @Test
    public void should_run_recovery_after_retry_3times_if_copyService_fail_when_start() {
        // Given
        String mockJobId = UUID.randomUUID().toString();
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask mockRestoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        mockRestoreTask.setRequestId(mockJobId);
        TaskResource mockResource = new TaskResource();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskResource(), mockResource);
        mockRestoreTask.setTargetObject(mockResource);
        final TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskEnv(), taskEnvironment);
        mockRestoreTask.setTargetEnv(taskEnvironment);
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        given(restoreTaskService.lockResources(any(), any(), anyList())).willReturn(true);
        given(providerManager.findProvider(eq(RestoreInterceptorProvider.class), any(), any())).willReturn(
            new MockRestoreInterceptorProvider());
        given(restoreResourceService.getLanFreeConfig(any(), any())).willReturn(Maps.newHashMap("123", "true"));
        JobBo jobBo = new JobBo();
        jobBo.setStatus(JobStatusEnum.READY.name());
        given(jobService.queryJob(any())).willReturn(jobBo);
        given(jobService.isJobPresent(anyString())).willReturn(true);
        given(restoreResourceService.queryEndpoints(isNull(), anyString(), anyString(), any()))
            .willReturn(Collections.singletonList(new Endpoint()));
        doThrow(new RuntimeException("proxy error")).when(restoreTaskService).startTask(any());
        // When
        restoreTaskManager.start(mockRestoreTask);
        // Then recovery函数验证recordJobStepWithError调用1次
        verify(restoreTaskService, times(3)).startTask(any());
        verify(jobLogRecorder, times(1)).recordJobStepWithError(any(), any(), any(), any());
    }
}
