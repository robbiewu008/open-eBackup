/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

/**
 * GetInitConfig Controller
 *
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE + "/v2/system")
@Slf4j
public class GetInitConfigController {
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
     * @param lld MutipartFile
     * @return 返回查询情况
     * @throws IOException IO异常
     */
    @ExterAttack
    @PostMapping("/lld/action/upload")
    public Object getInitConfig(@RequestParam("lld") MultipartFile lld) throws IOException {
        log.info("start registerComponent ldd");
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        Resource caFileResource = lld.getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("lld", caFileResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(
            NormalizerUtil.normalizeForString(certApi + "/v2/system/lld/action/upload"), httpEntity, Object.class);
        response.setStatus(responseEntity.getStatusCodeValue());
        return responseEntity.getBody();
    }
}


