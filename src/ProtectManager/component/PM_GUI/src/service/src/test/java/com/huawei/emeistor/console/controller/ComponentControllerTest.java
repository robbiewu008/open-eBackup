/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.bean.CertDetailResponse;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.CustomSubjectReq;
import com.huawei.emeistor.console.controller.request.ImportCertificateFiles;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.*;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;

import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.verify;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ComponentControllerTest.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class,
        RestTemplate.class, RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class,
        ComponentController.class, TimeoutUtils.class})
public class ComponentControllerTest {
    private URI uri;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @MockBean
    private RedissonClient redissonClient;

    @Autowired
    private ComponentController componentController;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @Test
    public void registerComponentSuccess() throws Exception {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        ResponseEntity<Object> responseObjectEntity = getResponseEntity();
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
                .thenReturn(responseObjectEntity);
        componentController.registerComponent("file", file, "1");
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    /**
     * 用例场景：更新HA证书
     * 前置条件：无
     * 检查点: 不报错
     */
    @Test
    public void update_ha_success() {
        ResponseEntity<CertDetailResponse> responseObjectEntity = getCertDetailResponseEntity();
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(CertDetailResponse.class)))
                .thenReturn(responseObjectEntity);
        componentController.updateHaCert("123");
    }

    /**
     * 用例场景：前端传入证书文件，获取证书文件详情
     * 前置条件：证书文件不为空，必传
     * 检查点: 返回了正确的对象，并且安全性字段为true
     */
    @Test
    public void get_cert_detail_success() {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        ResponseEntity<CertDetailResponse> responseObjectEntity = getCertDetailResponseEntity();
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(CertDetailResponse.class)))
                .thenReturn(responseObjectEntity);
        CertDetailResponse certDetail = componentController.getCertDetail(file);
        verify(restTemplate).postForEntity(anyString(), any(), eq(CertDetailResponse.class));
        Assert.assertTrue(certDetail.getIsSafety());
    }

    @Test
    public void importCertificateSuccess() throws Exception {
        ResponseEntity<Object> responseObjectEntity = getResponseEntity();
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
                .thenReturn(responseObjectEntity);
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        ImportCertificateFiles files = new ImportCertificateFiles();
        files.setCaCertificate(file);
        files.setAgentCertificate(file);
        files.setServerKey(file);
        files.setAgentCertificate(file);
        files.setAgentKey(file);
        componentController.importCertificate("123", files, "Admin@123", "System@123");
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    @Test
    public void importCertificateCtlListSuccess() throws Exception {
        ResponseEntity<Object> responseObjectEntity = getResponseEntity();
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
                .thenReturn(responseObjectEntity);
        MultipartFile crl = PowerMockito.mock(MultipartFile.class);
        componentController.importCertificateCtlList("123", crl);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    @Test
    public void exportCertificateRequestSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        byte[] res = componentController.exportCertificateRequest("123", "rsa-2048", null, requestEntity);
        Assert.assertNotNull(res);
    }

    @Test
    public void exportOCCertificateRequestSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        CustomSubjectReq customSubjectReq = new CustomSubjectReq();
        customSubjectReq.setCountry("CN");
        customSubjectReq.setState("STATE");
        customSubjectReq.setCity("NN");
        customSubjectReq.setOrganization("HW");
        customSubjectReq.setOrganizationUnit("UNIT");
        customSubjectReq.setCommonName("CN");
        byte[] res = componentController.
                exportCertificateRequest("123", "rsa-2048", customSubjectReq, requestEntity);
        Assert.assertNotNull(res);
        res = null;
        res = componentController.
                exportCertificateRequest("123", "rsa-2048", null, requestEntity);
        Assert.assertNotNull(res);
    }

    @Test
    public void downloadCaSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        byte[] res = componentController.downloadCa("123", requestEntity);
        Assert.assertNotNull(res);
    }

    /**
     * 用例场景：下载吊销列表
     * 前置条件：组件id和吊销列表id不为空
     * 检查点: 不报错
     */
    @Test
    public void download_crl_success() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        byte[] res = componentController.downloadCrl("123", "456", requestEntity);
        Assert.assertNotNull(res);
    }

    private ResponseEntity<Object> getResponseEntity() {
        Token token = new Token();
        token.setToken("sadfgasfasf");
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(token, HttpStatus.OK);
        return responseObjectEntity;
    }

    private ResponseEntity<CertDetailResponse> getCertDetailResponseEntity() {
        CertDetailResponse certDetailResponse = new CertDetailResponse();
        certDetailResponse.setIsSafety(true);
        ResponseEntity<CertDetailResponse> responseObjectEntity = new ResponseEntity(certDetailResponse, HttpStatus.OK);
        return responseObjectEntity;
    }

    private ResponseEntity<byte[]> getByteResponseEntity() {
        byte[] bytes = {(byte)0xB8};
        MultiValueMap<String, String> headers =  new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        ResponseEntity<byte[]> responseEntity = new ResponseEntity(bytes, headers, HttpStatus.OK);
        return responseEntity;
    }
}
