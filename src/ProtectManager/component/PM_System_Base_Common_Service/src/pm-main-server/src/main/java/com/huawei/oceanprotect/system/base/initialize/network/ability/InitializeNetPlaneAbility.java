/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.ability;

import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitNetworkResultCode;
import com.huawei.oceanprotect.system.base.initialize.network.util.DeviceManagerResponseUtils;
import com.huawei.oceanprotect.system.base.initialize.network.util.UnrepeatedUtils;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.ConApplicationDetailRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.NetPlaneRangeRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerDynamicConfigInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlaneRange;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.AssociateObjType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.Type;

import feign.RetryableException;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.AddressFamily;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.commons.lang3.StringUtils;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import java.net.ConnectException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 平面网络动作
 *
 * @author swx1010572
 * @since 2021-01-18
 */
@Slf4j
@Service
public class InitializeNetPlaneAbility implements InitializeNetPlane {
    private static final String BACKUP_NET_PLANE = "backupNetPlane";

    private static final String ARCHIVE_NET_PLANE = "archiveNetPlane";

    private static final String GLOBAL_REPLICAS = "global.replicas";

    private static final long WAIT_PODS = 10L * 1000L;

    /**
     * 执行动作
     *
     * @param service dm 对象
     * @param netPlaneAccessPortList 请求参数
     * @param netPlaneName 网络平面关联枚举类
     * @return 结束动作
     */
    @Override
    public InitNetworkResult doAction(DeviceManagerService service, List<String> netPlaneAccessPortList,
        String netPlaneName) {
        log.debug("Enter InitializeNetPlaneAbility.doAction(service,action)");
        return createNetPlaneAccess(service, netPlaneAccessPortList, netPlaneName);
    }

    /**
     * 执行动作
     *
     * @param service dm 对象
     * @param netPlaneAccessPortList 请求参数
     * @param netPlaneName 网络平面关联枚举类
     * @return 结束动作
     */
    private InitNetworkResult createNetPlaneAccess(DeviceManagerService service, List<String> netPlaneAccessPortList,
        String netPlaneName) {
        log.info("netPlaneAccessPortList: {}", netPlaneAccessPortList);
        DeviceManagerResponse<List<NetPlane>> netPlaneRes = service.getNetPlanes();
        if (!netPlaneRes.getError().isSuccess()) {
            log.error("get net Plane failure");
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "get net Plane failure");
        }

        // 区分当前的netPlane 属于备份 或者 归档 网络主机 存入action参数 并返回当前 使用的平面网络
        NetPlane netPlane = DeviceManagerResponseUtils.pourNetPlaneList(netPlaneName, netPlaneRes.getData());

