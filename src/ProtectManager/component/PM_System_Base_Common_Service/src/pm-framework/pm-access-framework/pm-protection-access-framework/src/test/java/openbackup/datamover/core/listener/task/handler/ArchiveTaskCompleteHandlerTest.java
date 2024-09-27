package openbackup.datamover.core.listener.task.handler;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.v1.archive.ArchiveTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.archive.ArchiveTaskService;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.protection.model.PolicyBo;
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

import static org.mockito.ArgumentMatchers.any;
import static org.powermock.api.mockito.PowerMockito.when;

/**
 * ArchiveTaskCompleteHandler LLT
 *
 * @author m00576658
 * @since 2021-03-26
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ArchiveTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class ArchiveTaskCompleteHandlerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private NotifyManager notifyManager;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private ProviderManager registry;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Mock
    private ArchiveTaskService archiveTaskService;

    @InjectMocks
    private ArchiveTaskCompleteHandler archiveTaskCompleteHandler;

    @Test
    public void testOnTaskCompleteSuccess() throws JsonProcessingException {
        RMap map = PowerMockito.mock(RMap.class);
        when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        when(map.put(any(), any())).thenReturn(null);
        when(map.get(ArgumentMatchers.eq("job_type"))).thenReturn("archive");
        when(map.get(ArgumentMatchers.eq("resource_sub_type"))).thenReturn("Oracle");
        PolicyBo policyBo = new PolicyBo();
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("storage_id", 1);
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(jsonObject.toString());
        policyBo.setExtParameters(node);
        when(map.get(ArgumentMatchers.eq("resource_sub_type"))).thenReturn("Oracle");
        when(map.get(ArgumentMatchers.eq("policy"))).thenReturn(JSONObject.fromObject(policyBo).toString());

        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);
        CopyProvider copyProvider = PowerMockito.mock(CopyProvider.class);
        when(registry.findProvider(any(), any())).thenReturn(copyProvider);
        PowerMockito.doNothing().when(userQuotaManager).increaseUsedQuota(Mockito.anyString(),Mockito.any());
        when(copyProvider.buildCopy(any(), any(), any())).thenReturn(new CopyInfoBo());
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid(UUID.randomUUID().toString());
        when(copyRestApi.saveCopy(any())).thenReturn(uuidObject);
        archiveTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        taskCompleteMessageBo.setJobStatus(6);
        archiveTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
        Assert.assertNotNull(6);
    }

    @Test
    public void testApplicable() {
        archiveTaskCompleteHandler.applicable("archive");
    }
}