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
