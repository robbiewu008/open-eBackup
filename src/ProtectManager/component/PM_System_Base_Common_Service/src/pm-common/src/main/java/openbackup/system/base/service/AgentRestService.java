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
package openbackup.system.base.service;

import static openbackup.system.base.service.hostagent.constant.HostAgentConstant.AGENT_UP_LOAD_API;
import static openbackup.system.base.service.hostagent.constant.HostAgentConstant.UP_LOAD_FILE_API;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.request.UploadFileRequest;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.InputStreamResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.http.client.MultipartBodyBuilder;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.io.InputStream;
import java.net.URI;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.text.Normalizer;

/**
 * k8s集群节点间通信服务
 *
 */
@Service
@Slf4j
public class AgentRestService {
    private static final long WAIT_TIME = 3 * 1000L;

    @Qualifier("agentClientRestTemplate")
    @Autowired
    private RestTemplate restTemplate;

    /**
     * 集群同步agent软件包
     *
     * @param newBiggestFile 文件路径
     * @param ip 待转发的的节点ip
     */
    @Async("agentSynchronousAgentClientExecutor")
    @Retryable(backoff = @Backoff(delay = WAIT_TIME))
    public void synchronousAgentClient(Path newBiggestFile, String ip) {
        try {
            String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(Constants.PM_ENDPOINT_NAME)
                + AGENT_UP_LOAD_API;
            log.info("Current uri for synchronous is: {}.", nodeUri);
            URI uri = new URL(normalizeForString(nodeUri)).toURI();
            HttpHeaders headers = new HttpHeaders();
            headers.setContentType(MediaType.APPLICATION_OCTET_STREAM);
            try (InputStream inputStream = Files.newInputStream(newBiggestFile)) {
                HttpEntity<InputStreamResource> requestEntity = new HttpEntity<>(new InputStreamResource(inputStream),
                    headers);
                restTemplate.exchange(uri, HttpMethod.PUT, requestEntity, Void.class);
            }
        } catch (Exception e) {
            log.error("Synchronous agent client failed, fail ip is: {}.", ip, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
    }

    /**
     * 集群同步agent包 只负责传文件到其他节点
     *
     * @param newBiggestFile 文件路径
     * @param ip 待转发的的节点ip
     */

    @Retryable(backoff = @Backoff(delay = WAIT_TIME))
    public void synchronousFile(Path newBiggestFile, String ip) {
        try {
            String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(Constants.PM_ENDPOINT_NAME)
                + UP_LOAD_FILE_API;
            log.info("Current uri for synchronous is: {}.", nodeUri);
            URI uri = new URL(normalizeForString(nodeUri)).toURI();
            HttpEntity<MultiValueMap<String, HttpEntity<?>>> requestEntity = constructRequest(newBiggestFile);
            log.info("finish to sync agent files, ip:{}, path:{}, fileName: {}", ip,
                newBiggestFile.getParent().toString(), newBiggestFile.getFileName().toString());
            ResponseEntity<Void> response = restTemplate.exchange(uri, HttpMethod.PUT, requestEntity, Void.class);
            log.info("finish to sync agent files, response:{}.", response.getStatusCode());
        } catch (Exception e) {
            log.error("Synchronous agent client failed, fail ip is: {}.", ip, ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
    }

    private static HttpEntity<MultiValueMap<String, HttpEntity<?>>> constructRequest(Path newBiggestFile) {
        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        // 使用 MultipartBodyBuilder 来构建 multipart 请求体
        MultipartBodyBuilder builder = new MultipartBodyBuilder();
        Resource fileResource = new FileSystemResource(newBiggestFile);
        // 此处cv的时候要注意 每个body都要指定正确的MediaType 否则接收端可能会解析不出来
        builder.part("agentClient", fileResource).contentType(MediaType.APPLICATION_OCTET_STREAM);
        UploadFileRequest request = new UploadFileRequest();
        request.setDetPath(newBiggestFile.getParent().toString());
        request.setFileName(newBiggestFile.getFileName().toString());
        builder.part("uploadRequest", request).contentType(MediaType.APPLICATION_JSON);
        // 创建 HttpEntity，包含请求头和请求体
        return new HttpEntity<>(builder.build(), headers);
    }

    // 根据endPointName获取端口号
    private int getPortByEndPointName(String endPointName) {
        switch (endPointName) {
            case Constants.PM_ENDPOINT_NAME:
                return ProtocolPortConstant.PM_INTERNAL_PORT;
            default:
                return ProtocolPortConstant.PM_INTERNAL_PORT;
        }
    }

    // 过滤不安全的特殊字符
    private String normalizeForString(String item) {
        if (VerifyUtil.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
    }
}
