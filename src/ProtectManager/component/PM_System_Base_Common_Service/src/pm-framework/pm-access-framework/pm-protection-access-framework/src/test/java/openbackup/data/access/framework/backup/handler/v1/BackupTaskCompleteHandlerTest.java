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
package openbackup.data.access.framework.backup.handler.v1;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.backup.handler.v1.BackupTaskCompleteHandler;
import openbackup.data.access.framework.backup.service.impl.JobBackupPostProcessService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v1.BackupFollowUpProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

/**
 * BackupTaskCompleteHandler LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(BackupTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class BackupTaskCompleteHandlerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ProviderManager registry;

    @Mock
    private NotifyManager notifyManager;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private JobService jobService;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Mock
    private JobBackupPostProcessService jobBackupPostProcessService;

    @InjectMocks
    private BackupTaskCompleteHandler backupTaskCompleteHandler;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Test
    public void testApplicable() {
        boolean applicable = backupTaskCompleteHandler.applicable("BACKUP");
        Assert.assertTrue(applicable);
    }

    @Test
    public void testOnTaskCompleteSuccess() throws Exception {
        String requestId = UUID.randomUUID().toString();
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(requestId);
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.get(ArgumentMatchers.eq("backup_type"))).thenReturn("FULL").thenReturn("log");
        PowerMockito.when(map.get(ArgumentMatchers.eq("job_id"))).thenReturn(requestId);
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType("Oracle");
        PowerMockito.when(map.get(ArgumentMatchers.eq("protected_object")))
                .thenReturn(JSONObject.fromObject(protectedObject).toString());
        CopyProvider copyProvider = PowerMockito.mock(CopyProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(copyProvider);
        BackupFollowUpProvider backupFollowUpProvider = PowerMockito.mock(BackupFollowUpProvider.class);
        PowerMockito.when(
                        registry.findProviderOrDefault(
                                ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(backupFollowUpProvider);
        CopyInfoBo copy = new CopyInfoBo();
        copy.setResourceId(UUID.randomUUID().toString());
        copy.setTimestamp(String.valueOf(System.currentTimeMillis()));
        copy.setResourceSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        PowerMockito.when(map.get("policy"))
                .thenReturn(
                        constructPolicyJson(
                                UUID.randomUUID().toString(), UUID.randomUUID().toString(), 10, true, true));
        List<CopyInfoBo> copyInfoList = Collections.singletonList(copy);
        PowerMockito.when(
                        copyProvider.buildCopy(
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString()))
                .thenReturn(copyInfoList);
        BasePage<Copy> basePage = PowerMockito.mock(BasePage.class);
        PowerMockito.when(
                        copyRestApi.queryCopies(
                                ArgumentMatchers.anyInt(),
                                ArgumentMatchers.anyInt(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyList()))
                .thenReturn(basePage);
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid(UUID.randomUUID().toString());
        PowerMockito.when(copyRestApi.saveCopy(ArgumentMatchers.any())).thenReturn(uuidObject);
        doNothing().when(jobService).updateJob(anyString(), any());
        doNothing().when(backupFollowUpProvider).handleSuccess(anyString(), anyString(), anyInt(), anyString());
        JobBo job = new JobBo();
        job.setJobId(UUIDGenerator.getUUID());
        job.setSourceId(UUIDGenerator.getUUID());
        when(jobService.queryJob(anyString())).thenReturn(job);
        doNothing().when(userQuotaManager).increaseUsedQuota(anyString(),any());
        PowerMockito.doNothing().when(jobBackupPostProcessService).onBackupJobSuccess(anyString());
        backupTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        verify(jobService, times(1)).updateJob(anyString(), any());
        verify(copyProvider, times(1)).buildCopy(anyString(), anyString(), anyString(), anyString());

        backupTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        verify(jobService, times(2)).updateJob(anyString(), any());
        verify(copyProvider, times(1)).buildCopy(anyString(), anyString(), anyString(), anyString());
    }

    /**
     * 备份完成没有查到副本信息，抛异常
     * 前置条件：备份完成
     * 检查点：没有吵到副本信息，抛LegoCheckedException
     * */
    @Test
    public void should_throw_LegoCheckedException_if_backup_complete_query_copy_info_fail() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("query backup copy info list fail.");
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.get(ArgumentMatchers.eq("backup_type"))).thenReturn("FULL");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType("Oracle");
        PowerMockito.when(map.get(ArgumentMatchers.eq("protected_object")))
                .thenReturn(JSONObject.fromObject(protectedObject).toString());
        CopyProvider copyProvider = PowerMockito.mock(CopyProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(copyProvider);
        List<CopyInfoBo> copyInfoList = new ArrayList<>();
        PowerMockito.when(
                        copyProvider.buildCopy(
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString()))
                .thenReturn(copyInfoList);
        backupTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
    }

    /**
     * 标准备份完成，有副本信息，副本信息根据timestamp升序排序正确
     * 前置条件：备份完成，生成副本信息
     * 检查点：副本信息根据timestamp字段升序排序正确
     * */
    @Test
    public void backup_complete_sort_copy_by_timestamp_asc_success() throws Exception {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.get(ArgumentMatchers.eq("backup_type"))).thenReturn("FULL");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType("Oracle");
        PowerMockito.when(map.get(ArgumentMatchers.eq("protected_object")))
                .thenReturn(JSONObject.fromObject(protectedObject).toString());
        CopyProvider copyProvider = PowerMockito.mock(CopyProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(copyProvider);
        List<CopyInfoBo> copyInfoList = constructFileSetCopies();
        PowerMockito.when(
                        copyProvider.buildCopy(
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString()))
                .thenReturn(copyInfoList);

        PowerMockito.when(map.get("policy"))
                .thenReturn(
                        constructPolicyJson(
                                UUID.randomUUID().toString(), UUID.randomUUID().toString(), 10, true, true));

        UuidObject resp = new UuidObject();
        resp.setUuid(UUID.randomUUID().toString());
        PowerMockito.when(copyRestApi.saveCopy(ArgumentMatchers.any())).thenReturn(resp);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        PowerMockito.whenNew(UpdateJobRequest.class).withAnyArguments().thenReturn(updateJobRequest);
        BackupFollowUpProvider backupFollowUpProvider = PowerMockito.mock(BackupFollowUpProvider.class);
        PowerMockito.when(
                registry.findProviderOrDefault(
                        ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(backupFollowUpProvider);
        doNothing().when(backupFollowUpProvider).handleSuccess(anyString(), anyString(), anyInt(), anyString());
        JobBo job = new JobBo();
        job.setJobId(UUIDGenerator.getUUID());
        job.setSourceId(UUIDGenerator.getUUID());
        when(jobService.queryJob(any())).thenReturn(job);
        doNothing().when(userQuotaManager).increaseUsedQuota(anyString(), any());
        backupTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        Assert.assertEquals("1629700162104", updateJobRequest.getCopyTime().toString());
    }

    @Test
    public void testOnTaskCompleteFailed() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(6);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.get(ArgumentMatchers.eq(ContextConstants.JOB_TYPE)))
                .thenReturn(JobTypeEnum.BACKUP.getValue());
        PowerMockito.when(map.get(ArgumentMatchers.eq(ContextConstants.JOB_ID)))
                .thenReturn(UUID.randomUUID().toString());
        BackupFollowUpProvider followUpProvider = PowerMockito.mock(BackupFollowUpProvider.class);
        PowerMockito.when(
                        registry.findProviderOrDefault(
                                ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(followUpProvider);
        doNothing().when(followUpProvider).handleFailure(anyString(), anyString(), anyInt());
        backupTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
        verify(followUpProvider, times(1)).handleFailure(anyString(), anyString(), anyInt());
    }

    @Test
    public void testUpdateLastedFullCopyExpirationTimeWhenFullCopyPermanentSave() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.get(ArgumentMatchers.eq("backup_type"))).thenReturn("FULL");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType(ResourceSubTypeEnum.SQL_SERVER.getType());
        PowerMockito.when(map.get(ArgumentMatchers.eq("protected_object")))
                .thenReturn(JSONObject.fromObject(protectedObject).toString());
        CopyProvider copyProvider = PowerMockito.mock(CopyProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(copyProvider);
        CopyInfoBo copy = new CopyInfoBo();
        copy.setResourceId(UUID.randomUUID().toString());
        copy.setTimestamp(String.valueOf(System.currentTimeMillis()));
        copy.setResourceSubType(ResourceSubTypeEnum.SQL_SERVER.getType());
        copy.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
        copy.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        Copy fullCopy = new Copy();
        fullCopy.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
        PowerMockito.when(
                        copyRestApi.queryCopyByCondition(
                                ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(fullCopy);
        List<CopyInfoBo> copyInfoList = Collections.singletonList(copy);
        PowerMockito.when(
                        copyProvider.buildCopy(
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyString()))
                .thenReturn(copyInfoList);
        BasePage<Copy> basePage = PowerMockito.mock(BasePage.class);
        PowerMockito.when(
                        copyRestApi.queryCopies(
                                ArgumentMatchers.anyInt(),
                                ArgumentMatchers.anyInt(),
                                ArgumentMatchers.anyString(),
                                ArgumentMatchers.anyList()))
                .thenReturn(basePage);
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid(UUID.randomUUID().toString());
        PowerMockito.when(copyRestApi.saveCopy(ArgumentMatchers.any())).thenReturn(uuidObject);
        BackupFollowUpProvider backupFollowUpProvider = PowerMockito.mock(BackupFollowUpProvider.class);
        PowerMockito.when(
                registry.findProviderOrDefault(
                        ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(backupFollowUpProvider);
        doNothing().when(backupFollowUpProvider).handleSuccess(anyString(), anyString(), anyInt(), anyString());
        JobBo job = new JobBo();
        job.setJobId(UUIDGenerator.getUUID());
        job.setSourceId(UUIDGenerator.getUUID());
        when(jobService.queryJob(any())).thenReturn(job);
        doNothing().when(userQuotaManager).increaseUsedQuota(anyString(), any());
        backupTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        Assert.assertNotNull(jobService);
    }

    private List<CopyInfoBo> constructFileSetCopies() {
        List<CopyInfoBo> copyInfos = new ArrayList<>();

        String[] timestamps =
                new String[] {
                    "1629689941906989",
                    "1629690585643134",
                    "1629690861672460",
                    "1629690969248884",
                    "1629699715331337",
                    "1629700043876410",
                    "1629700162104768",
                    "1629691648912696",
                    "1629696174928579",
                    "1629692635039102"
                };
        for (String timestamp : timestamps) {
            CopyInfoBo copyInfo = new CopyInfoBo();
            copyInfo.setTimestamp(timestamp);
            copyInfo.setResourceSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
            copyInfos.add(copyInfo);
        }
        return copyInfos;
    }

    private String constructPolicyJson(
            String policyId, String storageId, int archiveTimes, boolean openAggregation, boolean autoIndex) {
        JSONObject policyExtParam = new JSONObject();
        policyExtParam.put("storage_id", storageId);
        policyExtParam.put("network_acceleration", true);
        policyExtParam.put("is_synthetic_full_copy_period", true);
        policyExtParam.put("synthetic_full_copy_period", archiveTimes);
        policyExtParam.put("open_aggregation", openAggregation);
        policyExtParam.put("auto_index", autoIndex);

        JSONObject policyJsonObj = new JSONObject();
        policyJsonObj.put("uuid", policyId);

        JSONObject policyObj = new JSONObject();
        policyObj.put("ext_parameters", policyExtParam);
        policyObj.put("policy", policyJsonObj);

        return JSONObject.stringify(policyObj);
    }
}
