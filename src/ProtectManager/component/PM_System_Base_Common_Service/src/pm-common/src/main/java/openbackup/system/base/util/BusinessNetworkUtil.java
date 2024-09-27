package openbackup.system.base.util;

import openbackup.system.base.common.constants.StatefulsetConstants;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.beans.IpAddressInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NetPlaneInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * The BusinessNetworkUtil
 *
 * @author swx1010572
 * @since 2022-03-07
 */
@Slf4j
public class BusinessNetworkUtil {
    /**
     * 逗号
     */
    public static final String COMMA = ",";

    /**
     * 备份平面网络名称；
     */
    private static final String BACKUP_NET_PLANE = "backupNetPlane";

    /**
     * 网络状态信息集合
     */
    private static final String NETWORKS_STATUS = "k8s.v1.cni.cncf.io/networks-status";

    /**
     * 查询PROTECTENGINE的正则表达式
     */
    private static final String PATTERN_PROTECTENGINE = "^protectengine-\\d$";

    private static final String IP_NAME_PREFIX = "default/";

    /**
     * 是否打乱ip集合标识
     */
    private static volatile Boolean isDisrupt = new Boolean(false);

    /**
     * 通过POD和网络平面名称获取ip信息
     *
     * @param pod pod信息
     * @param netPlaneName 网络平面名称
     * @return ip列表
     */
    public static String parseNetPlane(NodePodInfo pod, String netPlaneName) {
        List<NetPlaneInfo> engines = pod.getNetPlaneInfos()
            .stream()
            .filter(netPlane -> netPlane.getNetPlaneName().contains(netPlaneName))
            .collect(Collectors.toList());

        if (CollectionUtils.isEmpty(engines)) {
            log.debug("get net plane engine info empty,node name :{}, net plane name:{}", pod.getPodName(),
                netPlaneName);
            return StringUtils.EMPTY;
        }
        List<String> ipNames = engines.stream()
            .map(netPlaneInfo -> IP_NAME_PREFIX + netPlaneInfo.getIpAddress())
            .collect(Collectors.toList());
        List<NetPlaneInfo> netPlaneList = pod.getNetPlaneInfos()
            .stream()
            .filter((netPlane) -> NETWORKS_STATUS.equals(netPlane.getNetPlaneName()))
            .collect(Collectors.toList());
        List<IpAddressInfo> ipAddressInfoList = netPlaneList.stream()
            .flatMap((netPlane) -> JSONArray.fromObject(netPlane.getIpAddress()).toBean(IpAddressInfo.class).stream())
            .collect(Collectors.toList());
        List<String> ipList = ipAddressInfoList.stream()
            .filter((ipAddressInfo) -> ipNames.contains(ipAddressInfo.getName()))
            .flatMap((ipAddressInfo) -> ipAddressInfo.getIps().stream())
            .collect(Collectors.toList());
        return String.join(",", ipList);
    }

    /**
     * 获取pod备份网络平面IP
     *
     * @param infra Target cluster service
     * @return Optional<String>
     */
    public static Optional<String> getBackupIp(InfrastructureRestApi infra) {
        List<NodePodInfo> podInfos = infra.getCollectNetPlaneInfo(StatefulsetConstants.PROTECTENGINE).getData();
        return getBackupIpFromList(podInfos);
    }

    /**
     * 获取pod备份网络平面IP
     *
     * @param podInfos 网络平面信息列表
     * @return 备份网络平面IP
     */
    public static Optional<String> getBackupIpFromList(List<NodePodInfo> podInfos) {
        if (podInfos == null) {
            log.error("get pod info from infra fail.");
            return Optional.empty();
        }
        List<String> names = podInfos.stream().map(NodePodInfo::getPodName).collect(Collectors.toList());
        log.debug("get pod info from infra successful, info : {}.", String.join(";", names));
        // 获取podName包含 protectengine 的pod info，并过滤出其中的NetPlaneInfo集合
        List<NetPlaneInfo> netPlaneInfos = podInfos.stream()
            .filter(nodePodInfo -> nodePodInfo.getPodName().matches(PATTERN_PROTECTENGINE))
            .flatMap(nodePodInfo -> nodePodInfo.getNetPlaneInfos().stream())
            .collect(Collectors.toList());
        // 获取名为 k8s.v1.cni.cncf.io/networks-status 的NetPlaneInfo
        List<NetPlaneInfo> rightNetPlaneInfo = netPlaneInfos.stream()
            .filter(netPlaneInfo -> NETWORKS_STATUS.equals(netPlaneInfo.getNetPlaneName()))
            .collect(Collectors.toList());
        // 获取netPlaneName为backupNetPlane中ipAddress的id
        List<NetPlaneInfo> planeInfos = netPlaneInfos.stream()
            .filter(netPlaneInfo -> netPlaneInfo.getNetPlaneName().contains(BACKUP_NET_PLANE))
            .collect(Collectors.toList());
        if (CollectionUtils.isEmpty(planeInfos) || planeInfos.size() < 1) {
            return Optional.empty();
        }
        // 获取的NetPlaneInfo中的IpAddressInfo
        List<IpAddressInfo> ipAddressInfos = rightNetPlaneInfo.stream()
            .flatMap(netPlaneInfo -> JSONArray.toCollection(JSONArray.fromObject(netPlaneInfo.getIpAddress()),
                IpAddressInfo.class).stream())
            .collect(Collectors.toList());
        if (CollectionUtils.isEmpty(ipAddressInfos)) {
            return Optional.empty();
        }
        // 获取名为 default/1 的ips 的第一个元素
        List<String> backupIps = new ArrayList<>();
        for (NetPlaneInfo netPlaneInfo : planeInfos) {
            String ipId = netPlaneInfo.getIpAddress();
            log.info("get ip id from infra, ip address :{}.", ipId);
            final String ipName = "default/" + ipId;
            backupIps.addAll(ipAddressInfos.stream()
                .filter(ipAddressInfo -> ipName.equals(ipAddressInfo.getName()))
                .map(ipAddressInfo -> ipAddressInfo.getIps().get(0))
                .collect(Collectors.toList()));
            // 打乱Ips地址
            log.info("disrupt state:{},before disrupt ips :{}", isDisrupt, String.join(";", backupIps));
            disruptIps(backupIps);
            log.info("disrupt state:{},after disrupt ips :{}", isDisrupt, String.join(";", backupIps));
        }
        backupIps = backupIps.stream().distinct().collect(Collectors.toList());
        return Optional.of(String.join(COMMA, backupIps));
    }

    private static void disruptIps(List<String> backupIps) {
        synchronized (isDisrupt) {
            if (isDisrupt) {
                Collections.reverse(backupIps);
            }
            isDisrupt = !isDisrupt;
        }
    }
}