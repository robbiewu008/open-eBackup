package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.ContentDisposition;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ReportController.class, RequestUtil.class, ContentDisposition.class})
public class ReportControllerTest {
    private URI uri;

    private static final String SESSION_VALUSE
        = "userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private final String url = "https:/v1/report";

    @InjectMocks
    private ReportController reportController;

    @Mock
    private RequestUtil requestUtil;

    @Mock
    private RestTemplate restTemplate;

    private HttpServletRequest request = new MockHttpServletRequest();

    private HttpServletResponse response = new MockHttpServletResponse();

    @Before
    public void setup() {
        ReflectionTestUtils.setField(reportController, "request", request);
        ReflectionTestUtils.setField(reportController, "response", response);
        Token token = new Token();
        token.setToken("sadfgasfasf");
        try {
            uri = new URL(NormalizerUtil.normalizeForString(url)).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        Cookie[] cookies = new Cookie[] {new Cookie(ConfigConstant.SESSION, SESSION_VALUSE)};
        request.setAttribute("Cookies", cookies);
    }

    @Test
    public void test_download_report_success() throws Exception {
        byte[] bytes = {(byte) 0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        HttpEntity httpEntity = PowerMockito.mock(HttpEntity.class);
        PowerMockito.whenNew(HttpEntity.class).withAnyArguments().thenReturn(httpEntity);
        HttpHeaders headers = PowerMockito.mock(HttpHeaders.class);
        ContentDisposition contentDisposition = PowerMockito.mock(ContentDisposition.class);
        PowerMockito.when(headers.getContentDisposition()).thenReturn(contentDisposition);
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class)))
            .thenReturn(responseEntity);
        reportController.downloadReport("testReportId", requestEntity);
    }

    @Test
    public void test_batch_download_report_success() throws Exception {
        byte[] bytes = {(byte) 0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        HttpEntity httpEntity = PowerMockito.mock(HttpEntity.class);
        PowerMockito.whenNew(HttpEntity.class).withAnyArguments().thenReturn(httpEntity);
        HttpHeaders headers = PowerMockito.mock(HttpHeaders.class);
        ContentDisposition contentDisposition = PowerMockito.mock(ContentDisposition.class);
        PowerMockito.when(headers.getContentDisposition()).thenReturn(contentDisposition);
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class)))
            .thenReturn(responseEntity);
        reportController.batchDownloadReport(requestEntity);
    }

    private ResponseEntity<byte[]> getByteResponseEntity() {
        byte[] bytes = {(byte) 0xB8};
        MultiValueMap<String, String> headers = new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        ResponseEntity<byte[]> responseEntity = new ResponseEntity(bytes, headers, HttpStatus.OK);
        return responseEntity;
    }
}
