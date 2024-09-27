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
package openbackup.data.access.framework.copy.mng.handler.v1;

import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.access.framework.copy.index.provider.DeleteCopyIndexProvider;
import openbackup.data.access.framework.copy.mng.handler.v1.CopyDeleteCompleteHandler;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.index.v1.DeleteIndexProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;

import org.junit.Assert;
import org.junit.Test;
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

import java.util.UUID;

/**
 * CopyDeleteCompleteHandler LLT
 *
 * @author m00576658
 * @since 2021-03-26
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyDeleteCompleteHandler.class)
@AutoConfigureMockMvc
public class CopyDeleteCompleteHandlerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private NotifyManager notifyManager;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private ProviderManager providerManager;

    @InjectMocks
    private CopyDeleteCompleteHandler copyDeleteCompleteHandler;

    @Mock
    private DeleteCopyIndexProvider deleteCopyIndexProvider;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Mock
    private CopyManagerService copyManagerService;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Test
    public void testOnTaskCompleteSuccess() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get(ContextConstants.COPY_ID)).thenReturn(UUID.randomUUID().toString());
        Copy copy = new Copy();
        copy.setResourceSubType("vim.VirtualMachine");
        copy.setIndexed("Indexed");
        copy.setChainId(UUID.randomUUID().toString());
        copy.setGn(0);
        copy.setPrevCopyGn(0);
        copy.setNextCopyGn(0);
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString(), eq(false))).thenReturn(copy);
        PowerMockito.doNothing()
            .when(deleteCopyIndexProvider)
            .sendDeleteCopyIndexMessage(ArgumentMatchers.any(), ArgumentMatchers.any());
        DeleteIndexProvider deleteIndexProvider = PowerMockito.mock(DeleteIndexProvider.class);
        PowerMockito.when(providerManager.findProvider(eq(DeleteIndexProvider.class), ArgumentMatchers.anyString()))
            .thenReturn(deleteIndexProvider);
        copyDeleteCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        taskCompleteMessageBo.setJobStatus(6);
        copyDeleteCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
        Assert.assertNotNull(providerManager);
    }

    @Test
    public void testApplicable() {
        Assert.assertTrue(copyDeleteCompleteHandler.applicable("COPY_DELETE"));
        Assert.assertTrue(copyDeleteCompleteHandler.applicable("COPY_EXPIRE"));
    }
}
