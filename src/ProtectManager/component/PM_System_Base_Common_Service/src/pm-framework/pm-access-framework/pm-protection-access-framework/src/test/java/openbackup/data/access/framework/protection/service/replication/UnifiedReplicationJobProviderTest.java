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
package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicateService;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationJobProvider;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;

/**
 * OracleBackupProviderTest LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(UnifiedReplicationJobProvider.class)
@AutoConfigureMockMvc
public class UnifiedReplicationJobProviderTest {
    @Mock
    private DmeReplicateService dmeReplicateService;

    @InjectMocks
    private UnifiedReplicationJobProvider unifiedReplicationJobProvider;

    @Test
    public void testStopJob() {
        unifiedReplicationJobProvider.stopJob(UUID.randomUUID().toString());
        Mockito.verify(dmeReplicateService, Mockito.times(1)).abortReplicationTask(any());
    }

    @Test
    public void testApplicable() {
        Assert.assertTrue(unifiedReplicationJobProvider.applicable("Engine_DataMover_Oracle_copy_replication"));
    }
}
