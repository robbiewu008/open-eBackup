/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.listener.task.handler;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.v1.archive.ArchiveImportTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import org.junit.Assert;
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

import java.util.UUID;

/**
 * ArchiveImportTaskCompleteHandler LLT
 *
 * @author m00576658
 * @since 2021-03-26
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ArchiveImportTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class ArchiveImportTaskCompleteHandlerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @InjectMocks
    private ArchiveImportTaskCompleteHandler archiveImportTaskCompleteHandler;

    @Test
    public void testOnTaskCompleteSuccess() {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        ArchiveImportProvider provider = PowerMockito.mock(ArchiveImportProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);

        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);
        archiveImportTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        archiveImportTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
        Mockito.verify(registry, Mockito.times(2)).
                findProvider(ArgumentMatchers.any(), ArgumentMatchers.any());
    }

    @Test
    public void testApplicable() {
        boolean applicable = archiveImportTaskCompleteHandler.applicable("archive_import");
        Assert.assertTrue(applicable);
    }
}
