package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.sdk.alarm.AlarmRestApi;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;
import static org.mockito.ArgumentMatchers.any;

/**
 * Operation Log Service Test
 *
 * @author c30044692
 * @since 2023/4/20
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OperationLogService.class)
public class OperationLogServiceTest {

    @Resource
    private OperationLogService operationLogService;

    @MockBean
    private AlarmRestApi AlarmRestApi;

    /**
     * 测试场景：正确处理 <br/>
     * 前置条件：AlarmRestApi处理成功 <br/>
     * 检查点：正确处理
     */
    @Test
    public void test_send_event_success () {
        Mockito.doNothing().when(AlarmRestApi).generateSystemLog(any());
        operationLogService.sendEvent(new LegoInternalEvent());
    }

    /**
     * 测试场景：异常处理情况 <br/>
     * 前置条件：AlarmRestApi处理异常 <br/>
     * 检查点：不抛出异常
     */
    @Test
    public void test_send_event_exception () {
        Mockito.doThrow(new RuntimeException("xx")).when(AlarmRestApi).generateSystemLog(any());
        operationLogService.sendEvent(new LegoInternalEvent());
    }
}