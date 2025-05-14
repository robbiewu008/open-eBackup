/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import com.huawei.oceanprotect.system.base.initialize.network.ability.InitDnsServerAbility;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitPortRouteAbility;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeNetPlaneAbility;
import com.huawei.oceanprotect.system.base.initialize.network.action.AddressAllocation;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkAction;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkResult;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.CfgInfo;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4Resource;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4RouteInfo;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitNetworkResultCode;
import com.huawei.oceanprotect.system.base.sdk.accesspoint.restapi.UpdateLogicApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.DeviceManagerServiceFactoryAbility;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.controller.Controller;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vstore.StoreTenant;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.InfrastructureRestApi;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.model.InfraResponseErrorInfo;
import com.huawei.oceanprotect.system.base.sdk.infrastructure.model.InfraResponseWithError;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.PostConstruct;

/**
 * 测试Ip分配情况
 *
 * @author swx1010572
 * @since 2021-01-11
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    DeviceManagerServiceFactory.class, DeviceManagerServiceFactoryAbility.class, InitializeCreate.class,
    InitCreateAbility.class, InitializeBondPort.class, InitBondPortAbility.class, InitializeLogicPort.class,
    InitializeLogicPortAbility.class, InitializeNetPlane.class, InitializeNetPlaneAbility.class,
    InitializePortRoute.class, InitPortRouteAbility.class, InitializeAnyBackupVip.class, InitAnyBackupVipAbility.class,
    InitializeDnsServer.class, InitDnsServerAbility.class, InitializeVlan.class, InitVlanAbility.class
})
public class DemoInitializeCreate {
    @Autowired
    private DeviceManagerServiceFactory deviceManagerServiceFactory;

    @Autowired
    InitializeCreate initializeCreate;

    @MockBean
    UpdateLogicApi updateLogicApi;

    @MockBean
    InfrastructureRestApi infrastructureRestApi;

    DeviceManagerService service;

    @PostConstruct
    void init() {
        InfraResponseWithError<Object> response = new InfraResponseWithError<>();
        InfraResponseErrorInfo infra = new InfraResponseErrorInfo();
        infra.setErrId("0");
        response.setError(infra);
        given(infrastructureRestApi.setCollectVipInfo(any())).willReturn(response);
    }

    /**
     * 获取会话
     */
    @Before
    public void setDeviceManagerService() {
        service = deviceManagerServiceFactory.getDeviceManagerService(
            new DeviceManagerInfo("https://51.6.135.227:8088/", "admin", "Admin@123"));
    }

    /**
     * 测试备份 create 初始化备份能力 --- 001;
     *
     * @throws Exception 异常
     */
    @Test
    public void testDoActionPass001() throws Exception {
        BackupNetworkConfig backup = createBackup();
        InitNetworkResult status = createPorts(backup);
        Assert.assertTrue(status.isOkay());
    }

    /**
     * 测试归档 create 初始化备份能力 --- 002;
     *
     * @throws Exception 异常
     */
    @Test
    public void testDoActionPass002() throws Exception {
        ArchiveNetworkConfig archive = createArchive();
        InitNetworkResult status = createFilePorts(archive);
        Assert.assertTrue(status.isOkay());
    }

    private BackupNetworkConfig createBackup() {
        BackupNetworkConfig backup = new BackupNetworkConfig();
        backup.setIpv4VlanId("522");

        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp("172.168.10.12");
        ipv4Resource.setEndIp("172.168.10.100");
        ipv4Resource.setMask("255.255.255.0");
        ipv4Resource.setGateway("172.168.10.1");
        backup.setIpv4cfg(ipv4Resource);

        List<Ipv4RouteInfo> ipv4RouteInfoList = new ArrayList<>();
        Ipv4RouteInfo ipv4RouteInfo = new Ipv4RouteInfo();
        ipv4RouteInfo.setTargetAddress("51.6.135.0");
        ipv4RouteInfo.setSubNetMask("255.255.255.0");
        ipv4RouteInfo.setGateway("172.168.10.1");
        ipv4RouteInfoList.add(ipv4RouteInfo);
        backup.setIpv4RouteCfg(ipv4RouteInfoList);

        return backup;
    }

    private ArchiveNetworkConfig createArchive() {
        ArchiveNetworkConfig archive = new ArchiveNetworkConfig();
        archive.setIpv4VlanId("522");
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp("172.168.10.12");
        ipv4Resource.setEndIp("172.168.10.100");
        ipv4Resource.setMask("255.255.255.0");
        ipv4Resource.setGateway("172.168.10.1");
        archive.setIpv4cfg(ipv4Resource);

        List<Ipv4RouteInfo> ipv4RouteInfoList = new ArrayList<>();
        Ipv4RouteInfo ipv4RouteInfo = new Ipv4RouteInfo();
        ipv4RouteInfo.setTargetAddress("51.6.135.0");
        ipv4RouteInfo.setSubNetMask("255.255.255.0");
        ipv4RouteInfo.setGateway("172.168.10.1");
        ipv4RouteInfoList.add(ipv4RouteInfo);
        archive.setIpv4RouteCfg(ipv4RouteInfoList);

        List<String> dnsServer = new ArrayList<>();
        dnsServer.add("51.6.135.226");
        dnsServer.add("51.6.135.227");
        dnsServer.add("51.6.135.228");
        archive.setIpv4DnsServers(dnsServer);
        return archive;
    }

    private InitNetworkResult createPorts(BackupNetworkConfig backupNetworkConfig) {
        DeviceManagerResponse<List<Controller>> controllers = service.getControllers();

        Ipv4Resource ipv4cfg = backupNetworkConfig.getIpv4cfg();
        AddressAllocation ipAddress = new AddressAllocation("IPV4", ipv4cfg.getStartIp(), ipv4cfg.getEndIp());

        // 获取所有的以太网口，以太网口的PARENTID是对应控制器的ID，槽位Location,CTE0.B5.P2代表B5槽位P2端口
        DeviceManagerResponse<List<EthPort>> ethPorts = service.getEthPorts();

        List<EthPort> ethPortsData = ethPorts.getData();
        Map<String, String> ethPortLocationMap = new HashMap<>();
        for (EthPort ethPort : ethPortsData) {
            ethPortLocationMap.put(ethPort.getLocation(), ethPort.getId());
        }
        CfgInfo cfg = new CfgInfo();
        cfg = injectionCfgInfo(cfg, backupNetworkConfig, null);
        DeviceManagerResponse<List<NetPlane>> netPlanes = service.getNetPlanes();
        if (isExistNetPlane(netPlanes, "backupNetPlane")) {
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "create archiveNetPlane netPlane failure");
        }
        Map<String, List<String>> ipLists = ipController(ipAddress, 4, controllers.getData().size());
        int size = controllers.getData().size();
        InitNetworkAction action = new InitNetworkAction();
        action = injectionInitNetWorkAction(action, cfg, ethPortLocationMap, true, true);
        size--;
        action.setIpList(ipLists.get(String.valueOf(size)));
        action.setController(controllers.getData().get(0));
        action.setNetPlaneRange("172.168.10.50-172.168.10.60");
        InitNetworkResult status = new InitNetworkResult();
        status.addInitBackActionResult(initializeCreate.doAction(service, action));
        return status;
    }

    private InitNetworkResult createFilePorts(ArchiveNetworkConfig archiveNetworkConfig) {
        DeviceManagerResponse<List<Controller>> controllers = service.getControllers();

        Ipv4Resource ipv4cfg = archiveNetworkConfig.getIpv4cfg();
        AddressAllocation ipAddress = new AddressAllocation("IPV4", ipv4cfg.getStartIp(), ipv4cfg.getEndIp());

        // 获取所有的以太网口，以太网口的PARENTID是对应控制器的ID，槽位Location,CTE0.B5.P2代表B5槽位P2端口
        DeviceManagerResponse<List<EthPort>> ethPorts = service.getEthPorts();
        CfgInfo cfg = new CfgInfo();
        cfg = injectionCfgInfo(cfg, null, archiveNetworkConfig);
        List<EthPort> ethPortsData = ethPorts.getData();
        Map<String, String> ethPortLocationMap = new HashMap<>();
        for (EthPort ethPort : ethPortsData) {
            ethPortLocationMap.put(ethPort.getLocation(), ethPort.getId());
        }
        Map<String, List<String>> ipLists = ipController(ipAddress, 3, controllers.getData().size());
        DeviceManagerResponse<List<NetPlane>> netPlanes = service.getNetPlanes();
        if (isExistNetPlane(netPlanes, "archiveNetPlane")) {
            return new InitNetworkResult(InitNetworkResultCode.FAILURE, "create archiveNetPlane netPlane failure");
        }
        int size = controllers.getData().size();
        InitNetworkAction action = new InitNetworkAction();
        action = injectionInitNetWorkAction(action, cfg, ethPortLocationMap, true, false);

        action.setIpList(ipLists.get(String.valueOf(size)));
        action.setController(controllers.getData().get(0));
        action.setNetPlaneRange("172.168.10.61-172.168.10.70");
        InitNetworkResult status = new InitNetworkResult();
        status.addInitBackActionResult(initializeCreate.doAction(service, action));
        return status;
    }

    private InitNetworkAction injectionInitNetWorkAction(InitNetworkAction action, CfgInfo cfg,
        Map<String, String> ethPortLocationMap, boolean isCreate, boolean isBackup) {
        action.setCfgInfo(cfg);
        action.setEthPortLocationMap(ethPortLocationMap);
        action.setStoreId(getStoreId());
        action.setCreate(isCreate);
        action.setBackup(isBackup);

        List<String> logicNameList = new ArrayList<>();
        logicNameList.add("_NFS_CIFS_Port");
        logicNameList.add("_iSCSI_Port");
        action.setLogicNameList(logicNameList);

        List<String> bondPortNameList = new ArrayList<>();
        bondPortNameList.add("_P0P1_Group");
        bondPortNameList.add("_P2P3_Group");
        action.setBondPortNameList(bondPortNameList);

        // 设置避免获取该对象 为null
        action.setLogicPortId(new ArrayList<>());

        action.setVlanId(false);
        return action;
    }

    /**
     * 创建 平面网络
     *
     * @param netPlanes 判断当前平面网络是否有 name
     * @param netPlaneName 平面网络name
     * @return 执行最终消息
     */
    private boolean isExistNetPlane(DeviceManagerResponse<List<NetPlane>> netPlanes, String netPlaneName) {
        for (NetPlane netPlane : netPlanes.getData()) {
            if (!netPlane.getName().equals(netPlaneName)) {
                continue;
            }
            return false;
        }
        return true;
    }

    /**
     * 注入可用参数
     *
     * @param cfg 返回请求参数
     * @param backupNetworkConfig 备份请求参数
     * @param archiveNetworkConfig 归档请求参数
     * @return cfgInfo
     */
    private CfgInfo injectionCfgInfo(CfgInfo cfg, BackupNetworkConfig backupNetworkConfig,
        ArchiveNetworkConfig archiveNetworkConfig) {
        if (backupNetworkConfig == null) {
            // cfg.setIpv4VlanId(archiveNetworkConfig.getIpv4VlanId());
            cfg.setIpv4cfg(archiveNetworkConfig.getIpv4cfg());
            cfg.setIpv4RouteCfg(archiveNetworkConfig.getIpv4RouteCfg());
            cfg.setIpv4DnsServers(archiveNetworkConfig.getIpv4DnsServers());
            return cfg;
        }
        if (archiveNetworkConfig == null) {
            cfg.setIpv4AnybackupVip(backupNetworkConfig.getIpv4AnybackupVip());
            // cfg.setIpv4VlanId(backupNetworkConfig.getIpv4VlanId());
            cfg.setIpv4RouteCfg(backupNetworkConfig.getIpv4RouteCfg());
            cfg.setIpv4cfg(backupNetworkConfig.getIpv4cfg());
            return cfg;
        }
        return cfg;
    }

    /**
     * 每个控制器获取的ip列表
     *
     * @param ipAddress 所有对象.
     * @param count 分配个数
     * @param size 分配份数
     * @return ip 列表
     */
    private Map<String, List<String>> ipController(AddressAllocation ipAddress, int count, int size) {
        List<String> ipCount = ipAddress.getAvailableIps(count * size);
        Map<String, List<String>> ipMap = new HashMap<>();
        for (int i = 0; i < size; i++) {
            List<String> ips = ipCount.subList(i, (i + 1) * count);
            ipMap.put(String.valueOf(i), ips);
        }
        return ipMap;
    }

    /**
     * 获取租户id
     *
     * @return 租户id
     */
    private String getStoreId() {
        DeviceManagerResponse<List<StoreTenant>> storeTenants = service.getStoreTenants();
        return storeTenants.getData().get(0).getId();
    }
}
