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
package com.huawei.oceanprotect.system.base.initialize.network.action;

import com.huawei.oceanprotect.system.base.dto.dorado.BondPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeStoragePool;
import com.huawei.oceanprotect.system.base.initialize.backstorage.beans.InitBackActionResult;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNfsService;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortRoute;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeUserServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitRouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.util.UnrepeatedUtils;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.config.DeviceManagerConfig;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfoEncoder;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.container.ContainerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.Type;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.StoragePoolRestApi;

import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.util.ObjectUtils;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * 处理DM交互的接口
 *
 */
@Data
@Slf4j
@Component
public class DeviceManagerHandler {
    /**
     * 单个控制器 备份（BACKUP）和归档(ARCHIVE) 需要的ip个数
     */
    private static final int IP_LOGIC_COUNT = InitConfigConstant.IP_LOGIC_COUNT;

    private static final int IP_VF_COUNT = InitConfigConstant.IP_VF_COUNT;

    private static final int SAFE_CONTAINER = 3;

    private static final long WAIT_PODS = 60L * 1000L;

    private static final long WAIT_CHECK_TIME = 2000L;

    private static final long DEFAULT_WAIT_TIME = 10L * 1000L;

    private static final String POD_STATUS = "Running";

    private static final String NO_IP_ERROR = "no enough ip";

    private static final String DATAENABLEENGINE_SERVER = "dataenableengine-server";

    private static final String PROTECTENGINE = "protectengine";

    private static final String BACKUP_NET_PLANE = InitConfigConstant.BACKUP_NET_PLANE;

    private static final String ARCHIVE_NET_PLANE = InitConfigConstant.ARCHIVE_NET_PLANE;

    private static final Pattern IPV4_PATTERN = Pattern.compile("(\\d+\\.){3}\\d+");

    private DeviceManagerService deviceManagerService;

    @Autowired
    private DeviceManagerServiceFactory deviceManagerServiceFactory;

    @Autowired
    private InitializeNetPlane initializeNetPlane;

    @Autowired
    private DeviceManagerConfig deviceManagerConfig;

    @Autowired
    private InitializePortRoute initializePortRoute;

    @Autowired
    private InitializePortService initializePortService;

    @Autowired
    private InitializeNfsService initializeNfsService;

    @Autowired
    private DeviceManagerInfoEncoder deviceManagerInfoEncoder;

    @Autowired
    private InitializeStoragePool initializeStoragePool;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private StoragePoolRestApi storagePoolRestApi;

    private InitializeUserServiceAbility initializeUserServiceAbility;

    @Autowired
    public void setInitializeUserServiceAbility(InitializeUserServiceAbility initializeUserServiceAbility) {
        this.initializeUserServiceAbility = initializeUserServiceAbility;
    }

    /**
     * 创建存储用户
     *
     * @return 获取创建之后密码
     */
    public String createStorageUser() {
        String pwd = "";
        try {
            pwd = initializeUserServiceAbility.createUserAndSetNeverExpire(deviceManagerService,
                UserUtils.getBusinessUsername());
            initializeUserServiceAbility.modifyUserLoginMethod(deviceManagerServiceFactory, deviceManagerInfoEncoder,
                deviceManagerConfig.getLink(), pwd);
            return pwd;
        } catch (DeviceManagerException e) {
            log.error("Create user or modify user login method failed");
            if (StringUtils.isEmpty(pwd)) {
                StringUtil.clean(pwd);
            }
            initializeUserServiceAbility.delUser(deviceManagerService, UserUtils.getBusinessUsername());
            throw e;
        }
    }

    /**
     * 更行NFSService4.1的信息
     */
    public void modifyNfsService() {
        log.info("start modify nfs service 4.1");
        try {
            initializeNfsService.modifyNfsService(deviceManagerService);
        } catch (DeviceManagerException e) {
            log.error("modify nfs 4.1 failed");
        }
        log.info("end modify nfs service 4.1");
    }

