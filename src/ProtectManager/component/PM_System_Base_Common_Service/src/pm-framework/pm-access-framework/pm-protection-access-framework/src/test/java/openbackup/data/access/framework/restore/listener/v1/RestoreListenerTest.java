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
package openbackup.data.access.framework.restore.listener.v1;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.restore.listener.v1.RestoreListener;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * RestoreListener LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(RestoreListener.class)
@AutoConfigureMockMvc
public class RestoreListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private JobService jobService;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @InjectMocks
    private RestoreListener restoreListener;

    @Test
    public void testRestore() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(any(), any())).thenReturn(null);
        JSONObject jsonObject = new JSONObject();
        String s = jsonObject.set("ext_parameters", "test").toString();
        PowerMockito.when(map.get(ArgumentMatchers.eq("ext_parameters"))).thenReturn(s);

        JobBo job = new JobBo();
        job.setUserId(UUIDGenerator.getUUID());
        Mockito.when(jobService.queryJob(anyString())).thenReturn(job);

        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());
        Copy copy = new Copy();
        copy.setGeneratedBy("Backup");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString(), ArgumentMatchers.anyBoolean())).thenReturn(copy);
        RestoreProvider provider = PowerMockito.mock(RestoreProvider.class);
        PowerMockito.when(registry.findProvider(any(), any())).thenReturn(provider);
        restoreListener.restore(data.toString(), acknowledgment);
        Mockito.verify(copyRestApi, Mockito.times(1))
                .queryCopyByID(job.getJobId(),false);
    }

    /**
     * 用例名称：如果Vmware执行文件细粒度下载，不查询任务<br/>
     * 前置条件：无<br/>
     * check点：Vmware文件细粒度下载不走查询任务逻辑<br/>
     */
    @Test
    public void should_callQueryJob0Time_when_restore_given_subTypeVmwareAndRestoreTypeDownload() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(any(), any())).thenReturn(null);
        JSONObject jsonObject = new JSONObject();
        String s = jsonObject.set("ext_parameters", "test").toString();
        PowerMockito.when(map.get(ArgumentMatchers.eq("ext_parameters"))).thenReturn(s);
        PowerMockito.when(map.get(ArgumentMatchers.eq("restore_type"))).thenReturn("download");
        PowerMockito.when(map.get(ArgumentMatchers.eq("copy_id"))).thenReturn(UUIDGenerator.getUUID());
        Copy copy = new Copy();
        copy.setGeneratedBy("Backup");
        copy.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString(), ArgumentMatchers.anyBoolean())).thenReturn(copy);
        RestoreProvider provider = PowerMockito.mock(RestoreProvider.class);
        PowerMockito.when(registry.findProvider(any(), any())).thenReturn(provider);
        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());
        restoreListener.restore(data.toString(), acknowledgment);
        Mockito.verify(jobService, Mockito.times(0)).queryJob(anyString());
    }
}
