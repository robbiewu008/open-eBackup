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
package openbackup.data.access.framework.backup.listener.v1;

import openbackup.data.access.framework.backup.listener.v1.ProtectionBackupListener;
import openbackup.data.access.framework.backup.service.IBackupService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupProvider;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;

import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;

import static org.mockito.Mockito.*;

import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.UUID;


/**
 * ProtectionBackupListener LLT
 *
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {ProtectionBackupListener.class})
public class ProtectionBackupListenerTest {
    private static final String HDFS_FILE_SET = "HDFSFileSet";
    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private IBackupService backupService;

    @MockBean
    private ProviderManager manager;

    @Autowired
    private ProtectionBackupListener protectionBackupListener;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private JobProvider jobProvider;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private UserQuotaManager userQuotaManager;

    @MockBean
    private FunctionSwitchService functionSwitchService;

    @MockBean
    private JobService jobService;

    @Before
    public void before(){
        Mockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
    }

    /**
     * 用例场景：正常执行Oracle备份流程
     * 前置条件：正常消费protection.backup消息
     * 检查点：无异常抛出
     */
    @Test
    public void test_handle_oracle_backup_message_success() {
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType(ResourceSubTypeEnum.VMWARE.getType());
        protectedObject.setResourceId(UUIDGenerator.getUUID());
        JobBo job = new JobBo();
        job.setUserId(UUIDGenerator.getUUID());
        job.setJobId(UUIDGenerator.getUUID());
        Mockito.when(jobService.queryJob(anyString())).thenReturn(job);
        Mockito.doNothing()
            .when(userQuotaManager)
            .checkBackupQuota(job.getUserId(), protectedObject.getResourceId());
        executeBackup(protectedObject);
        verify(manager, times(1)).findProvider(BackupProvider.class, ResourceSubTypeEnum.VMWARE.getType());
    }

    /**
     * 用例场景：正常执行HDFS备份流程
     * 前置条件：正常消费protection.backup消息
     * 检查点：无异常抛出
     */
    @Test
    public void test_handle_hdfs_backup_message_success() {
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSubType(HDFS_FILE_SET);
        protectedObject.setResourceId(UUIDGenerator.getUUID());
        JobBo job = new JobBo();
        job.setUserId(UUIDGenerator.getUUID());
        job.setJobId(UUIDGenerator.getUUID());
        Mockito.when(jobService.queryJob(anyString())).thenReturn(job);
        Mockito.doNothing()
            .when(userQuotaManager)
            .checkBackupQuota(job.getUserId(), protectedObject.getResourceId());
        executeBackup(protectedObject);
        verify(manager, times(0)).findProvider(BackupProvider.class, HDFS_FILE_SET);
    }

    private void executeBackup(ProtectedObject protectedObject) {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        JSONObject jsonObject = JSONObject.fromObject(protectedObject);
        String s = jsonObject.toString();
        PowerMockito.when(map.get(ArgumentMatchers.eq("protected_object"))).thenReturn(s);
        BackupProvider protectionProvider = PowerMockito.mock(BackupProvider.class);
        PowerMockito.when(
                manager.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(protectionProvider);
        protectionBackupListener.backup(data.toString(), acknowledgment);
    }
}
