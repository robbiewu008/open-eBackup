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
 * @author m00576658
 * @since 2021-02-25
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
