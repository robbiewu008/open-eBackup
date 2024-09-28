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
package openbackup.datamover.core.listener;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.archive.ArchiveExportListener;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.system.base.common.model.repository.RepositoryType;
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
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.UUID;

/**
 * ArchiveExportListener LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ArchiveExportListener.class)
@AutoConfigureMockMvc
public class ArchiveExportListenerTest {
    @Mock
    private ProviderManager registry;

    @InjectMocks
    private ArchiveExportListener archiveExportListener;

    @Test
    public void testArchiveImport() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();

        data.set("request_id", UUID.randomUUID().toString());
        data.set("storage_Id", UUID.randomUUID().toString());
        data.set("job_id", UUID.randomUUID().toString());
        data.set("copy_id", UUID.randomUUID().toString());
        data.set("archive_copy_id", UUID.randomUUID().toString());
        data.set("auto_retry_times",3);

        ArchiveImportProvider provider = PowerMockito.mock(ArchiveImportProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);
        archiveExportListener.archiveImport(data.toString(), acknowledgment);
        Mockito.verify(registry, Mockito.times(1))
                .findProvider( ArchiveImportProvider.class, String.valueOf(RepositoryType.S3.getType()));
    }
}
