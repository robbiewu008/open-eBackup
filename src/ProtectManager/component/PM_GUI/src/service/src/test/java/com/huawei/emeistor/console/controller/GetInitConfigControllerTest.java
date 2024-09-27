package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpHeaders;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {GetInitConfigController.class, RequestUtil.class})
public class GetInitConfigControllerTest {
    @InjectMocks
    private GetInitConfigController getInitConfigController;

    @Mock
    private HttpServletResponse response;

    @Mock
    private RestTemplate restTemplate;

    @Mock
    private RequestUtil requestUtil;

    @Test
    public void test_get_init_config_success() throws IOException {
        MultipartFile multipartFile = PowerMockito.mock(MultipartFile.class);
        HttpHeaders headers = new HttpHeaders();
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<Object> responseEntity = PowerMockito.mock(ResponseEntity.class);
        PowerMockito.when(responseEntity.getStatusCodeValue()).thenReturn(200);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class))).thenReturn(responseEntity);
        getInitConfigController.getInitConfig(multipartFile);
    }
}