        /* 目前可能有问题,暂时注释掉 */
        DeviceManagerResponse<List<EthPort>> ethAssociate = getNetPlaneAssociatePort(service, netPlane.getId());
        if (!ethAssociate.getError().isSuccess()) {
            log.error("get eth_port Associate failure");
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "get eth_port Associate failure");
        }
        Set<String> ethSet = new HashSet<>();
        if (!CollectionUtils.isEmpty(ethAssociate.getData())) {
            for (EthPort ethPort : ethAssociate.getData()) {
                ethSet.add(ethPort.getId());
            }
            log.info("already add ethPort size: {} ethSet: {}", ethAssociate.getData().size(), ethSet);
        }
        for (String associateId : netPlaneAccessPortList) {
            if (ethSet.contains(associateId)) {
                continue;
            }
            if (!service.addPlaneAssociate(Long.parseLong(netPlane.getId()), associateId).getError().isSuccess()) {
                log.error("associate A1 port failure");
                return new InitNetworkResult(InitNetworkResultCode.FAILURE, "associate A1port failure");
            }
        }
        return new InitNetworkResult(InitNetworkResultCode.SUCCESS, "associate net_plane success");
    }

    /**
     * 获取需要的控制器对象列表
     *
     * @param service dm 对象
     * @param controllerSize 需要的控制个数
     * @return 控制器对象列表
     */
    @Override
    public List<Controller> getController(DeviceManagerService service, int controllerSize) {
        List<String> controllerSizeList = UnrepeatedUtils.getControllerSize(controllerSize);
        if (controllerSizeList.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "controllerSizeList is no exist");
        }

        List<Controller> controllers = service.getControllers().getData();
        return controllers.stream().filter(controller ->
            controllerSizeList.contains(controller.getParentId())).collect(Collectors.toList());
    }

    /**
     * 获取 扩容控制器 ID集合;
     *
     * @param service dm 对象
     * @param isExpansion 是否扩容
     * @param netPlaneId 平面网络ID
     * @param controllerSize 传递的实际需要控制器数量
     * @return 扩容控制器的ID集合
     */
    @Override
    public Set<String> getExpansionControllerId(DeviceManagerService service, boolean isExpansion, String netPlaneId,
        int controllerSize) {
        List<Controller> data = getController(service, controllerSize);
        List<EthPort> ethPortList = getNetPlaneAssociatePort(service, netPlaneId).getData();
        return data.stream()
            .map(Controller::getId)
            .filter(controllerId -> StringUtils.isNotEmpty(controllerId) && isControlEthPortNeedExpansion(ethPortList,
                controllerId))
            .collect(Collectors.toSet());
    }

    /**
     * 获取对应类型的所有端口
     *
     * @param service dm 对象
     * @param logicType 端口角色类型
     * @return 端口角色类型的端口集合
     */
    @Override
    public List<EthPort> getEthPortList(DeviceManagerService service, LogicType logicType) {
        return service.getEthPorts()
            .getData()
            .stream()
            .filter(ethPort -> logicType.equals(ethPort.getLogicType()))
            .collect(Collectors.toList());
    }

    private boolean isControlEthPortNeedExpansion(List<EthPort> ethPortList, String controllerId) {
        // 如果 传入的端口数量为0 ,我们认为传入的控制器需要扩展
        if (ObjectUtils.isEmpty(ethPortList)) {
            return true;
        }
        return ethPortList.stream().noneMatch(ethPort -> controllerId.equals(ethPort.getDefWorkNode()));
    }

    /**
     * 获取集合端口是否存在控制器的前端卡 LocationName;
     *
     * @param service dm 对象
     * @param collect 过滤前的端口集合
     * @param controllerId 控制器的location
     * @return LocationName
     */
    @Override
    public String getEthPortOwnIngLocation(DeviceManagerService service, List<EthPort> collect, String controllerId) {
        for (EthPort ethPort : collect) {
            if (controllerId.equals(ethPort.getDefWorkNode())) {
                return ethPort.getLocation().replace(ethPort.getName(), "");
            }
        }
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "no find exit [{" + controllerId + "}] ethPort");
    }

    /**
     * 获取当前容器pod的size
     *
     * @param service dm 对象
     * @return containerPodSize
     */
    @Override
    public int getContainerPodSize(DeviceManagerService service) {
        ConApplicationDetailRest apiRest = service.getApiRest(ConApplicationDetailRest.class);
        ContainerInfo containerInfo = getContainerInfo(service);
        ContainerInfo conApplication = apiRest.getConApplication(containerInfo.getName(), containerInfo.getNamespace());
        Optional<String> podSize = Optional.of(conApplication.getDynamicConfigList()
            .stream()
            .filter(containerDynamicConfigInfo -> GLOBAL_REPLICAS.equals(containerDynamicConfigInfo.getConfigName()))
            .map(ContainerDynamicConfigInfo::getConfigValue))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED))
            .findFirst();
        if (!podSize.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
        return Integer.parseInt(podSize.get());
    }

    /**
     * 根据平面网络名称列表返回平面网络ID字符串
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称列表
     * @return 平面网络ID字符串
     */
    @Override
    public String getNetPlaneIdInfos(DeviceManagerService service, List<String> netPlaneName) {
        return StringUtils.join(service.getNetPlanes()
            .getData()
            .stream()
            .filter(netPlane -> netPlaneName.contains(netPlane.getName()))
            .map(NetPlane::getId)
            .collect(Collectors.toList()), ";");
    }

    private ContainerInfo getContainerInfo(DeviceManagerService service) {
        Optional<ContainerInfo> containerInfo = Optional.ofNullable(service.getConApplication().getData())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED))
            .stream()
            .findFirst();
        return containerInfo.orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED));
    }

    private DeviceManagerResponse<List<EthPort>> getNetPlaneAssociatePort(DeviceManagerService service,
        String netPlaneId) {
        return service.getEthPortsAssociate(AssociateObjType.NET_PLANE, netPlaneId);
    }

    /**
     * 执行 添加平面网段IP段
     *
     * @param service dm 对象
     * @param netPlaneIpRange 请求参数
     * @param networkType 网络类型
     * @param networkPlaneName 平面网络名字
     * @param ipType IP类型
     */
    @Override
    @Retryable(value = {
        SocketTimeoutException.class, ConnectException.class, RetryableException.class, UnknownHostException.class
    }, backoff = @Backoff(delay = WAIT_PODS))
    public void addIpRange(DeviceManagerService service, Map<String, String> netPlaneIpRange, String networkType,
        String ipType, String networkPlaneName) {
        log.debug("Enter InitializeNetPlaneAbility.addIpRange(session,action)");
        NetPlaneRangeRest netPlaneRangeRest = service.getApiRest(NetPlaneRangeRest.class);
        List<NetPlaneRange> netPlaneRangeList = netPlaneRangeRest.getPlaneIpRange(service.getDeviceId(),
            Type.NETWORK_PLANE, networkType);

        List<String> oldNetPlaneNameList = new ArrayList<>();
        // 删除不同的平面网络IP段
        netPlaneRangeList.stream().forEach(netPlaneRange -> {
            if (networkPlaneName.equals(netPlaneRange.getName())) {
                return;
            }

            String ipRange = netPlaneIpRange.get(netPlaneRange.getName());

            // 当IpRange不能get到的时候,删除该IpRange
            if (StringUtils.isEmpty(ipRange)) {
                netPlaneRangeRest.deletePlaneIpRange(service.getDeviceId(),
                    delRange(netPlaneRange.getName(), netPlaneRange.getParentName()));
                return;
            }

            // 当IpRange能够get到的时候 存在相同命令的IpRangeName,
            // 等下添加的时候不需要添加 再去判断实际的IpRange是否相同;
            if (ipRange.equals(getIpTypeRange(netPlaneRange, ipType))) {
                oldNetPlaneNameList.add(netPlaneRange.getName());
            } else {
                netPlaneRangeRest.deletePlaneIpRange(service.getDeviceId(),
                    delRange(netPlaneRange.getName(), netPlaneRange.getParentName()));
            }
        });
        // 添加新的平面网络IP段
        Set<String> netPlaneNameList = netPlaneIpRange.keySet();
        for (String netPlaneName : netPlaneNameList) {
            // 如果remove成功 说明该平面网络IP段已经存在,不需要添加;
            if (oldNetPlaneNameList.remove(netPlaneName)) {
                continue;
            }

            String ipRange = netPlaneIpRange.get(netPlaneName);
            netPlaneRangeRest.addPlaneIpRange(service.getDeviceId(),
                addRange(ipRange, netPlaneName, ipType, networkType));
        }
        log.info("end InitializeNetPlaneAbility.addIpRange(session,action)");
    }

    /**
     * 获取平面网络信息
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称
     * @return 平面网络信息
     */
    @Override
    public NetPlane getNetPlane(DeviceManagerService service, String netPlaneName) {
        List<NetPlane> netPlaneList = service.getNetPlanes().getData();
        return netPlaneList.stream()
            .filter(netPlane -> netPlaneName.equals(netPlane.getName()))
            .findFirst()
            .orElse(new NetPlane());
    }

    @Override
    public List<NetPlane> getAllNetPlane(DeviceManagerService service) {
        return service.getNetPlanes().getData();
    }

    /**
     * 获取平面网络IP段信息
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称
     * @return 平面网络IP段列表信息
     */
    @Override
    public List<NetPlaneRange> getIpRange(DeviceManagerService service, String netPlaneName) {
        NetPlaneRangeRest netPlaneRangeRest = service.getApiRest(NetPlaneRangeRest.class);
        return netPlaneRangeRest.getPlaneIpRange(service.getDeviceId(), Type.NETWORK_PLANE, netPlaneName);
    }

    private NetPlaneRange addRange(String ipRange, String netPlaneName, String ipType, String networkType) {
        NetPlaneRange netPlaneRange = new NetPlaneRange();
        if (InitConfigConstant.IPV4_TYPE_FLAG.equals(ipType)) {
            netPlaneRange.setAddressFamily(AddressFamily.IPV4);
            netPlaneRange.setIpv4Range(ipRange);
        } else {
            netPlaneRange.setAddressFamily(AddressFamily.IPV6);
            netPlaneRange.setIpv6Range(ipRange);
        }
        netPlaneRange.setName(netPlaneName);
        netPlaneRange.setParentType(Type.NETWORK_PLANE);
        netPlaneRange.setParentName(networkType);

        return netPlaneRange;
    }

    private NetPlaneRange delRange(String name, String parentName) {
        NetPlaneRange netPlaneRange = new NetPlaneRange();
        netPlaneRange.setName(name);
        netPlaneRange.setParentName(parentName);
        netPlaneRange.setParentType(Type.NETWORK_PLANE);
        return netPlaneRange;
    }

    private String getIpTypeRange(NetPlaneRange netPlaneRange, String ipType) {
        if (InitConfigConstant.IPV4_TYPE_FLAG.equals(ipType)) {
            return netPlaneRange.getIpv4Range();
        }
        return netPlaneRange.getIpv6Range();
    }
}
