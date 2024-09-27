package openbackup.data.access.framework.protection.listener.v1.archive;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.archive.ArchiveImportListener;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.system.base.common.model.repository.RepositoryType;
import openbackup.system.base.sdk.job.JobCenterRestApi;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * ArchiveImportListener LLT
 *
 * @author z00633516
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-02-12
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ArchiveImportListener.class})
public class ArchiveImportListenerTest {
    private static final String archiveImportMsg = "{\"msg_id\": \"a10d7306-0f6a-4c56-8dba-6b504f20302e\", " +
            "\"request_id\": \"9aea0d86-d66e-49d6-8330-aede53c23fe0\"}";

    private Acknowledgment acknowledgment;

    @Autowired
    ArchiveImportListener archiveImportListener;

    @MockBean
    private KafkaTemplate kafkaTemplate;

    @MockBean
    private JobCenterRestApi jobCenter;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private ProviderManager providerManager;

    @Before
    public void setUp() {
        acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
    }

    /**
     * 用例场景：正常执行副本导入
     * 前置条件：接收到副本导入消息
     * 检查点：无异常抛出
     */
    @Test
    public void test_archive_import() {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.get("repositoryType")).thenReturn(String.valueOf(RepositoryType.TAPE.getType()));
        PowerMockito.when(map.get("storageId")).thenReturn("a2574454-ea7b-4806-864d-c1767d4da035");
        PowerMockito.when(map.get("job_id")).thenReturn("004c6524-5a49-4689-80ee-bf19e2bb7019");
        ArchiveImportProvider provider = PowerMockito.mock(ArchiveImportProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);

        archiveImportListener.archiveImport(archiveImportMsg, acknowledgment);
        Mockito.verify(provider, Mockito.times(1)).scanArchive(any());
    }
}
