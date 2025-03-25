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
package openbackup.system.base.common.utils.network;

import static openbackup.system.base.common.constants.CommonErrorCode.OPERATION_FAILED;

import feign.RetryableException;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.alarm.bo.LocalClusterInfo;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterStorageNodeVo;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeControllerInfo;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.SpringBeanUtils;

import org.apache.commons.collections.CollectionUtils;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicReference;

/**
 * 内部网络工具
 *
 */
@Slf4j
public class InternalNetUtil {
    /**
     * 缓存过期时间 10s
     */
    private static final long CACHE_EXPIRE_TIME = 10 * 1000;

    /**
     * 当前控制器信息
     * 使用AtomicReference缓存单一对象 保证线程安全
     */
    private static final AtomicReference<LocalClusterInfo> CURRENT_CONTROLLER = new AtomicReference<>();

    private InternalNetUtil() {
    }

    /**
     * 从缓冲获取本地控制信息并进行缓存
     * 若没有找到或者报错，则返回空对象，并cache空对象10s
     *
     * @return 本地控制器
     */
    public static LocalClusterInfo getCurrentNodeInfoAndCache() {
        LocalClusterInfo cachedController = CURRENT_CONTROLLER.get();
        if (cachedController != null && isCacheValid(cachedController.getCacheTimestamp())) {
            updateCacheTime();
            return cachedController;
        }
        LocalClusterInfo newController;
        try {
            newController = getCurrentNodeInfo();
            if (newController != null) {
                log.info("Success to get local current node info, ipv4: {}, ipv6: {}, node name: {}.",
                    newController.getIpv4(), newController.getIpv6(), newController.getName());
            } else {
                log.warn("Fetched controller info is null. Using default LocalClusterInfo.");
                newController = new LocalClusterInfo();
            }
        } catch (LegoCheckedException | RetryableException e) {
            log.error("Fail to get current node info", ExceptionUtil.getErrorMessage(e));
            newController = new LocalClusterInfo();
        }
        CURRENT_CONTROLLER.set(newController);
        updateCacheTime();
        return newController;
    }

    private static void updateCacheTime() {
        CURRENT_CONTROLLER.get().setCacheTimestamp(System.currentTimeMillis());
    }

    private static boolean isCacheValid(long cacheTimestamp) {
        return !VerifyUtil.isEmpty(cacheTimestamp) && (System.currentTimeMillis() - cacheTimestamp) < CACHE_EXPIRE_TIME;
    }

    /**
     * 获取当前节点信息
     *
     * @return 当前节点信息
     */
    public static LocalClusterInfo getCurrentNodeInfo() {
        // 查询本地集群所有节点信息
        List<ClusterStorageNodeVo> clusterStorageNodeVos = SpringBeanUtils.getBean(ClusterInternalApi.class)
            .queryLocalClusterNodes();
        if (CollectionUtils.isEmpty(clusterStorageNodeVos)) {
            throw new LegoCheckedException(OPERATION_FAILED, "Fail to get cluster storage node list.");
        }

        Optional<String> nodenameOpt = getCurrentControllerName();
        if (!nodenameOpt.isPresent()) {
            throw new LegoCheckedException(OPERATION_FAILED, "Fail to filter local controller name.");
        }

        return clusterStorageNodeVos.stream()
            .filter(clusterStorageNodeVo -> nodenameOpt.get().equals(clusterStorageNodeVo.getNodeName()))
            .map(InternalNetUtil::toLocalController)
            .findAny()
            .orElseThrow(() -> new LegoCheckedException(OPERATION_FAILED, "Fail to filter local controller info."));
    }

    private static Optional<String> getCurrentControllerName() {
        // 获取当前所有网卡IP地址
        List<InetAddress> currentIpList = getCurrentIpList();
        for (InetAddress inetAddress : currentIpList) {
            // 通过基础设施接口获取本地节点信息
            InfraResponseWithError<NodeControllerInfo> nodeControllerInfo = SpringBeanUtils.getBean(
                InfrastructureRestApi.class).getNodeInfoByPodIp(inetAddress.getHostAddress());

            String nodename = Optional.ofNullable(nodeControllerInfo)
                .map(InfraResponseWithError::getData)
                .map(NodeControllerInfo::getControl)
                .orElse(null);

            if (nodename == null) {
                continue;
            }

            return Optional.of(nodename);
        }
        return Optional.empty();
    }

    private static LocalClusterInfo toLocalController(ClusterStorageNodeVo clusterStorageNodeVo) {
        LocalClusterInfo localController = new LocalClusterInfo();
        localController.setName(clusterStorageNodeVo.getNodeName());
        localController.setIpv4(clusterStorageNodeVo.getManagementIPv4());
        localController.setIpv6(clusterStorageNodeVo.getManagementIPv6());
        return localController;
    }

    @ExterAttack
    private static List<InetAddress> getCurrentIpList() {
        try {
            // 得到当前机器上在局域网内所有的网络接口
            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
            return getAllLocalIpList(networkInterfaces);
        } catch (SocketException e) {
            throw new LegoCheckedException(OPERATION_FAILED, "Fail to get local pod IP address.", e);
        }
    }

    private static List<InetAddress> getAllLocalIpList(Enumeration<NetworkInterface> networkInterfaces) {
        List<InetAddress> inetAddressList = new ArrayList<>();
        // 遍历所有的网络接口
        while (networkInterfaces.hasMoreElements()) {
            NetworkInterface ni = networkInterfaces.nextElement();

            // 获取当前接口下绑定到该网卡的所有的 IP地址。
            Enumeration<InetAddress> nias = ni.getInetAddresses();
            while (nias.hasMoreElements()) {
                InetAddress ia = nias.nextElement();

                // 只需要ipv4地址 排除ipv6地址和127.0.0.1 取ipv4地址
                if (!ia.isLinkLocalAddress() && !ia.isLoopbackAddress()) {
                    inetAddressList.add(ia);
                }
            }
        }
        return inetAddressList;
    }
}
