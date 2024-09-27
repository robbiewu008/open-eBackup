package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.copy.index.listener.v1.AsyncGenIndex;
import openbackup.data.access.framework.copy.index.listener.v1.CopyIndexListener;
import openbackup.data.access.framework.copy.mng.listener.CopyListener;
import openbackup.data.access.framework.core.common.model.CopyIndexResponse;
import openbackup.data.protection.access.provider.sdk.index.IndexerProvider;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.ServiceManager;
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

import java.util.Collections;
import java.util.List;
import java.util.UUID;

/**
 * CopyIndexListenerTest LLT
 *
 * @author m00576658
 * @since 2021-03-22
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyListener.class)
@AutoConfigureMockMvc
public class CopyIndexListenerTest {

    private static final String SLA_PROPERTIES = "{\"name\":\"wyx_rep\",\"type\":1,\"application\":\"vim.VirtualMachine\",\"created_time\":\"2021-07-09T17:46:31.328507\",\"uuid\":\"41de3cb7-8c4f-42ba-ae2d-e3ca6ae33fa1\",\"is_global\":false,\"policy_list\":[{\"uuid\":\"da505246-400f-42ac-9e68-6f8348e49eb5\",\"name\":\"wyx\",\"action\":\"replication\",\"ext_parameters\":{\"qos_id\":\"\",\"external_system_id\":\"4\"},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":7},\"schedule\":{\"trigger\":2,\"interval\":0,\"interval_unit\":\"h\",\"start_time\":\"2021-07-09T17:47:21\",\"window_start\":null,\"window_end\":null},\"type\":\"replication\"},{\"uuid\":\"2aecb57d-41e7-4c26-ac2d-ccc7eb112021\",\"name\":\"full\",\"action\":\"full\",\"ext_parameters\":{\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"qos_id\":\"\",\"fine_grained_restore\":false,\"ensure_consistency_backup\":true},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":7},\"schedule\":{\"trigger\":1,\"interval\":4,\"interval_unit\":\"h\",\"start_time\":\"2021-07-09T17:46:35\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\"},\"type\":\"backup\"}],\"resource_count\":null,\"archival_count\":null,\"replication_count\":null}";

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private ServiceManager serviceManager;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private InfrastructureRestApi infrastructureRestApi;

    @Mock
    private AsyncGenIndex asyncGenIndex;

    @InjectMocks
    private CopyIndexListener copyIndexListener;

    @Test
    public void testGenIndexer() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        CopyIndexResponse copyIndexResponse = new CopyIndexResponse();
        copyIndexResponse.setCopyId(UUID.randomUUID().toString());
        copyIndexResponse.setRequestId(UUID.randomUUID().toString());
        copyIndexResponse.setGenIndex("manual");
        Copy copy = new Copy();
        copy.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());

        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);
        InfraResponseWithError<List<NodePodInfo>> podInfos = new InfraResponseWithError<>();
        NodePodInfo nodePodInfo = new NodePodInfo();
        nodePodInfo.setPodName("dataenableengine-server-test");
        nodePodInfo.setNamespace("dpa");
        nodePodInfo.setPodStatus("Running");
        List<NodePodInfo> nodePodInfos = Collections.singletonList(nodePodInfo);
        podInfos.setData(nodePodInfos);
        PowerMockito.when(infrastructureRestApi.getInfraPodInfo(ArgumentMatchers.anyString())).thenReturn(podInfos);
        IndexerProvider provider = PowerMockito.mock(IndexerProvider.class);
        PowerMockito.when(serviceManager.getService(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);
        JSONObject data = JSONObject.fromObject(copyIndexResponse);
        PowerMockito.doNothing().when(asyncGenIndex).generateIndexFileAsync(ArgumentMatchers.any(), ArgumentMatchers.any());
        copyIndexListener.genIndexer(data.toString(), acknowledgment);
        Mockito.verify(acknowledgment, Mockito.times(2)).acknowledge();
    }

    @Test
    public void testGenIndexFail() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        CopyIndexResponse copyIndexResponse = new CopyIndexResponse();
        copyIndexResponse.setCopyId(UUID.randomUUID().toString());
        copyIndexResponse.setRequestId(UUID.randomUUID().toString());
        copyIndexResponse.setGenIndex("auto");
        Copy copy = new Copy();
        copy.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setSlaProperties(SLA_PROPERTIES);

        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);
        InfraResponseWithError<List<NodePodInfo>> podInfos = new InfraResponseWithError<>();
        NodePodInfo nodePodInfo = new NodePodInfo();
        nodePodInfo.setPodName("dataenableengine-server-test");
        nodePodInfo.setNamespace("dpa");
        nodePodInfo.setPodStatus("Running");
        List<NodePodInfo> nodePodInfos = Collections.singletonList(nodePodInfo);
        podInfos.setData(nodePodInfos);
        PowerMockito.when(infrastructureRestApi.getInfraPodInfo(ArgumentMatchers.anyString())).thenReturn(podInfos);
        IndexerProvider provider = PowerMockito.mock(IndexerProvider.class);
        PowerMockito.when(serviceManager.getService(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);
        JSONObject data = JSONObject.fromObject(copyIndexResponse);
        copyIndexListener.genIndexer(data.toString(), acknowledgment);
        Mockito.verify(acknowledgment, Mockito.times(2)).acknowledge();
    }

}
