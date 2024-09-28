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
package openbackup.system.base.service.forward;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JwtTokenUtils;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.RouterServiceUtils;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Service;
import org.springframework.web.client.ResourceAccessException;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

import java.net.URI;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 转发请求到具体一控，失败后尝试其他控制器，直到所有控制器都失败或者至少有一个控制器成功
 *
 */
@Slf4j
@Service
public class TryAllNodeServiceImpl implements TryAllNodeService {
    private static final String HTTP_HEADER_TRY_ALL_NODE = "try_all_nodes";

    private static final long RETRY_DELAY_TIME = 3000L;

    private static final String PM_BASE_POD_NAME = "pm-system-base";

    @Qualifier("internalRestTemplate")
    @Autowired
    private RestTemplate internalRestTemplate;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    /**
     * 转发请求到具体一控，失败后尝试其他控制器，直到所有控制器都失败或者至少有一个控制器成功
     *
     * @param httpRequest http请求
     * @param httpServletResponse httpServletResponse
     * @param requestBody Object
     * @param isOverwriteBody boolean
     * @param forwardCache ForwardCache
     * @return responseEntity
     */
    @Override
    public ResponseEntity<Object> processTryAllNodeRequest(HttpServletRequest httpRequest,
        HttpServletResponse httpServletResponse, Object requestBody,
        boolean isOverwriteBody, ForwardCache forwardCache) {
        log.info("Process try one node request, uri is: {}.", httpRequest.getRequestURI());
        Object body = isOverwriteBody ? requestBody : RouterServiceUtils.getBodyData(httpRequest);
        List<String> ipList = null;
        try {
            ipList = infrastructureRestApi.getEndpoints(PM_BASE_POD_NAME).getData();
        } catch (FeignException | LegoCheckedException e) {
            log.error("Get base pod ip failed", ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
        Collections.shuffle(ipList);
        LegoCheckedException exception = null;
        for (int i = 0; i < ipList.size(); i++) {
            URI uri = RouterServiceUtils.buildNodeUri(httpRequest, ipList.get(i), 30081);
            ResponseEntity<Object> responseEntity = null;
            try {
                responseEntity = forwardTryAllNodeRequestByIp(httpRequest, body, uri);
            } catch (Exception e) {
                log.error("Process try one node request fail, fail ip is: {}.", ipList.get(i),
                    ExceptionUtil.getErrorMessage(e));
                exception = LegoCheckedException.cast(e);
                if (i == ipList.size() - 1) {
                    log.error("Process try all node request fail, all node fail.");
                    throw exception;
                }
                continue;
            }
            return responseEntity;
        }
        log.error("PM-Base pod is not exist.");
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
    }

    /**
     * 根据所给ip转发给备份成员节点
     *
     * @param httpServletRequest 请求参数
     * @param body 请求体
     * @param uri 集群uri
     * @return 转发请求的返回体，连接失败则返回null，由上层函数处理
     */
    @Retryable(exclude = {LegoCheckedException.class, ResourceAccessException.class},
        backoff = @Backoff(delay = RETRY_DELAY_TIME))
    public ResponseEntity<Object> forwardTryAllNodeRequestByIp(HttpServletRequest httpServletRequest,
        Object body, URI uri) {
        try {
            String token = JwtTokenUtils.parsingTokenFromRequest();
            return forwardTryAllNodeRequest(httpServletRequest, body, token, uri);
        } catch (RestClientException | LegoUncheckedException | LegoCheckedException exception) {
            if (exception instanceof LegoCheckedException) {
                throw exception;
            }
            if (exception instanceof RestClientException) {
                throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Network connection failed",
                    exception);
            }
            throw new LegoCheckedException("Invoke try all node rest api failed.", exception);
        }
    }

    @ExterAttack
    private ResponseEntity<Object> forwardTryAllNodeRequest(HttpServletRequest httpServletRequest, Object body,
        String token, URI uri) {
        log.info("Forward try all node request, uri is:{}.", uri.toString());
        HttpHeaders httpHeaders = RouterServiceUtils.buildClusterHeader(httpServletRequest, token);
        httpHeaders.add(HTTP_HEADER_TRY_ALL_NODE, HTTP_HEADER_TRY_ALL_NODE);
        return getObjectResponseEntity(httpServletRequest, body, uri, httpHeaders, internalRestTemplate);
    }

    private ResponseEntity<Object> getObjectResponseEntity(HttpServletRequest httpServletRequest, Object body, URI uri,
        HttpHeaders httpHeaders, RestTemplate restTemplate) {
        HttpEntity<Object> httpEntity = new HttpEntity<>(body, httpHeaders);
        try {
            ResponseEntity<Object> responseEntity = restTemplate.exchange(uri,
                Objects.requireNonNull(HttpMethod.resolve(httpServletRequest.getMethod())), httpEntity, Object.class);
            return ResponseEntity.status(responseEntity.getStatusCode()).body(responseEntity.getBody());
        } catch (LegoCheckedException e) {
            log.error("Connect to node by uri fail: {}.", uri, ExceptionUtil.getErrorMessage(e));
            throw e;
        }
    }
}
