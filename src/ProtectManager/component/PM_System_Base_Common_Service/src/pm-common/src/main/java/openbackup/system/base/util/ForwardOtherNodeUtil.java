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
package openbackup.system.base.util;

import com.google.common.collect.ImmutableSet;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.RequestForwardRetryConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;

import javax.servlet.http.HttpServletRequest;

/**
 * 转发其他节点的工具类
 *
 */
@Slf4j
@Component
public class ForwardOtherNodeUtil {
    private static final String PM_BASE_POD_NAME = "pm-system-base";

    // PM SystemBase微服务监听的端口
    private static final int PM_BASE_PORT = 30081;
    private static final Set<Long> networkErrorCodes = ImmutableSet.of(CommonErrorCode.CONNECT_LDAP_SERVER_FAILED,
        CommonErrorCode.ALARM_SMTP_CONNECT_FAILED, CommonErrorCode.CONNECT_ADFS_SERVER_TIMEOUT,
        CommonErrorCode.ALARM_CONNECT_PROXY_FAILED);

    @Qualifier("pmSystemBaseRestTemplate")
    @Autowired
    private RestTemplate pmMsRestTemplate;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    /**
     * 尝试转发给其他节点
     *
     * @param request request
     * @param ex 异常
     * @param body 请求信息
     * @param actionName 操作接口
     * @return 请求返回值
     */
    public String tryRequestToOtherNodes(HttpServletRequest request, LegoCheckedException ex, Object body,
        String actionName) {
        // 目前节点级的重试只适用于单控和测试主机网络不通，但无法确定是网卡故障或者网络链路故障，因此尝试将请求转发到其它节点做一次重试
        // 如果重试可以成功，则返回成功节点的请求，如果转发非200，则仍然返回本节点处理的请求（业务检查导致的失败，所有节点都应该一致）
        if (needForwardToOtherNodes(ex, request.getHeader(RequestForwardRetryConstant.HTTP_HEADER_INTERNAL_RETRY))) {
            Optional<String> resp = forwardToOtherNodes(request, body, actionName);
            if (resp.isPresent()) {
                return resp.get();
            }
        }
        throw ex;
    }

    private Optional<String> forwardToOtherNodes(HttpServletRequest httpServletRequest, Object body,
        String actionName) {
        List<String> forwardIps = getForwardIps();
        log.info("The {} request is failed on this node, need retry by other nodes {}", actionName, forwardIps);
        LegoCheckedException rethrowEx = null;
        for (String ip : forwardIps) {
            try {
                URI uri = buildUri(httpServletRequest.getRequestURI(), ip);
                ResponseEntity<String> responseEntity = forwardTryAllNodeRequestByIp(httpServletRequest, body, uri);
                if (responseEntity.getStatusCode() == HttpStatus.OK) {
                    log.info("Request {} retry on node {} success.", actionName, ip);
                    return Optional.of(
                        StringUtils.isNotEmpty(responseEntity.getBody()) ? responseEntity.getBody() : "SUCCESS");
                }

                log.info("Request {} retry on node {} failed, status code is {}.", actionName, ip,
                    responseEntity.getStatusCode().value());
            } catch (LegoCheckedException ex) {
                // 当转发其他节点获得的异常非网络失败的异常时，说明这个节点可以连通，应该复用对应业务错误码，否则可能导致覆盖，仍然报网络异常
                if (!networkErrorCodes.contains(ex.getErrorCode())) {
                    rethrowEx = ex;
                }
            } catch (Exception ex) {
                // 转发其它节点只是尝试看能否成功，如果发生未知异常忽略即可
                log.error("Request {} retry on node {} failed.", actionName, ip, ExceptionUtil.getErrorMessage(ex));
            }
        }
        if (rethrowEx != null) {
            throw rethrowEx;
        }

        return Optional.empty();
    }

    private ResponseEntity<String> forwardTryAllNodeRequestByIp(HttpServletRequest httpServletRequest, Object body,
        URI uri) {
        HttpHeaders httpHeaders = buildHeader(httpServletRequest);
        HttpEntity<Object> httpEntity = new HttpEntity<>(body, httpHeaders);
        return pmMsRestTemplate.exchange(uri,
            Objects.requireNonNull(HttpMethod.resolve(httpServletRequest.getMethod())), httpEntity, String.class);
    }

    private List<String> getForwardIps() {
        try {
            List<String> ipList = infrastructureRestApi.getEndpoints(PM_BASE_POD_NAME).getData();
            // 当前节点不再转发
            ipList.remove(System.getenv("POD_IP"));
            return ipList;
        } catch (FeignException | LegoCheckedException e) {
            log.error("Get base system pod ips failed", ExceptionUtil.getErrorMessage(e));
            return Collections.emptyList();
        }
    }

    private URI buildUri(String uri, String ip) {
        String finalIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
        String finalUrl = "https://" + finalIp + ":" + PM_BASE_PORT + uri;
        try {
            return new URL(finalUrl).toURI();
        } catch (URISyntaxException | MalformedURLException e) {
            throw new LegoUncheckedException(CommonErrorCode.SYSTEM_ERROR, "Failed to build target url.");
        }
    }

    private HttpHeaders buildHeader(HttpServletRequest request) {
        HttpHeaders headers = new HttpHeaders();
        Enumeration<String> headerNameEnumeration = request.getHeaderNames();
        while (headerNameEnumeration.hasMoreElements()) {
            String headerName = headerNameEnumeration.nextElement();
            Enumeration<String> headerValueEnumeration = request.getHeaders(headerName);
            while (headerValueEnumeration.hasMoreElements()) {
                headers.add(headerName, headerValueEnumeration.nextElement());
            }
        }

        // 这里加上该标记，避免处理重试的节点无终止的递归重试
        headers.add(RequestForwardRetryConstant.HTTP_HEADER_INTERNAL_RETRY,
            RequestForwardRetryConstant.HTTP_HEADER_INTERNAL_RETRY);
        return headers;
    }

    private boolean needForwardToOtherNodes(LegoCheckedException ex, String retryHeader) {
        // 只针对特定错误码以及原始请求（重试标识为空）进行精准的跨控制器转发重试。
        return networkErrorCodes.contains(ex.getErrorCode()) && VerifyUtil.isEmpty(retryHeader);
    }
}
