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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.enums.JobResult;
import openbackup.openstack.adapter.enums.OpenStackJobStatus;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.io.IOException;
import java.util.Collections;
import java.util.Locale;
import java.util.Optional;

/**
 * {@link OpenStackBackupAdapter} 测试类
 *
 */
public class OpenStackBackupAdapterTest {
    private static final String RESOURCE_ID = "test resource id";

    private final ProtectionManager protectionManager = Mockito.mock(ProtectionManager.class);
    private final SlaManager slaManager = Mockito.mock(SlaManager.class);
    private final OpenStackJobManager jobManager = Mockito.mock(OpenStackJobManager.class);
    private final OpenStackResourceManager resourceManager = Mockito.mock(OpenStackResourceManager.class);

    private final OpenStackBackupAdapter adapter =
            new OpenStackBackupAdapter(protectionManager, slaManager, jobManager, resourceManager);

    /**
     * 用例场景：如果创建备份时，如果JobsSchedule为空，并且资源未保护，则先绑定预置SLA并执行手动备份成功
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行手动备份；2、返回值正确
     */
    @Test
    public void should_executeManualBackupSuccess_when_createJob_given_nullJobsScheduleAndUnprotectedResource()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(RESOURCE_ID);
        Mockito.when(resourceManager.queryResourceByVolumeId(backupJob.getInstanceId()))
            .thenReturn(Optional.of(resource));
        // 首次执行手动备份，先绑定预置SLA
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, false)).thenReturn(null);
        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(OpenStackConstants.GLOBAL_SLA_ID, RESOURCE_ID, backupJob))
            .thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(true);
        Mockito.doNothing().when(protectionManager).deactivate(RESOURCE_ID);
        Mockito.doNothing()
            .when(protectionManager)
            .manualBackup(RESOURCE_ID, OpenStackConstants.GLOBAL_SLA_ID, PolicyAction.FULL.getAction());
        resource.setProtectionStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.when(resourceManager.queryResourceById(RESOURCE_ID)).thenReturn(Optional.of(resource));
        Mockito.when(jobManager.queryLatestJob(RESOURCE_ID, JobTypeEnum.BACKUP.getValue()))
            .thenReturn(Optional.empty());

        OpenStackBackupJobDto resp = adapter.createJob(backupJob);
        assertThat(resp).usingRecursiveComparison().ignoringFields("status", "lastResult", "id").isEqualTo(backupJob);
        assertThat(resp.getId()).isEqualTo(RESOURCE_ID);
        assertThat(resp.getStatus()).isEqualTo(OpenStackJobStatus.STOP.getStatus());
        assertThat(resp.getLastResult()).isEqualTo(StringUtils.EMPTY);
    }

    /**
     * 用例场景：如果创建备份时，如果JobsSchedule为空，并且资源已保护，则执行手动备份成功
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行手动备份；2、返回值正确
     */
    @Test
    public void should_executeManualBackupSuccess_when_createJob_given_nullJobsScheduleAndProtectedResource()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(RESOURCE_ID);
        Mockito.when(resourceManager.queryResourceByVolumeId(backupJob.getInstanceId()))
            .thenReturn(Optional.of(resource));
        // 非首次执行手动备份
        String slaId = UUIDGenerator.getUUID();
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setSlaId(slaId);
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, false)).thenReturn(objectInfo);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(slaId, RESOURCE_ID, backupJob))
            .thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(true);
        Mockito.doNothing().when(protectionManager).deactivate(RESOURCE_ID);
        Mockito.doNothing()
            .when(protectionManager)
            .manualBackup(RESOURCE_ID, slaId, PolicyAction.FULL.getAction());

        resource.setProtectionStatus(ProtectionStatusEnum.PROTECTED.getType());
        Mockito.when(resourceManager.queryResourceById(RESOURCE_ID)).thenReturn(Optional.of(resource));

        JobBo job = new JobBo();
        job.setStatus(JobStatusEnum.SUCCESS.name());
        Mockito.when(jobManager.queryLatestJob(RESOURCE_ID, JobTypeEnum.BACKUP.getValue()))
            .thenReturn(Optional.of(job));

        OpenStackBackupJobDto resp = adapter.createJob(backupJob);
        assertThat(resp).usingRecursiveComparison().ignoringFields("status", "lastResult", "id").isEqualTo(backupJob);
        assertThat(resp.getId()).isEqualTo(RESOURCE_ID);
        assertThat(resp.getStatus()).isEqualTo(OpenStackJobStatus.RUNNING.getStatus());
        assertThat(resp.getLastResult()).isEqualTo(JobResult.SUCCESS.getResult());
    }

    /**
     * 用例场景：如果执行手动备份时，如果保护任务未成功，则抛出异常
     * 前置条件：保护失败
     * 检查点： 保护失败应抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createJobAndManualBackup_given_protectionJobNotSuccess()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(RESOURCE_ID);
        Mockito.when(resourceManager.queryResourceByVolumeId(backupJob.getInstanceId()))
            .thenReturn(Optional.of(resource));
        // 首次执行手动备份
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, false)).thenReturn(null);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(OpenStackConstants.GLOBAL_SLA_ID, RESOURCE_ID, backupJob))
            .thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(false);
        Mockito.doNothing().when(jobManager).forceStopJob(jobId);
        Mockito.doNothing()
            .when(resourceManager)
            .updateProtectionStatus(RESOURCE_ID, ProtectionStatusEnum.UNPROTECTED.getType());

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> adapter.createJob(backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(String.format("Protection job: %s is not success.", jobId));
    }

    /**
     * 用例场景：创建备份任务时，如果根据卷id找不到资源，则抛出异常
     * 前置条件：卷id不存在
     * 检查点：  卷id不存在的情况下，应抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createJob_given_noneExistVolumeId()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        Mockito.when(resourceManager.queryResourceByVolumeId(backupJob.getInstanceId()))
            .thenReturn(Optional.empty());

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> adapter.createJob(backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(
            String.format("Has no host contains this volume id: %s.", backupJob.getInstanceId()));
    }

    /**
     * 用例场景：如果创建备份时，如果JobsSchedule不为空，则执行保护资源操作
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行保护资源操作；2、返回值正确
     */
    @Test
    public void should_protectResource_when_createJob_given_unprotectedResource()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        Mockito.when(protectionManager.queryProtectedObject(backupJob.getInstanceId(), false)).thenReturn(null);

        String slaId = UUIDGenerator.getUUID();
        Mockito.when(slaManager.createSla(backupJob)).thenReturn(slaId);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(slaId, backupJob.getInstanceId(), backupJob))
            .thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(true);

        ProtectedResource resource = new ProtectedResource();
        resource.setProtectionStatus(ProtectionStatusEnum.PROTECTED.getType());
        resource.setUuid(backupJob.getInstanceId());
        Mockito.when(resourceManager.queryResourceById(backupJob.getInstanceId())).thenReturn(Optional.of(resource));
        Mockito.when(jobManager.queryLatestJob(backupJob.getInstanceId(), JobTypeEnum.BACKUP.getValue()))
            .thenReturn(Optional.empty());

        OpenStackBackupJobDto resp = adapter.createJob(backupJob);
        assertThat(resp).usingRecursiveComparison().ignoringFields("status", "lastResult", "id").isEqualTo(backupJob);
        assertThat(resp.getId()).isEqualTo(backupJob.getInstanceId());
        assertThat(resp.getStatus()).isEqualTo(OpenStackJobStatus.RUNNING.getStatus());
        assertThat(resp.getLastResult()).isEqualTo(StringUtils.EMPTY);
    }

    /**
     * 用例场景：创建备份任务，对资源创建保护时，如果保护任务未成功，则抛出异常
     * 前置条件：保护任务不成功
     * 检查点：  保护失败应抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createJobAndProtectResource_given_protectionJobNotSuccess()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(backupJob.getInstanceId());
        Mockito.when(resourceManager.queryResourceById(backupJob.getInstanceId())).thenReturn(Optional.of(resource));

        Mockito.when(protectionManager.queryProtectedObject(backupJob.getInstanceId(), false)).thenReturn(null);

        String slaId = UUIDGenerator.getUUID();
        Mockito.when(slaManager.createSla(backupJob)).thenReturn(slaId);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(slaId, backupJob.getInstanceId(), backupJob)).thenReturn(jobId);

        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(false);
        Mockito.doNothing().when(jobManager).forceStopJob(jobId);
        Mockito.doNothing()
            .when(resourceManager)
            .updateProtectionStatus(backupJob.getInstanceId(), ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.doNothing().when(slaManager).deleteSla(slaId);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> adapter.createJob(backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(String.format("Protection job: %s is not success.", jobId));
    }

    /**
     * 用例场景：创建备份任务，对资源创建保护时，如果资源已保护，则抛出异常
     * 前置条件：资源已保护
     * 检查点：  不能对已保护的资源再次创建保护
     */
    @Test
    public void should_throwLegoCheckedException_when_createJobAndProtectResource_given_existProtectedObject()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(backupJob.getInstanceId());
        Mockito.when(resourceManager.queryResourceById(backupJob.getInstanceId())).thenReturn(Optional.of(resource));

        Mockito.when(protectionManager.queryProtectedObject(backupJob.getInstanceId(), false))
            .thenReturn(new ProtectedObjectInfo());

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> adapter.createJob(backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(
            String.format(Locale.ENGLISH, "Resource: %s is already under protected.", backupJob.getInstanceId()));
    }

    /**
     * 用例场景：创建备份任务，对资源创建保护时，构造响应体时，资源不存在，则抛出异常
     * 前置条件：资源不存在
     * 检查点：  资源不存在时应抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_createJobAndBuildResponse_given_nonExistResource()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();

        Mockito.when(protectionManager.queryProtectedObject(backupJob.getInstanceId(), false)).thenReturn(null);

        String slaId = UUIDGenerator.getUUID();
        Mockito.when(slaManager.createSla(backupJob)).thenReturn(slaId);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.createProtection(slaId, backupJob.getInstanceId(), backupJob))
            .thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(true);

        Mockito.when(resourceManager.queryResourceById(backupJob.getInstanceId())).thenReturn(Optional.empty());

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> adapter.createJob(backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(
            String.format("Resource: %s not exist.", backupJob.getInstanceId()));
    }

    /**
     * 用例场景：更新备份时，如果JobsSchedule为空，执行手动备份成功
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行手动备份；2、返回值正确
     */
    @Test
    public void should_executeManualBackup_when_updateJob_given_nullJobsScheduleAndProtectedResource()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        String slaId = UUIDGenerator.getUUID();
        protectedObject.setSlaId(slaId);
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(protectedObject);
        OpenStackBackupAdapter spy = Mockito.spy(adapter);
        Mockito.doReturn(new OpenStackBackupJobDto()).when(spy).queryJob(RESOURCE_ID);

        OpenStackBackupJobDto result = spy.updateJob(RESOURCE_ID, backupJob);
        assertThat(result).isNotNull();
        Mockito.verify(protectionManager, Mockito.times(1))
            .manualBackup(RESOURCE_ID, slaId, PolicyAction.FULL.getAction());
    }

    /**
     * 用例场景：更新备份时，如果JobsSchedule存在，并且需要修改保护，则执行修改保护操作
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行修改保护；2、返回值正确
     */
    @Test
    public void should_callModifyProtectionOneTime_when_updateJob_given_protectedObjectDescriptionNotEqualToBackupJob()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        backupJob.setDescription("description value");

        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        String slaId = UUIDGenerator.getUUID();
        protectedObject.setSlaId(slaId);
        protectedObject.setExtParameters(Collections.singletonMap("description", "value"));

        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(protectedObject);

        OpenStackBackupAdapter spy = Mockito.spy(adapter);
        Mockito.doReturn(new OpenStackBackupJobDto()).when(spy).queryJob(RESOURCE_ID);
        Mockito.when(slaManager.updateSla(any(), any())).thenReturn(slaId);
        Mockito.when(jobManager.isJobSuccess(any())).thenReturn(true);

        spy.updateJob(RESOURCE_ID, backupJob);
        Mockito.verify(protectionManager, Mockito.times(1)).modifyProtection(slaId, backupJob, protectedObject);
    }

    /**
     * 用例场景：更新备份时，如果修改保护任务未成功，则抛出异常
     * 前置条件：修改保护任务未成功
     * 检查点： 修改保护任务未成功需抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_updateJob_given_modifyProtectionJobFail()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        backupJob.setDescription("description value");

        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        String slaId = UUIDGenerator.getUUID();
        protectedObject.setSlaId(slaId);
        protectedObject.setExtParameters(Collections.singletonMap("description", "value"));

        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(protectedObject);

        OpenStackBackupAdapter spy = Mockito.spy(adapter);
        Mockito.doReturn(new OpenStackBackupJobDto()).when(spy).queryJob(RESOURCE_ID);
        Mockito.when(slaManager.updateSla(any(), any())).thenReturn(slaId);
        Mockito.when(jobManager.isJobSuccess(any())).thenReturn(true);

        String jobId = UUIDGenerator.getUUID();
        Mockito.when(protectionManager.modifyProtection(slaId, backupJob, protectedObject)).thenReturn(jobId);
        Mockito.when(jobManager.isJobSuccess(jobId)).thenReturn(false);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> spy.updateJob(RESOURCE_ID, backupJob));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.SYSTEM_ERROR);
        assertThat(exception.getMessage()).isEqualTo(String.format("Modify protection job: %s was fail.", jobId));
    }

    /**
     * 用例场景：更新备份时，如果JobsSchedule存在，并且不需修改保护，则只执行更新SLA操作
     * 前置条件：base、protection服务正常
     * 检查点： 1、能够成功执行更新SLA；2、返回值正确
     */
    @Test
    public void should_callModifyProtectionZeroTime_when_updateJob_given_protectedObjectDescriptionEqualToBackupJob()
        throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        backupJob.setDescription("");

        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        String slaId = UUIDGenerator.getUUID();
        protectedObject.setSlaId(slaId);
        protectedObject.setExtParameters(Collections.emptyMap());

        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(protectedObject);

        OpenStackBackupAdapter spy = Mockito.spy(adapter);
        Mockito.doReturn(new OpenStackBackupJobDto()).when(spy).queryJob(RESOURCE_ID);
        Mockito.when(slaManager.updateSla(any(), any())).thenReturn(slaId);
        Mockito.when(jobManager.isJobSuccess(any())).thenReturn(true);

        spy.updateJob(RESOURCE_ID, backupJob);
        Mockito.verify(slaManager, Mockito.times(1)).updateSla(any(), any());
        Mockito.verify(protectionManager, Mockito.times(0)).modifyProtection(slaId, backupJob, protectedObject);
    }

    /**
     * 用例场景：如果保护对象存在且状态为未保护，直接返回
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为未保护，则不再执行deactivate操作
     */
    @Test
    public void should_callDeactivateZeroTime_when_stopJob_given_unprotectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(objectInfo);
        adapter.stopJob(RESOURCE_ID);
        Mockito.verify(protectionManager, Mockito.times(0)).deactivate(RESOURCE_ID);
    }

    /**
     * 用例场景：如果保护对象存在且状态为已保护保护，则执行禁用保护操作
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为已保护，则执行deactivate操作
     */
    @Test
    public void should_callDeactivateOneTime_when_stopJob_given_protectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.PROTECTED.getType());
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(objectInfo);
        adapter.stopJob(RESOURCE_ID);
        Mockito.verify(protectionManager, Mockito.times(1)).deactivate(RESOURCE_ID);
    }

    /**
     * 用例场景：如果保护对象存在且状态为已保护，直接返回
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为已保护，则不再执行activate和manualBackup操作
     */
    @Test
    public void should_callManualBackupZeroTime_when_startJob_given_protectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.PROTECTED.getType());
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(objectInfo);
        adapter.startJob(RESOURCE_ID);
        Mockito.verify(protectionManager, Mockito.times(0)).activate(RESOURCE_ID);
        Mockito.verify(protectionManager, Mockito.times(0)).manualBackup(anyString(), anyString(), anyString());
    }

    /**
     * 用例场景：如果保护对象存在且状态为未保护，则执行激活保护和手动备份操作
     * 前置条件：protection服务正常，存在保护对象
     * 检查点： 保护对象存在且为未保护，则执行activate和manualBackup操作
     */
    @Test
    public void should_callManualBackupOneTime_when_startJob_given_unprotectedObject() {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        objectInfo.setStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.when(protectionManager.queryProtectedObject(RESOURCE_ID, true)).thenReturn(objectInfo);
        adapter.startJob(RESOURCE_ID);
        Mockito.verify(protectionManager, Mockito.times(1)).activate(anyString());
        Mockito.verify(protectionManager, Mockito.times(1)).manualBackup(any(), any(), any());
    }
}
