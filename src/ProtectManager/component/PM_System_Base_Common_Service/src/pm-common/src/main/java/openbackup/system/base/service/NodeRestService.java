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

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequest;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.util.Base64Util;
import openbackup.system.base.util.ClusterFileUtils;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

/**
 * k8s集群节点间通信服务
 *
 */
@Service
@Slf4j
public class NodeRestService {
    private static final long WAIT_TIME = 3 * 1000L;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    @Qualifier("defaultNodeRestApi")
    private NodeRestApi defaultNodeRestApi;

    @Autowired
    @Qualifier("formDataNodeRestApi")
    private NodeRestApi formDataNodeRestApi;

    /**
     * 同步multipartFile到endPointName指定的所有节点
     *
     * @param multipartFile filePath对应的文件的multipartFile
     * @param filePath filePath 带文件名的文件路径
     * @param endPointName endPointName
     */
    public void syncAlarmDumpFile(MultipartFile multipartFile, String filePath, String endPointName) {
        List<String> ipList = getIpListByEndPointName(endPointName);
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(endPointName);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                String encryptToBase64 = Base64Util.encryptToBase64(filePath);
                formDataNodeRestApi.syncAlarmDumpFile(uri, multipartFile, encryptToBase64);
            } catch (URISyntaxException | MalformedURLException e) {
                log.error("Build uri failed.", ExceptionUtil.getErrorMessage(e));
            } catch (FeignException exception) {
                log.error("Sync alarm dump file failed.", ExceptionUtil.getErrorMessage(exception));
            }
        }
    }

    /**
     * createTargetCluster
     *
     * @param request request
     * @return id
     */
    public int createTargetCluster(TargetClusterRequest request) {
        List<String> ipList = getIpListByEndPointName(Constants.PM_ENDPOINT_NAME);
        if (VerifyUtil.isEmpty(ipList)) {
            log.error("Get ip list failed.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Get ip list failed.");
        }

        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(
                    Constants.PM_ENDPOINT_NAME);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                return defaultNodeRestApi.createTargetCluster(uri, request);
            } catch (LegoCheckedException e) {
                log.error("Process try one node request fail, fail ip is: {}.", ip, ExceptionUtil.getErrorMessage(e));
                if (e.getErrorCode() != CommonErrorCode.NETWORK_CONNECTION_TIMEOUT) {
                    // 只有网络异常才会尝试其他ip
                    throw e;
                }
            } catch (FeignException e) {
                log.error("Process try one node request fail, fail ip is: {}.", ip, ExceptionUtil.getErrorMessage(e));
            } catch (MalformedURLException | URISyntaxException e) {
                log.error("Process try one node request fail, fail ip is: {}.", ip, ExceptionUtil.getErrorMessage(e));
                throw LegoCheckedException.cast(e);
            }
        }
        throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Create target cluster failed.");
    }

    /**
     * 检查成员节点是否能访问到主节点的infra ip
     *
     * @param infraIp 主节点infra ip
     * @return 与主节点infra ip是否连通
     */
    public boolean checkConnectionToPrimaryInfraIp(String infraIp) {
        List<String> ipList = getIpListByEndPointName(Constants.PM_ENDPOINT_NAME);
        if (VerifyUtil.isEmpty(ipList)) {
            log.error("Get ip list failed.");
            return false;
        }
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(
                    Constants.PM_ENDPOINT_NAME);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                boolean isConnected = defaultNodeRestApi.checkConnectionToPrimaryInfraIp(uri, infraIp);
                if (isConnected) {
                    return true;
                }
            } catch (Exception e) {
                log.error("Check connection to primary infraip failed, fail ip is: {}.", ip,
                    ExceptionUtil.getErrorMessage(e));
            }
        }
        return false;
    }

    /**
     * 集群转发请求通用方法
     *
     * @param message 消息内容
     * @param ip 待转发的节点 IP
     * @param apiMethod 转发调用的 API 方法
     * @param action 描述当前操作的日志
     */
    private void forwardAgentRequest(String message, String ip, Consumer<URI> apiMethod, String action) {
        try {
            String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(Constants.PM_ENDPOINT_NAME);
            log.info("Current uri for {} is: {}.", action, nodeUri);
            URI uri = new URL(normalizeForString(nodeUri)).toURI();
            apiMethod.accept(uri);
        } catch (Exception e) {
            log.error("{} failed, fail ip is: {}.", action, ip, ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 集群转发注册 agent 请求
     *
     * @param message 消息内容
     * @param ip 待转发的节点 IP
     */
    public void forwardRegisterHostAgent(String message, String ip) {
        forwardAgentRequest(message, ip, uri -> defaultNodeRestApi.registerHostAgent(uri, message),
            "forwardRegisterHostAgent");
    }

    /**
     * 集群转发更新 agent 请求
     *
     * @param message 消息内容
     * @param ip 待转发的节点 IP
     */
    public void forwardUpdateAgentClient(String message, String ip) {
        forwardAgentRequest(message, ip, uri -> defaultNodeRestApi.updateAgentClient(uri, message),
            "forwardUpdateAgentClient");
    }

    /**
     * 集群转发修改 agent 资源类型请求
     *
     * @param message 消息内容
     * @param ip 待转发的节点 IP
     */
    public void forwardUpdateAgentClientPluginType(String message, String ip) {
        forwardAgentRequest(message, ip, uri -> defaultNodeRestApi.updateAgentClientPluginType(uri, message),
            "forwardUpdateAgentClientPluginType");
    }


    /**
     * 获取每个node下能通agent的备份网络平面ip列表
     *
     * @param agentUrl agentUrl 包含agent信息的url
     * @param endPointName endPointName
     * @return dmeIps 能连通agent的备份ip合集
     */
    public List<String> getCanConnectedIps(String agentUrl, String endPointName) {
        List<String> ipList = getIpListByEndPointName(endPointName);
        ArrayList<String> dmeIps = new ArrayList<>();
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(endPointName);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                String connectedDmeIps = defaultNodeRestApi.getConnectedDmeIps(uri, agentUrl);
                if (!VerifyUtil.isEmpty(connectedDmeIps) && !connectedDmeIps.isEmpty()) {
                    dmeIps.add(connectedDmeIps);
                }
            } catch (URISyntaxException | MalformedURLException e) {
                log.error("Build uri failed.", ExceptionUtil.getErrorMessage(e));
            } catch (FeignException exception) {
                log.error("Sync alarm dump file failed.", ExceptionUtil.getErrorMessage(exception));
            }
        }
        log.info("IP address of the backup network plane that can be pinged: {}", dmeIps);
        return dmeIps;
    }

    /**
     * 根据endPointName获取ip列表
     *
     * @param endPointName endPointName
     * @return ip列表
     */
    private List<String> getIpListByEndPointName(String endPointName) {
        try {
            InfraResponseWithError<List<String>> endpointsResponse =
                infrastructureRestApi.getEndpoints(endPointName);
            List<String> endpoints = endpointsResponse.getData();
            if (endpoints.isEmpty()) {
                log.warn("Get all node endpoint failed.");
            }
            return endpoints;
        } catch (LegoCheckedException | FeignException e) {
            log.error("Get all node endpoint failed", ExceptionUtil.getErrorMessage(e));
            return new ArrayList<>();
        }
    }

    // 过滤不安全的特殊字符
    private String normalizeForString(String item) {
        if (VerifyUtil.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
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

    private void syncFileHelper(MultipartFile multipartFile, String filePath,
        String endPointName) {
        List<String> ipList = getIpListByEndPointName(endPointName);
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(endPointName);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                String encryptToBase64 = Base64Util.encryptToBase64(filePath);
                formDataNodeRestApi.syncFile(uri, multipartFile, encryptToBase64);
            } catch (URISyntaxException | MalformedURLException e) {
                log.error("Build uri failed.", ExceptionUtil.getErrorMessage(e));
            } catch (FeignException exception) {
                log.error("Sync file failed.", ExceptionUtil.getErrorMessage(exception));
            }
        }
    }

    private void deleteFileHelper(String filePath, String endPointName) {
        List<String> ipList = getIpListByEndPointName(endPointName);
        for (String ip : ipList) {
            try {
                String nodeUri = Constants.HTTP_URL_SCHEME + ip + ":" + getPortByEndPointName(endPointName);
                log.info("Current uri is: {}.", nodeUri);
                URI uri = new URL(normalizeForString(nodeUri)).toURI();
                String encryptToBase64 = Base64Util.encryptToBase64(filePath);
                defaultNodeRestApi.deleteFile(uri, encryptToBase64);
            } catch (URISyntaxException | MalformedURLException e) {
                log.error("Build uri failed.", ExceptionUtil.getErrorMessage(e));
            } catch (FeignException exception) {
                log.error("delete file failed.", ExceptionUtil.getErrorMessage(exception));
            }
        }
    }

    /**
     * 将文件同步到集群中的所有节点
     *
     * @param filePath 文件路径
     */
    public void syncFile(String filePath) {
        try {
            MultipartFile multipartFile = ClusterFileUtils.createMultipartFile(filePath);
            syncFileHelper(multipartFile, filePath, Constants.PM_ENDPOINT_NAME);
        } catch (LegoCheckedException exception) {
            log.error("Sync file failed.", ExceptionUtil.getErrorMessage(exception));
        }
    }

    /**
     * 删除集群中所有节点的文件
     *
     * @param filePath 文件路径
     */
    public void deleteFile(String filePath) {
        deleteFileHelper(filePath, Constants.PM_ENDPOINT_NAME);
    }
}
