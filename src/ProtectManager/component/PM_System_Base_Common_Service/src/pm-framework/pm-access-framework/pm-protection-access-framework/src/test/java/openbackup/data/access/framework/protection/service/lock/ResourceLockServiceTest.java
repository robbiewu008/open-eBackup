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
package openbackup.data.access.framework.protection.service.lock;

import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.protection.service.lock.ResourceLockService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockResource;
import openbackup.system.base.sdk.lock.LockResponse;
import openbackup.system.base.sdk.lock.LockTypeEnum;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.service.DeployTypeService;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.UUID;

import static org.assertj.core.api.BDDAssertions.thenThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;

/**
 * 资源锁服务测试用例集合
 *
 **/
public class ResourceLockServiceTest {

    private final ResourceLockRestApi resourceLockRestApi = Mockito.mock(ResourceLockRestApi.class);

    private final JobLogRecorder jobLogRecorder = Mockito.mock(JobLogRecorder.class);

    private final DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);

    private final ResourceLockService resourceLockService = new ResourceLockService(resourceLockRestApi,
        jobLogRecorder, deployTypeService);

    /**
     * 用例名称：资源加锁成功<br/>
     * 前置条件：无<br/>
     * check点：验证加锁结果为true<br/>
     */
    @Test
    public void should_return_true_when_lock_resource() {
        //Given
        String requestId = UUID.randomUUID().toString();
        String lockId = UUID.randomUUID().toString();
        LockRequest request = new LockRequest();
        request.setRequestId(requestId);
        request.setLockId(lockId);
        request.setPriority(1);
        request.setResources(Collections.singletonList(new LockResource("resource1", LockTypeEnum.WRITE)));
        LockResponse lockResponse = new LockResponse();
        lockResponse.setSuccess(Boolean.TRUE);
        given(resourceLockRestApi.lock(any())).willReturn(lockResponse);
        //When
        final boolean result = resourceLockService.lock(request);
        //Then
        Assert.assertTrue(result);
    }

    /**
     * 用例名称：资源锁加锁失败<br/>
     * 前置条件：无<br/>
     * check点：验证加锁结果为false<br/>
     */
    @Test
    public void should_return_false_when_lock_resource_failed() {
        //Given
        String requestId = UUID.randomUUID().toString();
        String lockId = UUID.randomUUID().toString();
        LockRequest request = new LockRequest();
        request.setRequestId(requestId);
        request.setLockId(lockId);
        request.setPriority(1);
        request.setResources(Collections.singletonList(new LockResource("resource1", LockTypeEnum.WRITE)));
        LockResponse lockResponse = new LockResponse();
        lockResponse.setSuccess(Boolean.FALSE);
        given(resourceLockRestApi.lock(any())).willReturn(lockResponse);
        //When
        final boolean result = resourceLockService.lock(request);
        //Then
        Assert.assertFalse(result);
    }

    /**
     * 用例名称：资源加锁失败<br/>
     * 前置条件：无<br/>
     * check点：验证加锁结果为false<br/>
     */
    @Test
    public void should_return_false_when_lock_resource_given_api_exception() {
        //Given
        String requestId = UUID.randomUUID().toString();
        String lockId = UUID.randomUUID().toString();
        LockRequest request = new LockRequest();
        request.setRequestId(requestId);
        request.setLockId(lockId);
        request.setPriority(1);
        request.setResources(Collections.singletonList(new LockResource("resource1", LockTypeEnum.WRITE)));
        given(resourceLockRestApi.lock(any())).willThrow(new RuntimeException("time ount"));
        //When
        final boolean result = resourceLockService.lock(request);
        //Then
        Assert.assertFalse(result);
    }

    /**
     * 用例名称：资源解锁成功<br/>
     * 前置条件：无<br/>
     * check点：未抛出异常，接口调用次数符合预期<br/>
     */
    @Test
    public void should_success_when_unlock_resource() {
        //Given
        String requestId = UUID.randomUUID().toString();
        String lockId = UUID.randomUUID().toString();
        //When
        resourceLockService.unlock(requestId, lockId, true);
        //Then
        Mockito.verify(resourceLockRestApi, times(1)).unlock(eq(lockId), eq(requestId));
        Mockito.verify(jobLogRecorder, times(2)).recordJobStep(any(), any(), any(), any());
    }

    /**
     * 用例名称：资源解锁失败<br/>
     * 前置条件：无<br/>
     * check点：抛出异常符合预期，接口调用次数符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_unlock_resource_failed() {
        //Given
        String requestId = UUID.randomUUID().toString();
        String lockId = UUID.randomUUID().toString();
        doThrow(new RuntimeException("timeout")).when(resourceLockRestApi).unlock(any(), any());
        //When and Then
        thenThrownBy(() -> resourceLockService.unlock(requestId, lockId, true)).isInstanceOf(LegoCheckedException.class)
            .hasMessage("unlock resource failed");
        Mockito.verify(resourceLockRestApi, times(1)).unlock(eq(lockId), eq(requestId));
        Mockito.verify(jobLogRecorder, times(2)).recordJobStep(any(), any(), any(), any());
    }
}