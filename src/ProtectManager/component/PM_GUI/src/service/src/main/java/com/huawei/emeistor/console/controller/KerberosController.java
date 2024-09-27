/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.KerberosCreateReq;
import com.huawei.emeistor.console.controller.request.KerberosUpdateReq;
import com.huawei.emeistor.console.controller.response.KerberosIDResp;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

/**
 * Kerberos Rest Controller
 *
 * @author m00576658
 * @since 2021-08-13
 */
@RestController
@Slf4j
@RequestMapping(ConfigConstant.CONSOLE + "/v1/kerberos")
@Validated
public class KerberosController {
    private static final String KERBEROS = "/v1/kerberos";

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String kerberosUrl;

    /**
     * 创建kerberos
     *
     * @param kerberosCreateReq kerberosCreateReq
     * @param krb5File krb5File
     * @param keytabFile keytabFile
     * @return KerberosID
     */
    @ExterAttack
    @PostMapping
    @ResponseBody
    public KerberosIDResp createKerberos(KerberosCreateReq kerberosCreateReq,
        @RequestParam(required = false) MultipartFile krb5File,
        @RequestParam(required = false) MultipartFile keytabFile) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("name", NormalizerUtil.normalizeForString(kerberosCreateReq.getName()));
        map.add("password", NormalizerUtil.normalizeForString(kerberosCreateReq.getPassword()));
        map.add("principalName", NormalizerUtil.normalizeForString(kerberosCreateReq.getPrincipalName()));
        map.add("createModel", NormalizerUtil.normalizeForString(kerberosCreateReq.getCreateModel()));
        if (keytabFile != null && !keytabFile.isEmpty()) {
            map.add("keytabFile", keytabFile.getResource());
        }
        if (krb5File != null && !krb5File.isEmpty()) {
            map.add("krb5File", krb5File.getResource());
        }
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);

        String createUrl = NormalizerUtil.normalizeForString(kerberosUrl + KERBEROS);
        ResponseEntity<KerberosIDResp> response = restTemplate.exchange(createUrl, HttpMethod.POST, httpEntity,
            KerberosIDResp.class);
        return response.getBody();
    }

    /**
     * 更新kerberos
     *
     * @param kerberosId kerberosId
     * @param updateReq updateReq
     * @param krb5File krb5File
     * @param keytabFile keytabFile
     */
    @ExterAttack
    @PostMapping("/{kerberos_id}")
    @ResponseBody
    public void updateKerberos(@PathVariable("kerberos_id") String kerberosId, KerberosUpdateReq updateReq,
        @RequestParam(required = false) MultipartFile krb5File,
        @RequestParam(required = false) MultipartFile keytabFile) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("name", NormalizerUtil.normalizeForString(updateReq.getName()));
        map.add("password", NormalizerUtil.normalizeForString(updateReq.getPassword()));
        map.add("principalName", NormalizerUtil.normalizeForString(updateReq.getPrincipalName()));
        map.add("createModel", NormalizerUtil.normalizeForString(updateReq.getCreateModel()));
        if (keytabFile != null && !keytabFile.isEmpty()) {
            map.add("keytabFile", keytabFile.getResource());
        }
        if (krb5File != null && !krb5File.isEmpty()) {
            map.add("krb5File", krb5File.getResource());
        }
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);

        String createUrl = NormalizerUtil.normalizeForString(kerberosUrl + KERBEROS + "/");
        restTemplate.postForEntity(NormalizerUtil.normalizeForString(createUrl + kerberosId), httpEntity, Object.class);
    }
}
