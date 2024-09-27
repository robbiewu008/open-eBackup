package openbackup.system.base.service;

import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;

import feign.FeignException;
import openbackup.system.base.service.KubernatesService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * KubernatesServiceTest
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-26
 */
@SpringBootTest(classes = {KubernatesService.class})
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
public class KubernatesServiceTest {
    @Autowired
    private KubernatesService kubernatesService;

    @MockBean
    private InfrastructureRestApi infrastructureRestApi;

    @MockBean
    private NodeRestApi nodeRestApi;

    /**
     * 用例场景：同步事件转储文件
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_syncAlarmDumpFile_success() {
        InfraResponseWithError<List<String>> endpointsResponse = new InfraResponseWithError<>();
        List<String> ipList = Collections.singletonList("8.40.102.81");
        endpointsResponse.setData(ipList);

        PowerMockito.when(infrastructureRestApi.getEndpoints("")).thenReturn(endpointsResponse);
        kubernatesService.syncAlarmDumpFile(null, "", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：同步事件转储文件
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_syncAlarmDumpFile_when_ip_list_empty_then_success() {
        InfraResponseWithError<List<String>> endpointsResponse = new InfraResponseWithError<>();
        endpointsResponse.setData(new ArrayList<>());

        PowerMockito.when(infrastructureRestApi.getEndpoints("")).thenReturn(endpointsResponse);
        kubernatesService.syncAlarmDumpFile(null, "", "");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：同步事件转储文件
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_syncAlarmDumpFile_when_feign_exception_then_success() {
        FeignException feignException = PowerMockito.mock(FeignException.class);
        PowerMockito.when(feignException.getMessage()).thenReturn("");
        PowerMockito.when(feignException.getStackTrace()).thenReturn(new StackTraceElement[0]);
        PowerMockito.when(infrastructureRestApi.getEndpoints("")).thenThrow(feignException);
        kubernatesService.syncAlarmDumpFile(null, "", "");
        Assert.assertTrue(true);
    }
}