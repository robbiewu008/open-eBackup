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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.RequestForwardRetryConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.exception.NonForwardableException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;
import openbackup.system.base.redis.RedisSetService;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.service.ManageIpStartInitService;

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
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashSet;
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
    // PM SystemBase微服务监听的端口
    private static final int PM_BASE_PORT = 30081;

    private static final Set<Long> networkErrorCodes =
        ImmutableSet.of(CommonErrorCode.CONNECT_LDAP_SERVER_FAILED, CommonErrorCode.DOMAIN_NAME_RESOLVED_FAILED,
            CommonErrorCode.ALARM_SMTP_CONNECT_FAILED, CommonErrorCode.CONNECT_ADFS_SERVER_TIMEOUT,
            CommonErrorCode.ALARM_CONNECT_PROXY_FAILED, CommonErrorCode.SEND_EMAIL_DYNAMIC_PWD_FAILED);

    @Qualifier("nodeForwardRestTemplate")
    @Autowired
    private RestTemplate nodeForwardRestTemplate;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private ManageIpStartInitService manageIpStartInitService;

    @Autowired
    private RedisSetService redisSetService;

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
        if (networkErrorCodes.contains(ex.getErrorCode())) {
            // 如果当前的失败是网络引起的 则需要移除优先队列 其他错误如认证失败等 则不需要移除
            String currentIp = System.getenv("POD_IP");
            redisSetService.removeFromSet(Constants.HIGH_PRIORITY_NODE_CACHE_KEY, currentIp);
            log.warn("Request is failed on current node: {}, will remove from high priority node", currentIp);
        }
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
        Set<String> highPriorityIps = redisSetService.getAllFromSet(Constants.HIGH_PRIORITY_NODE_CACHE_KEY);
        Set<String> mangeIpNodes = redisSetService.getAllFromSet(Constants.NODE_WITH_MANAGE_IP_CACHE_KEY);
        Optional<String> result = Optional.empty();
        Set<String> priorityIps = new HashSet<>();
        priorityIps.addAll(highPriorityIps);
        priorityIps.addAll(mangeIpNodes);
        if (!VerifyUtil.isEmpty(priorityIps)) {
            result = forwardToNode(httpServletRequest, body, actionName, new ArrayList<>(priorityIps));
        }
        if (result.isPresent()) {
            return result;
        }
        log.debug("request {} retry on high priority nodes:{} failed, will continue on the rest nodes", actionName,
            priorityIps);
        // 如果高优先级队列和有管理ip的列表都没有成功 则继续尝试剩余的节点
        List<String> ipSet = getForwardIps(priorityIps);
        result = forwardToNode(httpServletRequest, body, actionName, ipSet);
        if (result.isPresent()) {
            return result;
        }
        log.debug("request {} retry on rest nodes:{} failed, will return fail", actionName, ipSet);
        return result;
    }

    private Optional<String> forwardToNode(HttpServletRequest httpServletRequest, Object body, String actionName,
        List<String> forwardIps) {
        for (String ip : forwardIps) {
            try {
                URI uri = buildUri(httpServletRequest.getRequestURI(), ip);
                ResponseEntity<String> responseEntity = forwardTryAllNodeRequestByIp(httpServletRequest, body, uri);
                if (responseEntity.getStatusCode() == HttpStatus.OK) {
                    log.debug("Request {} retry on node {} success.", actionName, ip);
                    // 调用成功 加入到优先级列表 供下次调用
                    manageIpStartInitService.addToPriorityList(Constants.HIGH_PRIORITY_NODE_CACHE_KEY, ip);
                    return Optional
                        .of(StringUtils.isNotEmpty(responseEntity.getBody()) ? responseEntity.getBody() : "SUCCESS");
                }
            } catch (LegoCheckedException ex) {
                log.warn("node :{} get network related exception,  will try on other node, ex:", ip,
                    ExceptionUtil.getErrorMessage(ex));
            } catch (Exception ex) {
                // 转发其它节点只是尝试看能否成功，如果发生未知异常忽略即可
                log.error("Request {} retry on node {} failed.", actionName, ip, ExceptionUtil.getErrorMessage(ex));
            }
        }
        return Optional.empty();
    }

    private ResponseEntity<String> forwardTryAllNodeRequestByIp(HttpServletRequest httpServletRequest, Object body,
        URI uri) {
        HttpHeaders httpHeaders = buildHeader(httpServletRequest);
        HttpEntity<Object> httpEntity = new HttpEntity<>(body, httpHeaders);
        return nodeForwardRestTemplate.exchange(uri,
            Objects.requireNonNull(HttpMethod.resolve(httpServletRequest.getMethod())), httpEntity, String.class);
    }

    private List<String> getForwardIps(Set<String> priorityIps) {
        List<String> ipList = infrastructureRestApi.getEndpoints(Constants.PM_ENDPOINT_NAME).getData();
        Set<String> ipSet = new HashSet<>();
        ipList.stream()
            .filter(element -> !priorityIps.contains(element)) // 筛选出不在 linkedList 中的元素
            .forEach(ipSet::add); // 添加到结果列表中
        return new ArrayList<>(ipSet);
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
        if (!networkErrorCodes.contains(ex.getErrorCode()) || !VerifyUtil.isEmpty(retryHeader)) {
            return false;
        }
        if ((ex.getCause() instanceof NonForwardableException)) {
            log.warn("current cause is marked as non forwardable , will not forward, cause:{}", ex.getCause());
            return false;
        }
        return true;
    }
}