    /**
     * 添加空闲盘入存储池
     *
     * @param deviceId deviceId
     * @param username username
     */
    public void addFreeDiskToPool(String deviceId, String username) {
        List<StoragePool> storagePools = storagePoolRestApi.getStoragePools(deviceId, username).getData();
        for (StoragePool storagePool : storagePools) {
            initializeStoragePool.doAction(deviceId, username, storagePool, new InitBackActionResult());
        }
    }

    /**
     * 执行动作
     *
     * @param deviceId deviceId
     * @param username username
     * @param bondPortList 绑定端口列表
     */
    public void addBondPort(String deviceId, String username, List<BondPortDto> bondPortList) {
        initializePortService.addBondPort(deviceId, username, bondPortList);
    }

    /**
     * 获取平面网络ID字符串
     *
     * @param service dm 对象
     * @param netPlaneName 平面网络名称列表
     * @return 平面网络ID字符串
     */
    public String getNetPlaneIdInfos(DeviceManagerService service, List<String> netPlaneName) {
        return initializeNetPlane.getNetPlaneIdInfos(service, netPlaneName);
    }

    /**
     * 添加逻辑端口
     *
     * @param logicPortDtoList 逻辑端口列表
     */
    public void addLogicPort(LogicPortDto logicPortDtoList) {
        initializePortService.addLogicPort(logicPortDtoList);
    }

    /**
     * 获取DeviceManager的handler
     *
     * @param storageAuth 存储认证信息
     * @return deviceManagerService
     */
    public DeviceManagerService achiveDeviceManagerService(StorageAuth storageAuth) {
        // 采用环境中设置的用户名密码
        String link = deviceManagerConfig.getLink();

        // 原计划如果是本地鉴权，则采用本机5555端口免鉴权方案，否则采用填写的用户名密码
        // 但是免鉴权可能有问题
        // 免鉴权可能有问题：deviceManagerConfig.isLocal()?deviceManagerConfig.getUsername():storageAuth.getUsername();
        String user = storageAuth.getUsername();

        // 免鉴权可能有问题：deviceManagerConfig.isLocal()?deviceManagerConfig.getPassword():storageAuth.getPassword();
        char[] chars = storageAuth.getPassword().toCharArray();
        String thePassword = new String(chars);

        // 清理明文密码
        StringUtil.clean(chars);

        // DeviceManager信息
        DeviceManagerInfo deviceManagerInfo = new DeviceManagerInfo(link, user, thePassword, deviceManagerInfoEncoder);

        // 清理明文密码
        StringUtil.clean(thePassword);

        // 获取DeviceManager服务
        deviceManagerService = deviceManagerServiceFactory.getDeviceManagerService(deviceManagerInfo);
        return deviceManagerService;
    }

    /**
     * 创建所需的端口和加入对应前端口
     *
     * @param netPlaneName 需要的控制个数
     * @param port 加入端口名
     */
    public void addressAllocationInitNetPlaneFrontPort(String netPlaneName, List<String> port) {
        log.info("DeviceManagerHandler.addressAllocationInitNetPlaneFrontPort(): start");
        // 创建平面网络
        residualOperation(netPlaneName);
        // 绑定平面网络与前端卡
        initializeNetPlane.doAction(deviceManagerService, getNetPlaneAccessPortList(port), netPlaneName);
        log.info("DeviceManagerHandler.addressAllocationInitNetPlaneFrontPort(): end");
    }

    /**
     * 获取控制器对象信息列表
     *
     * @param controllerSize 需要的控制个数
     * @return 控制器对象信息列表
     */
    public List<Controller> getControllers(int controllerSize) {
        // 获取控制器
        return initializeNetPlane.getController(deviceManagerService, controllerSize);
    }

    /**
     * 获取容器pod的大小个数
     *
     * @return 容器pod的大小个数
     */
    public int getContainerPodSize() {
        return initializeNetPlane.getContainerPodSize(deviceManagerService);
    }

    private List<String> getNetPlaneAccessPortList(List<String> port) {
        // 获取所有的以太网口，以太网口的PARENTID是对应控制器的ID，槽位Location,CTE0.B5.P2代表B5槽位P2端口
        DeviceManagerResponse<List<EthPort>> ethPorts = deviceManagerService.getEthPorts();
        return ethPorts.getData()
            .stream()
            .filter(eth -> port.contains(eth.getLocation()))
            .map(EthPort::getId)
            .collect(Collectors.toList());
    }


