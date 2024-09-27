package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.copy.index.listener.v1.CopyIndexResponseListener;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.UUID;

/**
 * CopyIndexListenerTest LLT
 *
 * @author m00576658
 * @since 2021-03-22
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyIndexResponseListener.class)
@AutoConfigureMockMvc
public class CopyIndexResponseListenerTest {

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @InjectMocks
    private CopyIndexResponseListener CopyIndexResponseListener;

    @Before
    public void createMocks() {
        MockitoAnnotations.initMocks(this);
    }

    /*
     * 用例名称：副本索引创建成功
     * 前置条件：反馈的kafka消息中的status是success
     * check点：将副本状态标记为了“已索引”
     */
    @Test
    public void copy_Index_response_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        String copyId = UUID.randomUUID().toString();
        JSONObject data = new JSONObject();
        data.set(ContextConstants.COPY_ID, copyId);
        data.set(CopyIndexConstants.STATUS, CopyIndexConstants.SUCCESS);
        PowerMockito.doNothing()
            .when(copyRestApi)
            .updateCopyIndexStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        CopyIndexResponseListener.copyIndexResponse(data.toString(), acknowledgment);

        Mockito.verify(copyRestApi).updateCopyIndexStatus(copyId, CopyIndexStatus.INDEXED.getIndexStaus(), null);
    }

    /*
     * 用例名称：副本索引创建失败
     * 前置条件：反馈的kafka消息中的status是failure，消息包含errorCode
     * check点：将副本状态标记为了“索引失败”，并反馈了对应的errorCode
     */
    @Test
    public void copy_Index_response_fail_when_response_index_task_execute_fail() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        String copyId = UUID.randomUUID().toString();
        JSONObject data = new JSONObject();
        data.set(ContextConstants.COPY_ID, copyId);
        data.set(CopyIndexConstants.STATUS, "failure");
        data.set(ContextConstants.ERROR_CODE, CopyIndexStatus.INDEX_SCAN_RESPONSE_ERROR_LABEL.getIndexStaus());
        PowerMockito.doNothing()
            .when(copyRestApi)
            .updateCopyIndexStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        CopyIndexResponseListener.copyIndexResponse(data.toString(), acknowledgment);

        Mockito.verify(copyRestApi)
            .updateCopyIndexStatus(copyId, CopyIndexStatus.INDEX_FAIL.getIndexStaus(),
                CopyIndexStatus.INDEX_SCAN_RESPONSE_ERROR_LABEL.getIndexStaus());
    }
}
