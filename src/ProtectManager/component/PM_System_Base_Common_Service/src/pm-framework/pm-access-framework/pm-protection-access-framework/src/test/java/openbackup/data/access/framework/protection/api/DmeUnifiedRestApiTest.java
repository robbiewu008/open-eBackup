package openbackup.data.access.framework.protection.api;

import com.github.tomakehurst.wiremock.junit.WireMockRule;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.rest.SslFeignConfiguration;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.config.FeignClientConfig;
import openbackup.system.base.service.SensitiveDataEliminateService;
import feign.Client;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.http.HttpMessageConvertersAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.cloud.openfeign.EnableFeignClients;
import org.springframework.cloud.openfeign.FeignAutoConfiguration;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Import;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.context.junit4.SpringRunner;

import java.net.URI;

import static com.github.tomakehurst.wiremock.client.WireMock.containing;
import static com.github.tomakehurst.wiremock.client.WireMock.ok;
import static com.github.tomakehurst.wiremock.client.WireMock.post;
import static com.github.tomakehurst.wiremock.client.WireMock.serverError;
import static com.github.tomakehurst.wiremock.client.WireMock.stubFor;
import static org.mockito.BDDMockito.given;

/**
 * DME FeignClient客户端单元测试类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/6
 **/
@ActiveProfiles("test")
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SslFeignConfiguration.class, DmeUnifiedRestApi.class, CommonFeignConfiguration.class})
@EnableFeignClients(clients = {DmeUnifiedRestApi.class})
@Import({FeignAutoConfiguration.class, HttpMessageConvertersAutoConfiguration.class})
@ComponentScan(basePackages = {"com.huawei.oceanprotect.system.base.common.rest"})
public class DmeUnifiedRestApiTest {
    @Rule
    public WireMockRule wireMockRule = new WireMockRule(18089);

    @Rule
    public final ExpectedException exception = ExpectedException.none();

    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @MockBean
    private FeignClientConfig feignClientConfig;

    @MockBean
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    @MockBean
    private SslFeignConfiguration sslFeignConfiguration;

    @Value("${services.endpoints.protectengine.dme}")
    public String test;

    @Before
    public void init(){
        final Client mockClient = new Client.Default(null, (host, session) -> true);
        given(feignClientConfig.getInternalClient()).willReturn(mockClient);
    }

    /**
     * 用例名称：dme创建接口正常响应时，feign client api响应正常<br/>
     * 前置条件：无<br/>
     * check点：接口响应为200，即不会抛异常<br/>
     */
    @Test
    public void should_return_200_when_create_restore_task_given_param_correct() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        stubFor(
                post("/v1/dme-unified/tasks/restore")
                        .withHeader("Content-Type", containing(MediaType.APPLICATION_JSON_VALUE))
                        .willReturn(ok().withHeader("Content-Type", MediaType.APPLICATION_JSON_VALUE)));
        // When
        dmeUnifiedRestApi.createRestoreTask(PowerMockito.mock(URI.class), restoreTask);
        Mockito.verify(dmeUnifiedRestApi, Mockito.times(1)).createRestoreTask(PowerMockito.mock(URI.class), restoreTask);
    }

    /**
     * 用例名称：dme创建接口返回错误响应时，ErrorDecoder是否正确转换异常<br/>
     * 前置条件：无<br/>
     * check点：调用api调用抛出LegoCheckedException<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_create_restore_task_given_error_response() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        JSONObject response = new JSONObject();
        response.set("errorCode", "12345");
        response.set("errorMessage", "test111");
        response.set("detailParams", new String[] {"111", "222"});
        stubFor(
                post("/v1/dme-unified/tasks/restore")
                        .withHeader("Content-Type", containing(MediaType.APPLICATION_JSON_VALUE))
                        .willReturn(
                                serverError()
                                        .withHeader("Content-Type", MediaType.APPLICATION_JSON_VALUE)
                                        .withBody(response.toString())));
        // Then
        exception.expect(LegoCheckedException.class);
        exception.expectMessage("test111");
        // When
        dmeUnifiedRestApi.createRestoreTask(PowerMockito.mock(URI.class), restoreTask);
    }

    /**
     * 用例名称：dme创建接口返回非约定的错误对象，ErrorDecoder是否正确转换异常<br/>
     * 前置条件：无<br/>
     * check点：调用api调用抛出LegoUncheckedException<br/>
     */
    @Test
    public void should_throw_LegoUncheckedException_when_create_restore_task_given_empty_response() {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask restoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        JSONObject response = new JSONObject();
        stubFor(
                post("/v1/dme-unified/tasks/restore")
                        .withHeader("Content-Type", containing(MediaType.APPLICATION_JSON_VALUE))
                        .willReturn(
                                serverError()
                                        .withHeader("Content-Type", MediaType.APPLICATION_JSON_VALUE)
                                        .withBody(response.toString())));
        // Then
        exception.expect(LegoUncheckedException.class);
        // When
        dmeUnifiedRestApi.createRestoreTask(PowerMockito.mock(URI.class), restoreTask);
    }
}
