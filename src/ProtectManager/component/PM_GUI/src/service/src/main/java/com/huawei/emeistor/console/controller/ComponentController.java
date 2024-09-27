/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.bean.CertDetailResponse;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.CustomSubjectReq;
import com.huawei.emeistor.console.controller.request.ImportCertificateFiles;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;
import javax.validation.Valid;
import javax.validation.constraints.Pattern;

/**
 * Component Controller
 *
 * @author l00557046
 * @version [OceanStor 100 8.1]
 * @since 2020-06-16
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE + "/v1/certs")
public class ComponentController {
    private static final String COMPONENT_URL = "/v1/certs/components/";
    private static final String CRL_IMPORT = "/crls/action/import";
    private static final String REQ_EXPORT = "/request-file/action/export";
    private static final String DOWNLOAD = "/ca/action/download";
    private static final String CERT_IMPORT = "/action/import";
    private static final String HA_UPDATE = "/ha/action/update";
    private static final String PUSH_UPDATE = "/action/push-update";

    @Autowired
    private SessionService sessionService;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String certApi;

    /**
     * 注册外部组件
     *
     * @param name String
     * @param ca MutipartFile
     * @param type string
     * @throws IOException IO异常
     */
    @ExterAttack
    @PostMapping("/components")
    public void registerComponent(
        @RequestParam("name") String name,
        @RequestParam("ca") MultipartFile ca, @RequestParam("type") String type) throws IOException {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        Resource caFileResource = ca.getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("ca", caFileResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(NormalizerUtil.normalizeForString(
            certApi + "/v1/certs/components" + "?name=" + name + "&type=" + type), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
    }

    /**
     * 获取证书详情
     *
     * @param cert 证书文件
     * @return 证书详情
     */
    @ExterAttack
    @PostMapping("/detail")
    public CertDetailResponse getCertDetail(@RequestParam("cert") MultipartFile cert) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        Resource certFileResource = cert.getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("cert", certFileResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<CertDetailResponse> responseEntity = restTemplate.postForEntity(
                NormalizerUtil.normalizeForString(certApi + "/v1/certs/detail"),
                httpEntity, CertDetailResponse.class);
        response.setStatus(responseEntity.getStatusCodeValue());
        return responseEntity.getBody();
    }

    /**
     * 替换证书
     *
     * @param componentId   组件ID
     * @param files 导入文件
     * @param serverPass    服务器私钥密码
     * @param agentPass    服务器私钥密码
     * @throws IOException  IO异常
     */
    @ExterAttack
    @PostMapping("/components/{componentId}/action/import")
    public void importCertificate(
        @PathVariable("componentId") String componentId, ImportCertificateFiles files,
        @RequestHeader(value = "serverPass", required = false) String serverPass,
        @RequestHeader(value = "agentPass", required = false) String agentPass) throws IOException {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        headers.set("serverPass", NormalizerUtil.normalizeForString(serverPass));
        headers.set("agentPass", NormalizerUtil.normalizeForString(agentPass));
        Resource caFileResource = files.getCaCertificate().getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("caCertificate", caFileResource);
        if (files.getServerCertificate() != null) {
            Resource certFileResource = files.getServerCertificate().getResource();
            map.add("serverCertificate", certFileResource);
        }
        if (files.getServerKey() != null) {
            Resource keyFileResource = files.getServerKey().getResource();
            map.add("serverKey", keyFileResource);
        }
        if (files.getDhParam() != null) {
            Resource keyFileResource = files.getDhParam().getResource();
            map.add("dhParam", keyFileResource);
        }
        if (files.getAgentCertificate() != null) {
            Resource certFileResource = files.getAgentCertificate().getResource();
            map.add("agentCertificate", certFileResource);
        }
        if (files.getAgentKey() != null) {
            Resource keyFileResource = files.getAgentKey().getResource();
            map.add("agentKey", keyFileResource);
        }
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(NormalizerUtil.normalizeForString(
            certApi + COMPONENT_URL + componentId + CERT_IMPORT), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
    }

    /**
     * 推送更新证书
     *
     * @param componentId   组件ID
     * @param files 导入文件
     * @param serverPass    服务器私钥密码
     * @param agentPass    服务器私钥密码
     * @throws IOException  IO异常
     */
    @ExterAttack
    @PostMapping("/components/{componentId}/action/push-update")
    public void pushUpdateCertificate(
        @PathVariable("componentId") String componentId, ImportCertificateFiles files,
        @RequestHeader(value = "serverPass", required = false) String serverPass,
        @RequestHeader(value = "agentPass", required = false) String agentPass) throws IOException {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        headers.set("serverPass", NormalizerUtil.normalizeForString(serverPass));
        headers.set("agentPass", NormalizerUtil.normalizeForString(agentPass));
        Resource caFileResource = files.getCaCertificate().getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("caCertificate", caFileResource);
        if (files.getServerCertificate() != null) {
            Resource certFileResource = files.getServerCertificate().getResource();
            map.add("serverCertificate", certFileResource);
        }
        if (files.getServerKey() != null) {
            Resource keyFileResource = files.getServerKey().getResource();
            map.add("serverKey", keyFileResource);
        }
        if (files.getDhParam() != null) {
            Resource keyFileResource = files.getDhParam().getResource();
            map.add("dhParam", keyFileResource);
        }
        if (files.getAgentCertificate() != null) {
            Resource certFileResource = files.getAgentCertificate().getResource();
            map.add("agentCertificate", certFileResource);
        }
        if (files.getAgentKey() != null) {
            Resource keyFileResource = files.getAgentKey().getResource();
            map.add("agentKey", keyFileResource);
        }
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(NormalizerUtil.normalizeForString(
            certApi + COMPONENT_URL + componentId + PUSH_UPDATE), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
    }

    /**
     * 更新HA证书
     *
     * @param componentId 组件ID
     */
    @ExterAttack
    @PostMapping("/components/{componentId}/ha/action/update")
    public void updateHaCert(@PathVariable("componentId") String componentId) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(NormalizerUtil.normalizeForString(
                certApi + COMPONENT_URL + componentId + HA_UPDATE), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
    }

    /**
     * 导入吊销列表
     *
     * @param componentId 组件ID
     * @param crl 吊销列表
     * @throws IOException IO异常
     */
    @ExterAttack
    @PostMapping("/components/{componentId}/crls/action/import")
    public void importCertificateCtlList(
        @PathVariable("componentId") String componentId,
        @RequestParam(value = "crl", required = false) MultipartFile crl) throws IOException {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        Resource caFileResource = crl.getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("crl", caFileResource);
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(NormalizerUtil.normalizeForString(
            certApi + COMPONENT_URL + componentId + CRL_IMPORT), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
    }

    /**
     * 导出请求文件
     *
     * @param componentId 组件ID
     * @param algorithm 算法名称
     * @param customSubjectReq 自定义subject信息
     * @param requestEntity requestEntity
     * @return byte[]
     */
    @ExterAttack
    @GetMapping("/components/{componentId}/request-file/action/export")
    public byte[] exportCertificateRequest(
        @PathVariable("componentId") String componentId,
        @RequestParam("algorithm")
        @Pattern(regexp = "rsa-2048|rsa-4096") String algorithm,
        @Valid CustomSubjectReq customSubjectReq,
        RequestEntity requestEntity) {
        return getBytes(certApi + COMPONENT_URL
            + componentId + REQ_EXPORT + "?algorithm=" + algorithm
            + genSubjectQueryStr(customSubjectReq), requestEntity);
    }

    private String genSubjectQueryStr(CustomSubjectReq customSubjectReq) {
        StringBuilder sb = new StringBuilder();
        if (customSubjectReq == null) {
            return sb.toString();
        }
        if (!StringUtils.isEmpty(customSubjectReq.getCountry())) {
            sb.append("&country=").append(customSubjectReq.getCountry());
        }
        if (!StringUtils.isEmpty(customSubjectReq.getState())) {
            sb.append("&state=").append(customSubjectReq.getState());
        }
        if (!StringUtils.isEmpty(customSubjectReq.getCity())) {
            sb.append("&city=").append(customSubjectReq.getCity());
        }
        if (!StringUtils.isEmpty(customSubjectReq.getOrganization())) {
            sb.append("&organization=").append(customSubjectReq.getOrganization());
        }
        if (!StringUtils.isEmpty(customSubjectReq.getOrganizationUnit())) {
            sb.append("&organizationUnit=").append(customSubjectReq.getOrganizationUnit());
        }
        if (!StringUtils.isEmpty(customSubjectReq.getCommonName())) {
            sb.append("&commonName=").append(customSubjectReq.getCommonName());
        }
        return sb.toString();
    }

    /**
     * 下载CA
     *
     * @param componentId 组件ID
     * @param requestEntity requestEntity
     * @return byte[]
     */
    @ExterAttack
    @GetMapping("/components/{componentId}/ca/action/download")
    public byte[] downloadCa(
        @PathVariable("componentId") String componentId,
        RequestEntity requestEntity) {
        return getBytes(certApi + COMPONENT_URL + componentId + DOWNLOAD, requestEntity);
    }

    /**
     * 下载吊销列表
     *
     * @param componentId 组件ID
     * @param crlId crl id
     * @param requestEntity requestEntity
     * @return byte[]
     */
    @ExterAttack
    @GetMapping("/components/{componentId}/crl/action/download")
    public byte[] downloadCrl(
            @PathVariable("componentId") String componentId, @RequestParam("crlId") String crlId,
            RequestEntity requestEntity) {
        return getBytes(certApi + COMPONENT_URL + componentId + "/crl/action/download?crlId=" + crlId, requestEntity);
    }

    private byte[] getBytes(String url, RequestEntity requestEntity) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(requestEntity.getBody(),
            headers);
        ResponseEntity<byte[]> responseEntity = restTemplate.exchange(NormalizerUtil.normalizeForString(url),
            HttpMethod.GET, httpEntity, byte[].class);
        response.setStatus(responseEntity.getStatusCodeValue());
        response.setHeader(ConfigConstant.CONTENT_DISPOSITION,
            responseEntity.getHeaders().getContentDisposition().toString());
        return responseEntity.getBody();
    }
}


