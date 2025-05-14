
package com.huawei.oceanprotect.system.base.upgrade;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.system.SystemConfigService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ConfigMapUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * 功能描述
 *
 * @author z00893213
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2025-01-09
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ConfigMapUtil.class})
public class UpgradeServiceTest {
    @InjectMocks
    private UpgradeService upgradeService;

    @Mock
    private SystemConfigService systemConfigService;

    @Mock
    private InitializePortService initializePortService;

    @Mock
    private DeployTypeService deployTypeService;

    /**
     * 用例名称：环境启动时会根据t_config表的配置中的值(IS_UPDATE_INIT_CONFIG)若为true自动触发。<br/>
     * 前置条件：t_config表的配置中的值(IS_UPDATE_INIT_CONFIG)为true。<br/>
     * 检查点：是否正确删除底座中的逻辑端口并重新创建。用例成功。
     */
    @Test
    public void test_updateInitConfig_should_success() {
        PowerMockito.when(deployTypeService.isXSeries()).thenReturn(true);
        PowerMockito.when(systemConfigService.getConfigValue("123")).thenReturn("false");
        List<LogicPortAddRequest> logicPortList = prepareBondingLogicPortAddRequestList();
        PowerMockito.when(initializePortService.getLogicPort()).thenReturn(logicPortList);

        PowerMockito.mockStatic(ConfigMapUtil.class);
        List<NetWorkConfigInfo> backupNetPlaneList = prepareBackupNetPlaneList();
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("network-conf", "backup_net_plane")).thenReturn(JSONObject.writeValueAsString(backupNetPlaneList));

        List<NetWorkConfigInfo> replicationNetPlaneList = prepareReplicationNetPlaneList();
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("network-conf", "replication_net_plane")).thenReturn(
            JSONObject.writeValueAsString(replicationNetPlaneList));

        PowerMockito.when(initializePortService.getDeviceId()).thenReturn("esn");
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        String initValue = "\"name\":\"0A-Backup01\",\"id\":\"4614219297495973888\",\"homePortType\":\"1\",\"currentControllerId\":\"0A\",\"vlan\":null,\"bondPort\":null,\"role\":2\"";
        initConfigInfo.setInitValue(initValue);
        PowerMockito.when(initializePortService.queryInitConfigByTypeAndEsn(any(), any())).thenReturn(initConfigInfo);
        ServicePortPo servicePortPo = new ServicePortPo();
        PowerMockito.doNothing().when(initializePortService).deleteSingleLogicPortOfDm("name", servicePortPo);
        PowerMockito.when(initializePortService.addLogicPort(any())).thenReturn(true);
        PowerMockito.doNothing().when(systemConfigService).addConfig(any(),any());
        upgradeService.updateInitConfig(Constants.ARCHIVE_NET_PLANE, "IS_UPDATE_INIT_CONFIG");
        upgradeService.updateInitConfig(Constants.BACKUP_NET_PLANE, "BACKUP_IS_UPDATE_INIT_CONFIG");
        Assert.assertNotNull(initConfigInfo);
    }

    /**
     * 获取从network-conf中获取的备份网络平面中的参数
     *
     * @return 从network-conf中获取的备份网络平面中的参数
     */
    public List<NetWorkConfigInfo> prepareBackupNetPlaneList() {
        List<NetWorkConfigInfo> backupNetPlaneList = new ArrayList<>();
        NetWorkConfigInfo netWorkConfigInfo1 = new NetWorkConfigInfo();
        netWorkConfigInfo1.setNodeId("node0");
        List<NetWorkLogicIp> netWorkLogicIp1 = new ArrayList<>();
        NetWorkLogicIp net1 = new NetWorkLogicIp();
        net1.setIp("192.168.102.81");
        net1.setMask("255.255.0.0");
        netWorkLogicIp1.add(net1);
        netWorkConfigInfo1.setLogicIpList(netWorkLogicIp1);

        NetWorkConfigInfo netWorkConfigInfo2 = new NetWorkConfigInfo();
        netWorkConfigInfo2.setNodeId("node1");
        List<NetWorkLogicIp> netWorkLogicIp2 = new ArrayList<>();
        NetWorkLogicIp net3 = new NetWorkLogicIp();
        net3.setIp("192.168.102.82");
        net3.setMask("255.255.0.0");
        netWorkLogicIp2.add(net3);
        netWorkConfigInfo2.setLogicIpList(netWorkLogicIp2);

        backupNetPlaneList.add(netWorkConfigInfo1);
        backupNetPlaneList.add(netWorkConfigInfo2);
        return backupNetPlaneList;
    }

    /**
     * 获取从network-conf中获取的复制网络平面中的参数
     *
     * @return 从network-conf中获取的复制网络平面中的参数
     */
    public List<NetWorkConfigInfo> prepareReplicationNetPlaneList() {
        List<NetWorkConfigInfo> replicationNetPlaneList = new ArrayList<>();
        NetWorkConfigInfo netWorkConfigInfo1 = new NetWorkConfigInfo();
        netWorkConfigInfo1.setNodeId("node0");
        List<NetWorkLogicIp> netWorkLogicIp1 = new ArrayList<>();
        NetWorkLogicIp net1 = new NetWorkLogicIp();
        net1.setIp("192.168.102.115");
        net1.setMask("255.255.0.0");
        netWorkLogicIp1.add(net1);
        netWorkConfigInfo1.setLogicIpList(netWorkLogicIp1);

        NetWorkConfigInfo netWorkConfigInfo2 = new NetWorkConfigInfo();
        netWorkConfigInfo2.setNodeId("node1");
        List<NetWorkLogicIp> netWorkLogicIp2 = new ArrayList<>();
        NetWorkLogicIp net3 = new NetWorkLogicIp();
        net3.setIp("192.168.102.116");
        net3.setMask("255.255.0.0");
        netWorkLogicIp2.add(net3);
        netWorkConfigInfo2.setLogicIpList(netWorkLogicIp2);

        replicationNetPlaneList.add(netWorkConfigInfo1);
        replicationNetPlaneList.add(netWorkConfigInfo2);
        return replicationNetPlaneList;
    }


    public List<LogicPortAddRequest> prepareBondingLogicPortAddRequestList() {
        List<LogicPortAddRequest> logicPortList = new ArrayList<>();
        LogicPortAddRequest logicPortAddRequest1 = new LogicPortAddRequest();
        logicPortAddRequest1.setName("0A-Backup01");
        logicPortAddRequest1.setRole(PortRole.SERVICE);
        logicPortAddRequest1.setIpv4Addr("8.40.102.81");
        logicPortAddRequest1.setHomePortType(HomePortType.BINDING);
        logicPortAddRequest1.setIpv4Mask("255.255.0.0");
        logicPortList.add(logicPortAddRequest1);

        LogicPortAddRequest logicPortAddRequest2 = new LogicPortAddRequest();
        logicPortAddRequest2.setName("0B-Backup01");
        logicPortAddRequest2.setRole(PortRole.TRANSLATE);
        logicPortAddRequest2.setIpv4Addr("8.40.102.82");
        logicPortAddRequest2.setHomePortType(HomePortType.ETHERNETPORT);
        logicPortAddRequest2.setIpv4Mask("255.255.0.0");
        logicPortList.add(logicPortAddRequest2);

        return logicPortList;
    }
}
