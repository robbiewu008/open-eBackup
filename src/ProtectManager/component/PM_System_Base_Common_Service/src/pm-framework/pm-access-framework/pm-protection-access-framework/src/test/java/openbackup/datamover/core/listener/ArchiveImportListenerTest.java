/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.listener;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.archive.ArchiveExportListener;
import openbackup.data.access.framework.protection.listener.v1.archive.ArchiveImportListener;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.system.base.common.utils.JSONObject;
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

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

/**
 * ArchiveImportListener LLT
 *
 * @author m00576658
 * @since 2021-03-05
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ArchiveExportListener.class)
@AutoConfigureMockMvc
public class ArchiveImportListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @InjectMocks
    private ArchiveImportListener archiveImportListener;

    @Test
    public void testArchiveImport() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());

        RMap map = PowerMockito.mock(RMap.class);

        PowerMockito.when(map.get(eq("repositoryType"))).thenReturn("0");
        PowerMockito.when(redissonClient.getMap(anyString(), eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        ArchiveImportProvider provider = PowerMockito.mock(ArchiveImportProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);

        archiveImportListener.archiveImport(data.toString(), acknowledgment);
        Mockito.verify(registry, Mockito.times(1))
                .findProvider(ArchiveImportProvider.class, (String) map.get("repositoryType"));
    }
}
