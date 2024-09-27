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
package openbackup.data.access.framework.protection.service;

import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.when;

import com.huawei.oceanprotect.client.resource.manager.entity.AgentLanFree;
import com.huawei.oceanprotect.client.resource.manager.entity.SanClientConfig;
import com.huawei.oceanprotect.client.resource.manager.service.AgentLanFreeSanClientService;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.SanClientInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述 sanClient测试类
 *
 * @author n30046257
 * @version [DataBackup 1.3.0]
 * @since 2023-07-27
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({SanClientService.class})
public class SanClientServiceTest {
    /**
     * 测试场景：获取未配置sanClient的agents
     * 前置条件：代理主机配置sanClient
     * 检查点：返回的agents数量为0
     */
    @Test
    public void get_Not_Configured_sanClient_agents() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 创建测试数据
        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent1 = new Endpoint("1", "Agent1", 8080, "Linux");
        agents.add(agent1);

        // 设置mock方法的行为
        when(sanClientServiceMock.isBindedSanClient(agent1.getId())).thenReturn(true);

        // 调用被测试方法
        String[] result = sanClientService.getAgentsNotConfiguredSanclient(agents);

        // 验证结果
        Assert.assertEquals(0, result.length);
    }

    /**
     * 测试场景：填充代理的sanClient信息
     * 前置条件：代理主机配置sanClient
     * 检查点：返回的SanClients数量为1
     */
    @Test
    public void fill_agent_params() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 创建测试数据
        Endpoint agent = new Endpoint("1", "Agent1", 8080, "Linux");

        // 设置mock方法的行为
        when(sanClientServiceMock.getAgentLanFreeSanClientByResourceId("1"))
                .thenReturn(mockSanClientConfig());
        when(sanClientServiceMock.getAgentLanFreeByResourceId("resourceId1"))
                .thenReturn(mockAgentLanFree());
        when(resourceServiceMock.getResourceById("resourceId1"))
                .thenReturn(Optional.of(mockEnv()));

        // 调用被测试方法
        sanClientService.fillAgentParams(agent);

        // 验证结果
        Assert.assertArrayEquals(new String[]{"wwpn1", "wwpn2"}, agent.getWwpns());
        Assert.assertEquals(1, agent.getSanClients().size());
        Assert.assertEquals("resourceId1", agent.getSanClients().get(0).getId());
    }

    /**
     * 测试场景：通过环境的uuid找到对应的代理主机
     * 前置条件：环境正确配置属性
     * 检查点：返回的agent信息与环境的信息一致
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test
    public void find_agent_by_uuid() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 设置mock方法的行为
        ProtectedEnvironment protectedEnvironmentMock = PowerMockito.mock(ProtectedEnvironment.class);
        when(protectedEnvironmentMock.getUuid()).thenReturn("1");
        when(protectedEnvironmentMock.getEndpoint()).thenReturn("Agent1");
        when(protectedEnvironmentMock.getPort()).thenReturn(8080);
        when(protectedEnvironmentMock.getOsType()).thenReturn("Linux");
        when(protectedEnvironmentMock.getLinkStatus())
                .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());

        when(resourceServiceMock.getResourceById("1"))
                .thenReturn(Optional.of(protectedEnvironmentMock));

        // 调用被测试方法
        Endpoint result = Whitebox.invokeMethod(sanClientService, "findAgentByUuid", "1");

        // 验证结果
        Assert.assertNotNull(result);
        Assert.assertEquals("1", result.getId());
        Assert.assertEquals("Agent1", result.getIp());
        Assert.assertEquals(8080, result.getPort());
        Assert.assertEquals("Linux", result.getAgentOS());
    }

    /**
     * 测试场景：sanClientConfig为空抛出LegoCheckedException异常
     * 前置条件：sanClientConfig为空
     * 检查点：抛出LegoCheckedException异常
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_sanClientConfig_is_empty() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 设置mock方法的行为
        when(sanClientServiceMock.getAgentLanFreeSanClientByResourceId(anyString()))
                .thenReturn(null);

        Whitebox.invokeMethod(sanClientService, "getSanClientConfig", "1");
    }

    /**
     * 测试场景：备份任务下发sanClient备份任务，数据协议必须为FC
     * 前置条件：sanClientConfig的dataProtocol不是FC
     * 检查点：抛出LegoCheckedException异常
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_dataProtocol_not_FC() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        Whitebox.invokeMethod(sanClientService, "validateSanClientConfig", new SanClientConfig(),
                "1.1.1.1");
    }

    /**
     * 测试场景：sanClient为空抛出LegoCheckedException异常
     * 前置条件：sanClient为空
     * 检查点：抛出LegoCheckedException异常
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_sanClient_is_empty() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 设置mock方法的行为
        when(resourceServiceMock.getResourceById("resourceId1"))
                .thenReturn(Optional.of(mockEnv()));
        when(sanClientServiceMock.getAgentLanFreeByResourceId("1")).thenReturn(null);

        Whitebox.invokeMethod(sanClientService, "setSanClients", new Endpoint(), mockSanClientConfig());
    }

    /**
     * 测试场景：sanClientInfos为空抛出LegoCheckedException异常
     * 前置条件：根据资源uuid找不到在线的环境
     * 检查点：抛出LegoCheckedException异常
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_sanClientInfos_is_empty() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);

        // 设置mock方法的行为
        ProtectedEnvironment env = mockEnv();
        env.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        when(resourceServiceMock.getResourceById("resourceId1")).thenReturn(Optional.of(env));

        Whitebox.invokeMethod(sanClientService, "setSanClients", new Endpoint(), mockSanClientConfig());
    }

    /**
     * 测试场景：填充agent参数时不检查数据协议，协议为FC则填充agent的Wwpns字段。协议为ISCSI则填充agent的iqns字段
     * 前置条件：sanClientConfig的数据协议为FC或者ISCSI
     * 检查点：agent的Wwpns字段为wwpn1，agent的iqns字段为iqns
     *
     * @throws Exception 调用私有方法抛出异常
     */
    @Test
    public void fill_agent_params_not_check_data_protocol() throws Exception {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);


        // 设置mock方法的行为

        SanClientConfig sanClientConfig = mockSanClientConfig();
        when(sanClientServiceMock.getAgentLanFreeSanClientByResourceId(anyString()))
                .thenReturn(sanClientConfig);
        when(sanClientServiceMock.getAgentLanFreeByResourceId("resourceId1"))
                .thenReturn(mockAgentLanFree());
        when(resourceServiceMock.getResourceById("resourceId1"))
                .thenReturn(Optional.of(mockEnv()));
        Endpoint agent = new Endpoint();
        agent.setId("1");
        Whitebox.invokeMethod(sanClientService, "fillAgentParamsNotCheckDataProtocol", agent);
        Assert.assertEquals(agent.getWwpns()[0], "wwpn1");

        sanClientConfig.setClientIqns("[iqns]");
        sanClientConfig.setDataProtocol("ISCSI");
        EncryptorUtil encryptorUtil = PowerMockito.mock(EncryptorUtil.class);
        when(sanClientServiceMock.getAgentLanFreeSanClientByResourceId(anyString()))
                .thenReturn(sanClientConfig);
        when(encryptorUtil.getDecryptPwd(anyString())).thenReturn("iqns");
        sanClientService.setEncryptorUtil(encryptorUtil);
        Whitebox.invokeMethod(sanClientService, "fillAgentParamsNotCheckDataProtocol", agent);
        Assert.assertEquals(agent.getIqns().get(0), "iqns");
    }

    /**
     * 测试场景：恢复任务配置sanClient成功
     * 前置条件：待恢复的副本为SanClient副本
     * 检查点：恢复任务的高级参数的sanclientInvolved字段值为"true"
     */
    @Test
    public void restore_task_config_sanClient_success() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

        // 设置mock方法的行为
        when(copyManagerService.checkSanCopy(anyString())).thenReturn(Boolean.TRUE);
        when(sanClientServiceMock.isBindedSanClient(anyString())).thenReturn(true);
        SanClientConfig sanClientConfig = mockSanClientConfig();
        when(sanClientServiceMock.getAgentLanFreeSanClientByResourceId(anyString()))
                .thenReturn(sanClientConfig);
        when(sanClientServiceMock.getAgentLanFreeByResourceId("resourceId1"))
                .thenReturn(mockAgentLanFree());
        when(resourceServiceMock.getResourceById("resourceId1"))
                .thenReturn(Optional.of(mockEnv()));

        // 调用测试方法

        RestoreTask task = mockRestoreTask();
        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);
        sanClientService.configSanClient(task.getCopyId(), task.getAgents(), task.getAdvanceParams(),
        task.getDataLayout());

        // 校验测试点
        Assert.assertEquals(task.getAdvanceParams().get(SanClientService.IS_SANCLIENT), "true");
        Assert.assertEquals(task.getAdvanceParams().get(SanClientService.ADVANCE_PARAMS_KEY_MULTI_POST_JOB), "true");
        Assert.assertEquals(task.getDataLayout().getClientProtocolType(), new Integer(1));
    }

    /**
     * 测试场景：清理任务agent的iqns成功
     * 前置条件：待恢复的副本为SanClient副本
     * 检查点：agent中的iqns为空以及sanclient中iqns为空
     */
    @Test
    public void clean_iqns_of_agent_success() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

        // 设置mock方法的行为
        when(copyManagerService.checkSanCopy(anyString())).thenReturn(Boolean.TRUE);

        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent = new Endpoint();
        agent.setId("1");
        List<String> iqns = new ArrayList<>();
        iqns.add("9527");
        iqns.add("9527a");
        agent.setIqns(iqns);
        List<SanClientInfo> sanClientInfos = new ArrayList<>();
        SanClientInfo sanClientInfo = new SanClientInfo();
        sanClientInfo.setIqns(iqns);
        sanClientInfos.add(sanClientInfo);
        agent.setSanClients(sanClientInfos);
        agents.add(agent);

        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);
        sanClientService.cleanAgentIqns(agents, "9527");

        Assert.assertEquals(agents.get(0).getIqns().get(0), "\u0000\u0000\u0000\u0000");
        Assert.assertEquals(agents.get(0).getSanClients().get(0).getIqns().get(0), "\u0000\u0000\u0000\u0000");
        Assert.assertEquals(agents.get(0).getIqns().get(1), "\u0000\u0000\u0000\u0000\u0000");
        Assert.assertEquals(agents.get(0).getSanClients().get(0).getIqns().get(1), "\u0000\u0000\u0000\u0000\u0000");
    }


    /**
     * 测试场景：清空删除副本任务agent列表
     * 前置条件：待恢复的副本为SanClient副本
     * 检查点：agent列表为空
     */
    @Test
    public void config_copy_delete_agent_success() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

        // 设置mock方法的行为
        when(copyManagerService.checkSanCopy(anyString())).thenReturn(Boolean.TRUE);

        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent = new Endpoint();
        agent.setId("1");
        List<String> iqns = new ArrayList<>();
        iqns.add("9527");
        iqns.add("9527a");
        agent.setIqns(iqns);
        List<SanClientInfo> sanClientInfos = new ArrayList<>();
        SanClientInfo sanClientInfo = new SanClientInfo();
        sanClientInfo.setIqns(iqns);
        sanClientInfos.add(sanClientInfo);
        agent.setSanClients(sanClientInfos);
        agents.add(agent);

        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);
        sanClientService.configCopyDeleteAgent("9527", agents);
        Assert.assertTrue(VerifyUtil.isEmpty(agents));
    }

    /**
     * 测试场景：不处理副本删除任务的agent
     * 前置条件：待恢复的副本不为SanClient副本
     * 检查点：agent不变
     */
    @Test
    public void when_copy_is_not_SanClient_not_clear_agent() {
        // 创建mock对象
        AgentLanFreeSanClientService sanClientServiceMock = PowerMockito.mock(AgentLanFreeSanClientService.class);
        ResourceService resourceServiceMock = PowerMockito.mock(ResourceService.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);

        // 设置mock方法的行为
        when(copyManagerService.checkSanCopy(anyString())).thenReturn(Boolean.FALSE);

        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent = new Endpoint();
        agent.setId("1");
        List<String> iqns = new ArrayList<>();
        iqns.add("9527");
        iqns.add("9527a");
        agent.setIqns(iqns);
        List<SanClientInfo> sanClientInfos = new ArrayList<>();
        SanClientInfo sanClientInfo = new SanClientInfo();
        sanClientInfo.setIqns(iqns);
        sanClientInfos.add(sanClientInfo);
        agent.setSanClients(sanClientInfos);
        agents.add(agent);

        SanClientService sanClientService = new SanClientService(sanClientServiceMock, resourceServiceMock,
                copyManagerService);
        sanClientService.configCopyDeleteAgent("9527", agents);
        Assert.assertEquals(agent, agents.get(0));
    }


    private AgentLanFree mockAgentLanFree() {
        AgentLanFree agentLanFree = new AgentLanFree();
        agentLanFree.setWwpns("[{\"wwpn\":\"21000024ff2f4190\"},{\"wwpn\":\"21000024ff2f4190\"}]");
        return agentLanFree;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setPort(0);
        environment.setOsType("Linux");
        environment.setUuid("uuid");
        environment.setEndpoint("Endpoint");
        return environment;
    }

    private SanClientConfig mockSanClientConfig() {
        SanClientConfig sanClientConfig = new SanClientConfig();
        sanClientConfig.setDataProtocol("FC");
        sanClientConfig.setClientWwpns("[{\"wwpn\":\"wwpn1\"},{\"wwpn\":\"wwpn2\"}]");
        sanClientConfig.setSanclientResourceIds("[\"resourceId1\",\"resourceId2\"]");
        return sanClientConfig;
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = new RestoreTask();
        BaseDataLayout dataLayout = new BaseDataLayout();
        task.setDataLayout(dataLayout);
        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent = new Endpoint();
        agent.setId("1");
        agents.add(agent);
        task.setAgents(agents);
        task.setCopyId("1");
        task.setAdvanceParams(new HashMap<>());
        return task;
    }
}