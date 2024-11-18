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

import com.google.common.collect.Lists;
import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.access.framework.protection.mocks.TaskResourceMocker;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.lock.LockRequest;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.beans.BeanUtils;

import java.util.ArrayList;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.anyString;

/**
 * 恢复任务辅助工具类
 *
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KmcHelper.class})
public class RestoreTaskHelperTest {
    /**
     * 用例名称：验证创建恢复任务成功<br/>
     * 前置条件：无<br/>
     * check点：返回的jobId为指定的jobId<br/>
     */
    @Test
    public void should_return_job_id_when_createJob_given_correct_request() {
        // Given
        final String passwdEncrypted = "Admin@123456789";
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask mockRestoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        final Copy mockCopy = CopyMocker.mockCommonCopy();
        mockCopy.setStatus(CopyStatus.INVALID.getValue());
        final TaskResource taskResource = TaskResourceMocker.mockFullInfo();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskEnv(), taskEnvironment);
        mockRestoreTask.setTargetEnv(taskEnvironment);
        mockRestoreTask.setTargetObject(taskResource);
        RestoreTaskContext mockContext = new RestoreTaskContext();
        mockContext.setTaskRequest(mockRequest);
        mockContext.setRestoreTask(mockRestoreTask);
        mockContext.setCopy(mockCopy);
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
        PowerMockito.when(kmcHelper.encrypt(anyString())).thenReturn(passwdEncrypted);
        // When
        final CreateJobRequest jobRequest = RestoreTaskHelper.buildRestoreJobRequest(mockContext);
        // Then
        Assert.assertFalse(jobRequest.isEnableStop());
        Assert.assertEquals(mockRestoreTask.getTargetEnv().getAuth().getAuthPwd(), passwdEncrypted);
        Assert.assertEquals(jobRequest.getExtendField().get("restoreTargetType"), null);
        Assert.assertEquals(jobRequest.getExtendField().get("copyOriginalStatus"), "Invalid");
    }

    @Test
    public void should_return_task_resource_when_buildTaskResourceByCopy() {
        //Given
        final Copy copy = CopyMocker.mockHdfsCopy();
        //When
        final TaskResource taskResource = RestoreTaskHelper.buildTaskResourceByCopy(copy);
        //Then
        Assert.assertNotNull(taskResource);
        Assert.assertEquals(taskResource.getName(), "yangtest");
    }

    @Test
    public void should_return_req_when_buildUpdateCopyStatusReq() {
        //Given
        final CopyStatus status = CopyStatus.RESTORING;
        //When
        final CopyStatusUpdateParam updateParam = RestoreTaskHelper.buildUpdateCopyStatusReq(status);
        //Then
        Assert.assertNotNull(updateParam);
        Assert.assertEquals(updateParam.getStatus(), status);
    }

    @Test
    public void should_lock_three_resource_when_lockResources_given_two_resources() {
        //Given
        final String requestId = UUID.randomUUID().toString();
        final String copyId = UUID.randomUUID().toString();
        final ArrayList<LockResourceBo> lockResources = Lists.newArrayList(new LockResourceBo("1", LockType.WRITE),
            new LockResourceBo("2", LockType.READ));
        //When
        final LockRequest lockRequest= RestoreTaskHelper.buildLockRequest(requestId, copyId, lockResources);
        //Then
        Assert.assertEquals(lockRequest.getRequestId(), requestId);
        Assert.assertEquals(lockRequest.getLockId(), requestId);
        Assert.assertNotNull(lockRequest.getResources());
    }

    @Test
    public void should_return_restore_task_when_parseFromJobMessage() {
        //Given
        final String taskId = UUID.randomUUID().toString();
        final JobMessage jobMessage = new JobMessage();
        final RestoreTask givenTask = new RestoreTask();
        givenTask.setTaskId(taskId);
        jobMessage.setPayload(JSONObject.fromObject(givenTask));
        //When
        final RestoreTask restoreTask = RestoreTaskHelper.parseFromJobMessage(JSONObject.fromObject(jobMessage).toString());
        //Then
        Assert.assertNotNull(restoreTask);
        Assert.assertEquals(restoreTask.getTaskId(), taskId);
    }

    @Test
    public void should_return_job_request_when_buildJobRequestWithData() {
        //Given
        JSONObject data = new JSONObject();
        data.set("enableStop", true);
        //When
        final UpdateJobRequest updateJobRequest = RestoreTaskHelper.buildJobRequestWithData(data);
        //Then
        Assert.assertNotNull(updateJobRequest);
        Assert.assertTrue((Boolean) updateJobRequest.getData().get("enableStop"));
    }

    @Test
    public void should_return_job_request_when_buildJobRequestWithStatus() {
        //Given
        final ProviderJobStatusEnum givenStatus = ProviderJobStatusEnum.PARTIAL_SUCCESS;
        //When
        final UpdateJobRequest updateJobRequest = RestoreTaskHelper.buildJobRequestWithStatus(givenStatus);
        //Then
        Assert.assertNotNull(updateJobRequest);
        Assert.assertEquals(updateJobRequest.getStatus(), JobStatusEnum.PARTIAL_SUCCESS);
    }
}