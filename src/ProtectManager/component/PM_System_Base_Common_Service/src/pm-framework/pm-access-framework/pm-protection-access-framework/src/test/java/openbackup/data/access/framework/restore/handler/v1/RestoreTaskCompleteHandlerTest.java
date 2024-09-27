package openbackup.data.access.framework.restore.handler.v1;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.restore.handler.v1.RestoreTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.VMWareRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
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
 * RestoreTaskCompleteHandler LLT
 *
 * @author m00576658
 * @since 2021-03-26
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(RestoreTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class RestoreTaskCompleteHandlerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private VMWareRestApi vmWareRestApi;

    @Mock
    private JobCenterRestApi jobCenter;

    @Mock
    private NotifyManager notifyManager;

    @InjectMocks
    private RestoreTaskCompleteHandler restoreTaskCompleteHandler;

    @Test
    public void testOnTaskCompleteSuccess() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);
        JSONObject ext = new JSONObject();
        ext.put("parentUuid", "parentUuid");
        ext.put("vm_name", "vm_name");
        ext.put("isDeleteOriginalVM", "true");
        Copy copy = new Copy();
        copy.setResourceId("resource_id");

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.get(ContextConstants.COPY_ID)).thenReturn(UUID.randomUUID().toString());
        PowerMockito.when(map.get("restore_copy_id")).thenReturn(UUID.randomUUID().toString());
        PowerMockito.when(map.get("sub_type")).thenReturn("MySQL");
        PowerMockito.when(map.get("object_type")).thenReturn(ResourceSubTypeEnum.VMWARE.getType());
        PowerMockito.when(map.get("ext_parameters")).thenReturn(ext);
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);

        restoreTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        taskCompleteMessageBo.setJobStatus(6);
        restoreTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
        Mockito.verify(redissonClient, Mockito.times(2))
                .getMap(taskCompleteMessageBo.getJobRequestId(), StringCodec.INSTANCE);
    }

    @Test
    public void testApplicable() {
        Assert.assertTrue(restoreTaskCompleteHandler.applicable("INSTANT_RESTORE"));
        Assert.assertTrue(restoreTaskCompleteHandler.applicable("RESTORE"));
    }
}