    /**
     * 完成平面网络和容器
     *
     * @param replicas 启动容器的个数;
     * @param kataReplicas 安全容器的个数;
     * @param netPlaneName 平面网络名称和前端卡的map
     */
    public void accessNetPlaneAndPod(String replicas, String kataReplicas, Map<String, String> netPlaneName) {
        log.info("InitNetworkConfigThread.AccessNetPlaneAndPod: start");
        createNetPlaneAccess(replicas, kataReplicas, netPlaneName);
        log.info("InitNetworkConfigThread.AccessNetPlaneAndPod: end");
    }

    private void residualOperation(String netPlaneName) {
        // 查找备份网络，如果不存在则直接返回
        if (isExistNetPlane(netPlaneName)) {
            log.error("no find backup_plane");
            // 没有查询到对应的归档或者备份网络 ,不需要构造的场景,不会触发的错误码
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
    }

    /**
     * 关联平面网络和容器
     *
     * @param replicas 请求扩容参数
     * @param kataReplicas 请求扩容参数
     * @param netPlaneName 平面网络名称和前端卡的map
     */
    private void createNetPlaneAccess(String replicas, String kataReplicas, Map<String, String> netPlaneName) {
        DeviceManagerResponse<List<ContainerInfo>> conApplicationList = null;
        for (int index = 0; index < SAFE_CONTAINER; index++) {
            try {
                conApplicationList = deviceManagerService.getConApplication();
                break;
            } catch (DeviceManagerException e) {
                // 如果上面 抛出异常直接 返回 不用睡眠, 睡眠之后查询的时候完成. 这里去除 ？？
                log.error("The system is buys: ", e);
                try {
                    Thread.sleep(WAIT_CHECK_TIME);
                } catch (InterruptedException interruptedException) {
                    log.error("wait two time failed");
                }
            }
        }
        // 如果3次之后查询失败 报错系统错误,
        if (ObjectUtils.isEmpty(conApplicationList)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        }

        try {
            netPlaneAccess(conApplicationList.getData(), replicas, kataReplicas, netPlaneName);
        } catch (Exception e) {
            // 如果上面 抛出异常直接 返回 不用睡眠, 睡眠之后查询的时候完成. 这里去除 ？？
            log.error("Read timed out: ", e);
            throw new DeviceManagerException(CommonErrorCode.OPERATION_FAILED, "operation failed", "");
        }
    }

    private void netPlaneAccess(List<ContainerInfo> containerInfoList, String replicas, String kataReplicas,
        Map<String, String> netPlaneName) {
        if (StringUtils.isEmpty(containerInfoList)) {
            log.error("not find Access container");
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "operation failed");
        }
        List<String> setParameter = new ArrayList<>();
        for (ContainerInfo containerInfo : containerInfoList) {
            setParameter.add("atomic=true");
            setParameter.add("wait=true");
            setParameter.add("timeout=1800"); // 设置超时时间为30min
            String id = netPlaneName.get(InitConfigConstant.BACKUP_NET_PLANE);
            if (!StringUtils.isEmpty(id)) {
                setParameter.add("global.backupNetPlane=" + id);
            }
            if (!StringUtils.isEmpty(replicas)) {
                setParameter.add("global.replicas=" + replicas);
            }
            id = netPlaneName.get(InitConfigConstant.ARCHIVE_NET_PLANE);
            if (!StringUtils.isEmpty(id)) {
                setParameter.add("global.archiveNetPlane=" + id);
            }
            id = netPlaneName.get(InitConfigConstant.COPY_NET_PLANE);
            if (!StringUtils.isEmpty(id)) {
                setParameter.add("global.copyNetPlane=" + id);
            }
            if (!StringUtils.isEmpty(kataReplicas)) {
                setParameter.add("global.kataReplicas=" + kataReplicas);
            }
            containerInfo.setSetParameters(setParameter);
            if (!deviceManagerService.modifyConApplication(containerInfo).getError().isSuccess()) {
                log.error(
                    "Leave DeviceManagerHandlerAction.createNetPlaneAccess: false, modify conApplication failed");
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "operation failed");
            }
        }
    }

    private String getNetPlaneId(String netPlaneName) {
        return initializeNetPlane.getNetPlane(deviceManagerService, netPlaneName)
            .getId();
    }

    /**
     * 得到当前的控制器列表
     *
     * @return 控制器列表
     */
    public List<String> getControllerNames() {
        List<Controller> controllers = initializeNetPlane.getController(deviceManagerService,
            initializeNetPlane.getContainerPodSize(deviceManagerService));
        List<String> locationList = new ArrayList<>();
        for (Controller controller : controllers) {
            locationList.add(controller.getLocation());
        }
        return locationList;
    }

    private List<Ipv4RouteInfo> getRouteInfos(final String ipType, List<InitRouteInfo> routes) {
        List<Ipv4RouteInfo> routeInfos = new ArrayList<>();
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            List<Ipv6RouteInfo> collect = routes.stream()
                .map(route -> new Ipv6RouteInfo(route.getTargetAddress(), route.getSubNetMask(), route.getGateway()))
                .collect(Collectors.toList());
            for (Ipv6RouteInfo ipv6RouteInfo : UnrepeatedUtils.unrepeatedRoutingIpv6(collect)) {
                Ipv4RouteInfo ipv4RouteInfo = new Ipv4RouteInfo();
                ipv4RouteInfo.setTargetAddress(ipv6RouteInfo.getTargetAddress());
                ipv4RouteInfo.setGateway(ipv6RouteInfo.getGateway());
                ipv4RouteInfo.setSubNetMask(ipv6RouteInfo.getPrefixLength());
                routeInfos.add(ipv4RouteInfo);
            }
        } else {
            List<Ipv4RouteInfo> collect = routes.stream()
                .map(route -> new Ipv4RouteInfo(route.getTargetAddress(), route.getSubNetMask(), route.getGateway()))
                .collect(Collectors.toList());
            routeInfos.addAll(UnrepeatedUtils.unrepeatedRoutingIpv4(collect));
        }
        return routeInfos;
    }

    /**
     * 创建 平面网络
     *
     * @param netPlaneName 平面网络name
     * @return 执行最终消息
     */
    private boolean isExistNetPlane(String netPlaneName) {
        DeviceManagerResponse<List<NetPlane>> netPlanes = deviceManagerService.getNetPlanes();

        // 不存在平面网络的时候创建平面网络
        List<NetPlane> netPlaneList = netPlanes.getData();
        if (StringUtils.isEmpty(netPlaneList)) {
            return !createNetPlane(netPlaneName);
        }
        for (NetPlane netPlane : netPlaneList) {
            if (!netPlane.getName().equals(netPlaneName)) {
                continue;
            }
            return false;
        }
        return !createNetPlane(netPlaneName);
    }

    private boolean createNetPlane(String netPlaneName) {
        NetPlane netPlane = new NetPlane();
        netPlane.setName(netPlaneName);
        netPlane.setObjtype(Type.NETWORK_PLANE);
        netPlane.setFailover("1");
        return deviceManagerService.addNetPlane(netPlane).getError().isSuccess();
    }

    /**
     * 等待 dorado 数据更新时间
     */
    public void waitDatabaseUpdateTime() {
        try {
            Thread.sleep(WAIT_PODS);
        } catch (InterruptedException e) {
            log.error("wait five time failed");
        }
    }

    /**
     * 比较两个 ip 的大小 返回比较大的ip
     *
     * @param oldNetPlane oldNetPlane
     * @param newNetPlane newNetPlane
     * @return 比较结果
     */
    private boolean compareRange(String oldNetPlane, String newNetPlane) {
        String[] oldIp = oldNetPlane.split("\\.");
        String[] newIp = newNetPlane.split("\\.");
        for (int i = 0; i < oldIp.length; i++) {
            if (oldIp[i].equals(newIp[i])) {
                continue;
            }
            if (Integer.parseInt(oldIp[i]) < Integer.parseInt(newIp[i])) {
                return true;
            }
        }
        return false;
    }
}
