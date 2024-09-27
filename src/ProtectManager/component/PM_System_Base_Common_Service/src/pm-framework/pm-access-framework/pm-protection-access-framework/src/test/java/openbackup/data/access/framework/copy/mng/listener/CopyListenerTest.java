package openbackup.data.access.framework.copy.mng.listener;

import openbackup.data.access.framework.copy.mng.listener.CopyListener;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyProvider;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Before;
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
import java.util.concurrent.atomic.AtomicReference;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doAnswer;

/**
 * CopyListener LLT
 *
 * @author m00576658
 * @since 2021-03-03
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyListener.class)
@AutoConfigureMockMvc
public class CopyListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @InjectMocks
    private CopyListener copyListener;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private JobService jobService;

    @Before
    public void before(){
        Mockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
    }

    @Test
    public void testDelete() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        String copyId = UUID.randomUUID().toString();
        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(any(), any())).thenReturn(null);
        PowerMockito.when(map.get(ContextConstants.COPY_ID)).thenReturn(copyId);

        Copy copy = new Copy();
        copy.setResourceSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.eq(copyId), ArgumentMatchers.anyBoolean())).thenReturn(copy);
        CopyProvider provider = PowerMockito.mock(CopyProvider.class);
        PowerMockito.when(registry.findProvider(any(), any(), any())).thenReturn(provider);

        AtomicReference<Boolean> enableStop = new AtomicReference<>(true);

        PowerMockito.when(deployTypeService.isCloudBackup()).thenReturn(true);

        doAnswer(invocation -> {
            UpdateJobRequest updateJobRequest = invocation.getArgument(1);
            enableStop.set(updateJobRequest.getEnableStop());
            return null;
        }).when(jobCenterRestApi).updateJob(any(), any(), any());

        copyListener.delete(data.toString(), acknowledgment);

        Assert.assertFalse(enableStop.get());
    }

}
