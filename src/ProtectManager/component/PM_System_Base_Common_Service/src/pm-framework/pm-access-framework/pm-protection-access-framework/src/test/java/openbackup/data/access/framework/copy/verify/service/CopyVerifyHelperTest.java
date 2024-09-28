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
import static org.assertj.core.api.BDDAssertions.thenThrownBy;

import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.UpdateCopyPropertiesRequest;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobMessage;
import openbackup.system.base.sdk.lock.LockRequest;
import openbackup.system.base.sdk.lock.LockTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Locale;
import java.util.UUID;

/**
 * 副本校验辅助类单元测试集合
 *
 **/
public class CopyVerifyHelperTest {

    /**
     * 用例名称：验证构建副本校验任务请求成功<br/>
     * 前置条件：无<br/>
     * check点：验证创建请求中的值符合预期<br/>
     */
    @Test
    public void should_build_successful_when_buildCreateJobReq() {
        // given
        String mockRequestId = UUID.randomUUID().toString();
        Copy mockCopy = CopyMocker.mockNasCopy();
        ArrayList<Endpoint> mockEndpoints = Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999));
        CopyVerifyTask copyVerifyTask = mockTask(mockRequestId, mockCopy, mockEndpoints);
        CopyVerifyManagerContext mockContext = getMockContext(mockRequestId, mockCopy, copyVerifyTask);
        // when
        CreateJobRequest actualRequest = CopyVerifyHelper.buildCreateJobReq(mockContext);
        // then
        Assert.assertEquals(actualRequest.getCopyId(), mockCopy.getUuid());
        Assert.assertEquals(actualRequest.getRequestId(), mockRequestId);
        Assert.assertEquals(actualRequest.getJobId(), mockRequestId);
        Assert.assertEquals(actualRequest.getType(), JobTypeEnum.COPY_VERIFY.getValue());
        Assert.assertFalse(actualRequest.getIsSystem());
        Assert.assertFalse(actualRequest.getMessage().isInContext());
        Assert.assertEquals(actualRequest.getMessage().getTopic(), TopicConstants.COPY_VERIFY_EXECUTE);
    }

    /**
     * 用例名称：验证解析副本校验任务时，message为空时抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_IllegalArgumentException_when_parseFromJobMessage_given_message_is_empty() {
        // when and then
        Assert.assertThrows("Copy verify job message is empty", IllegalArgumentException.class,
                () -> CopyVerifyHelper.parseFromJobMessage(""));
    }

    /**
     * 用例名称：验证解析副本校验任务时，message中的payload为空时抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_IllegalArgumentException_when_parseFromJobMessage_given_message_payload_is_empty() {
        //Given
        final JobMessage jobMessage = new JobMessage();
        //When
        Assert.assertThrows(IllegalArgumentException.class,
                () -> CopyVerifyHelper.parseFromJobMessage(JSONObject.fromObject(jobMessage).toString()));
    }

    /**
     * 用例名称：验证解析副本校验任务成功<br/>
     * 前置条件：无<br/>
     * check点：验异解析的任务对象id和期望一致<br/>
     */
    @Test
    public void should_build_successful_when_parseFromJobMessage_given_message_payload_is_empty() {
        //Given
        final String taskId = UUID.randomUUID().toString();
        final JobMessage jobMessage = new JobMessage();
        final CopyVerifyTask givenTask = new CopyVerifyTask();
        givenTask.setTaskId(taskId);
        jobMessage.setPayload(JSONObject.fromObject(givenTask));
        //When
        final CopyVerifyTask actualTask = CopyVerifyHelper.parseFromJobMessage(
            JSONObject.fromObject(jobMessage).toString());
        //Then
        Assert.assertNotNull(actualTask);
        Assert.assertEquals(actualTask.getTaskId(), taskId);
    }

    /**
     * 用例名称：验证构造更新副本请求对象成功<br/>
     * 前置条件：无<br/>
     * check点：验证结果中的值与期望一致<br/>
     */
    @Test
    public void should_build_successful_when_buildUpdateRequest() {
        // given
        String key = CopyPropertiesKeyConstant.KEY_VERIFY_STATUS;
        String value = CopyVerifyStatusEnum.VERIFY_SUCCESS.getVerifyStatus();
        // when
        UpdateCopyPropertiesRequest actualRequest = CopyVerifyHelper.buildUpdateRequest(key, value);
        // then
        Assert.assertNotNull(actualRequest);
        Assert.assertEquals(actualRequest.getKey(), key);
        Assert.assertEquals(actualRequest.getValue(), value);
    }

    /**
     * 用例名称：验证构造资源锁请求成功<br/>
     * 前置条件：无<br/>
     * check点：验证请求中的值和锁类型符合期望<br/>
     */
    @Test
    public void should_build_successful_when_buildLockRequest() {
        // given
        String mockRequestId = UUID.randomUUID().toString();
        String mockCopyId = UUID.randomUUID().toString();
        // when
        LockRequest lockRequest = CopyVerifyHelper.buildLockRequest(mockRequestId, mockRequestId, mockCopyId);
        // then
        Assert.assertNotNull(lockRequest);
        Assert.assertEquals(lockRequest.getLockId(), mockRequestId);
        Assert.assertEquals(lockRequest.getRequestId(), mockRequestId);
        Assert.assertEquals(lockRequest.getPriority(), 10);
        Assert.assertEquals(lockRequest.getResources().size(), 1);
        Assert.assertEquals(lockRequest.getResources().get(0).getId(), mockCopyId);
        Assert.assertEquals(lockRequest.getResources().get(0).getLockType(), LockTypeEnum.READ);
    }

    /**
     * 用例名称：验证副本中没有副本校验状态，校验抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验证异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_copyHasVerificationFile_given_copy_no_verification_file_generated() {
        // given
        Copy mockCopy = new Copy();
        mockCopy.setProperties(new JSONObject().toString());
        // then
        Assert.assertThrows("No verification file is generated for the copy.",
                LegoCheckedException.class, () -> CopyVerifyHelper.copyHasVerificationFile(mockCopy));
    }

    /**
     * 用例名称：验证副本中没有验证文件没有产生，校验抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验证异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_copyHasVerificationFile_given_copy_verify_status_is_verify_file_not_exist() {
        // given
        Copy mockCopy = new Copy();
        JSONObject copyProperties = new JSONObject();
        copyProperties.set(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS,
            CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST.getVerifyStatus());
        mockCopy.setProperties(copyProperties.toString());
        // then
        Assert.assertThrows("No verification file is generated for the copy.",
                LegoCheckedException.class, () -> CopyVerifyHelper.copyHasVerificationFile(mockCopy));
    }

    /**
     * 用例名称：验证副本状态不是正常时，校验抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验证异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_checkCopyStatus_given_copy_status_is_not_normal() {
        String mockCopyId = UUID.randomUUID().toString();
        Copy mockCopy = new Copy();
        mockCopy.setUuid(mockCopyId);
        mockCopy.setStatus(CopyStatus.RESTORING.getValue());
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] status is [%s], can not verify",
            mockCopy.getUuid(), mockCopy.getStatus());
        // then
        Assert.assertThrows(errorMessage,
                LegoCheckedException.class, () -> CopyVerifyHelper.checkCopyStatus(mockCopy));
    }

    /**
     * 用例名称：验证副本状类型是归档副本时，校验抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验证异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_copyIsNotGeneratedByArchive_given_copy_is_archive() {
        String mockCopyId = UUID.randomUUID().toString();
        Copy mockCopy = new Copy();
        mockCopy.setUuid(mockCopyId);
        mockCopy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] is generate by [%s], can not verify",
            mockCopy.getUuid(), mockCopy.getGeneratedBy());
        // then
        Assert.assertThrows(errorMessage, LegoCheckedException.class,
                () -> CopyVerifyHelper.copyIsNotGeneratedByArchive(mockCopy));
    }

    /**
     * 用例名称：验证副本状类型是复制副本时，校验抛出异常<br/>
     * 前置条件：无<br/>
     * check点：验证异常类型和错误信息符合预期<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_copyIsNotGeneratedByReplication_given_copy_is_archive() {
        String mockCopyId = UUID.randomUUID().toString();
        Copy mockCopy = new Copy();
        mockCopy.setUuid(mockCopyId);
        mockCopy.setGeneratedBy(CopyGeneratedByEnum.BY_REPLICATED.value());
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] is generate by [%s], can not verify",
                mockCopy.getUuid(), mockCopy.getGeneratedBy());
        // then
        Assert.assertThrows(errorMessage, LegoCheckedException.class,
                () -> CopyVerifyHelper.copyIsNotGeneratedByReplication(mockCopy));
    }

    private static CopyVerifyManagerContext getMockContext(String mockRequestId, Copy mockCopy,
        CopyVerifyTask copyVerifyTask) {
        CopyVerifyManagerContext mockContext = new CopyVerifyManagerContext();
        mockContext.setRequestId(mockRequestId);
        mockContext.setSubTask(false);
        mockContext.setTask(copyVerifyTask);
        mockContext.setCopy(mockCopy);
        return mockContext;
    }

    private static CopyVerifyTask mockTask(String mockRequestId, Copy mockCopy, ArrayList<Endpoint> mockEndpoints) {
        CopyVerifyTask copyVerifyTask = new CopyVerifyTask();
        copyVerifyTask.setCopyId(mockCopy.getUuid());
        copyVerifyTask.setRequestId(mockRequestId);
        copyVerifyTask.setTaskId(mockRequestId);
        copyVerifyTask.setAgents(mockEndpoints);
        return copyVerifyTask;
    }

}
