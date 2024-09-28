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
package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.testdata.TestDataGenerator;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ManualBackupReq;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import feign.FeignException;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;

/**
 * {@link ProtectionManager} 测试类
 *
 */
public class ProtectionManagerTest {
    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);
    private final OpenStackUserManager userManager = Mockito.mock(OpenStackUserManager.class);
    private final ProtectionManager protectionManager = new ProtectionManager(protectObjectRestApi, userManager);
    private static final String RESOURCE_ID = "test resource id";
    private static final String SLA_ID = "test sla id";
    private static final String USER_ID = "test user id";
    private static final String BACKUP_ACTION = "difference_increment";

    /**
     * 用例场景：查询受保护对象正常返回
     * 前置条件：protection服务正常
     * 检查点： 查询受保护对象正常返回
     */
    @Test
    public void should_returnProtectedObject_when_queryProtectedObject() {
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(new ProtectedObjectInfo());
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(RESOURCE_ID, true);
        assertThat(protectedObject).isNotNull();
    }

    /**
     * 用例场景：如果保护对象不存在，如果需要抛出异常，则抛出LegoCheckedException
     * 前置条件：protection服务正常
     * 检查点： 保护对象不存在，且需要返回异常，则抛出LegoCheckedException
     */
    @Test
    public void should_throwOpenStackException_when_queryProtectedObjectWithStrict_given_nullProtectedObject() {
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(null);
        OpenStackException exception =
            Assert.assertThrows(OpenStackException.class, () -> protectionManager.queryProtectedObject(RESOURCE_ID, true));
        assertThat(exception.getErrorCode()).isEqualTo(OpenStackErrorCodes.NOT_FOUND);
        assertThat(exception.getMessage()).isEqualTo(String.format("Backup job: %s not found.", RESOURCE_ID));
    }

    /**
     * 用例场景：如果保护对象不存在，如果strict为false，则返回null
     * 前置条件：protection服务正常
     * 检查点： 保护对象不存在，且不需要抛出异常，则返回null
     */
    @Test
    public void should_returnNull_when_queryProtectedObjectWithoutStrict_given_nullProtectedObject() {
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(null);
        ProtectedObjectInfo protectedObject = protectionManager.queryProtectedObject(RESOURCE_ID, false);
        assertThat(protectedObject).isNull();
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_queryProtectedObject_given_feignException() {
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenThrow(FeignException.class);
        LegoCheckedException exception =
            Assert.assertThrows(LegoCheckedException.class, () -> protectionManager.queryProtectedObject(RESOURCE_ID, true));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call query protected object error.");
    }

    /**
     * 用例场景：如果保护对象存在且状态为已保护，则正常调用deactivate方法
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为已保护，则调用deactivate方法
     */
    @Test
    public void should_callProtectObjectRestApiDeactivateOneTime_when_deactivateProtection_given_protectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.PROTECTED.getType());
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(objectInfo);
        protectionManager.deactivate(RESOURCE_ID);
        Mockito.verify(protectObjectRestApi, Mockito.times(1)).deactivate(any());
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_deactivateProtection_given_feignException() {
        Mockito.doThrow(FeignException.class).when(protectObjectRestApi).deactivate(any());
        LegoCheckedException exception =
                Assert.assertThrows(LegoCheckedException.class, () -> protectionManager.deactivate(RESOURCE_ID));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call deactivate error.");
    }

    /**
     * 用例场景：如果保护对象存在且状态为未保护，则正常调用activate方法
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为未保护，则调用activate方法
     */
    @Test
    public void should_callProtectObjectRestApiActivateOneTime_when_activateProtection_given_unprotectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(objectInfo);
        protectionManager.activate(RESOURCE_ID);
        Mockito.verify(protectObjectRestApi, Mockito.times(1)).activate(any());
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_activateProtection_given_feignException() {
        Mockito.when(protectObjectRestApi.getProtectObject(RESOURCE_ID)).thenReturn(new ProtectedObjectInfo());
        Mockito.doThrow(FeignException.class).when(protectObjectRestApi).activate(any());
        LegoCheckedException exception =
            Assert.assertThrows(LegoCheckedException.class, () -> protectionManager.activate(RESOURCE_ID));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call activate error.");
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createManualBackup_given_feignException() {
        Mockito.when(userManager.obtainUserId()).thenReturn(USER_ID);
        ManualBackupReq backupReq = new ManualBackupReq();
        backupReq.setSlaId(SLA_ID);
        backupReq.setAction(BACKUP_ACTION);
        Mockito.when(protectObjectRestApi.manualBackup(anyString(), anyString(), any())).thenThrow(FeignException.class);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> protectionManager.manualBackup(RESOURCE_ID, SLA_ID, BACKUP_ACTION));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call manual backup error.");
    }

    /**
     * 用例场景：调用创建手动备份任务接口成功
     * 前置条件：protection服务正常，
     * 检查点： 能成功调用创建手动备份接口
     */
    @Test
    public void should_callProtectObjectRestApiManualBackupOneTime_when_createManualBackup() {
        Mockito.when(userManager.obtainUserId()).thenReturn(USER_ID);
        protectionManager.manualBackup(RESOURCE_ID, SLA_ID, BACKUP_ACTION);
        Mockito.verify(protectObjectRestApi, Mockito.times(1)).manualBackup(anyString(), anyString(), any());
    }

    /**
     * 用例场景：调用删除保护接口成功
     * 前置条件：protection服务正常，
     * 检查点： 能成功调用删除保护接口
     */
    @Test
    public void should_callDeleteProtectedObjectOneTime_when_deleteProtection() {
        protectionManager.delete(RESOURCE_ID);
        Mockito.verify(protectObjectRestApi, Mockito.times(1)).deleteProtectedObjects(any());
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_deleteProtection_given_feignException() {
        Mockito.doThrow(FeignException.class).when(protectObjectRestApi).deleteProtectedObjects(any());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> protectionManager.delete(RESOURCE_ID));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call delete protected object error.");
    }

    /**
     * 用例场景：调用protectObjectRestApi创建保护接口成功时，返回任务id
     * 前置条件：protection服务正常
     * 检查点： protection服务正常，返回保护任务id
     */
    @Test
    public void should_returnJobId_when_createProtection() throws IOException {
        String userId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String slaId = UUIDGenerator.getUUID();
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.when(protectObjectRestApi.createProtectedObject(anyString(), any()))
            .thenReturn(Collections.singletonList(jobId));

        String result = protectionManager.createProtection(slaId, resourceId, backupJob);
        assertThat(result).isEqualTo(jobId);
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createProtection_given_feignException() throws IOException {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String slaId = UUIDGenerator.getUUID();
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.doThrow(FeignException.class).when(protectObjectRestApi).createProtectedObject(anyString(), any());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> protectionManager.createProtection(slaId, resourceId, backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call create protection error.");
    }

    /**
     * 用例场景：调用protectObjectRestApi修改保护接口成功时，返回任务id
     * 前置条件：protection服务正常
     * 检查点： protection服务正常，返回保护任务id
     */
    @Test
    public void should_returnJobId_when_modifyProtection() throws IOException {
        String userId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String slaId = UUIDGenerator.getUUID();
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        protectedObject.setResourceId(resourceId);
        protectedObject.setExtParameters(new HashMap<>());

        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.when(protectObjectRestApi.modifyProtectedObject(anyString(), any()))
            .thenReturn(jobId);

        String result = protectionManager.modifyProtection(slaId, backupJob, protectedObject);
        assertThat(result).isEqualTo(jobId);
    }

    /**
     * 用例场景：调用远程接口失败时，抛出异常
     * 前置条件：protection服务异常
     * 检查点： protection服务异常，抛出指定异常
     */
    @Test
    public void should_throwLegoCheckedException_when_modifyProtection_given_feignException() throws IOException {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String slaId = UUIDGenerator.getUUID();
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        protectedObject.setResourceId(resourceId);
        protectedObject.setExtParameters(new HashMap<>());

        Mockito.when(userManager.obtainUserId()).thenReturn(userId);
        Mockito.doThrow(FeignException.class).when(protectObjectRestApi).modifyProtectedObject(anyString(), any());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> protectionManager.modifyProtection(slaId, backupJob, protectedObject));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo("call modify protection error.");
    }
}
