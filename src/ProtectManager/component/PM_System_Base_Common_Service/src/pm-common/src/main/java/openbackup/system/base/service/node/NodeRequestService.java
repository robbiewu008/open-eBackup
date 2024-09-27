/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.service.node;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Optional;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * NodeRequestServiceImpl
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-24
 */

@RequiredArgsConstructor
@Slf4j
@Service
public class NodeRequestService {
    private final InfrastructureRestApi infrastructureRestApi;

    @Value("${NODE_NAME}")
    private String nodeName;

    /**
     * 请求发往指定节点
     *
     * @param request 请求方法
     * @param hostName pod host name
     * @param port 端口
     * @param <T> 返回值
     * @return 返回值
     */
    public <T> T requestToNode(Function<URI, T> request, String hostName, String port) {
        return request.apply(buildUri(findNodeIp(hostName), port));
    }

    /**
     * 请求发往当前节点
     *
     * @param request 请求方法
     * @param port 端口
     * @param <T> 返回值
     * @return 返回值
     */
    public <T> T requestToCurNode(Function<URI, T> request, String port) {
        return request.apply(buildUri(findNodeIp(nodeName), port));
    }

    /**
     * 请求发往指定节点
     *
     * @param request 请求方法
     * @param hostName pod host name
     * @param port 端口
     */
    public void requestToNodeNoRes(Consumer<URI> request, String hostName, String port) {
        request.accept(buildUri(findNodeIp(hostName), port));
    }

    private String findNodeIp(String hostName) {
        List<NodeDetail> allNodeInfo = getAllNodeInfo();
        Optional<String> nodeIp = allNodeInfo.stream()
            .filter(nodeDetail -> hostName.equals(nodeDetail.getHostName()))
            .map(NodeDetail::getAddress)
            .findAny();
        return nodeIp.orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Node not exist"));
    }

    private URI buildUri(String ip, String port) {
        String finalIp = Ipv6AddressUtil.isIpv6Address(ip) ? "[" + ip + "]" : ip;
        try {
            return new URI(String.format(Locale.ENGLISH, "https://%s:%s/", finalIp, port));
        } catch (URISyntaxException e) {
            throw new LegoUncheckedException(CommonErrorCode.SYSTEM_ERROR, "Failed to build target url.");
        }
    }

    private List<NodeDetail> getAllNodeInfo() {
        InfraResponseWithError<List<NodeDetail>> response = infrastructureRestApi.getInfraNodeInfo();
        if (VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getData())) {
            return Collections.emptyList();
        }
        return response.getData();
    }
}
