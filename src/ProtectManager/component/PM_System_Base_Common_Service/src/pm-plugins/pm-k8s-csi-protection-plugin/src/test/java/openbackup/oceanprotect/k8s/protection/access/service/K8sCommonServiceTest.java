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
package openbackup.oceanprotect.k8s.protection.access.service;

import static org.mockito.Mockito.spy;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.service.AgentBusinessService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.oceanprotect.k8s.protection.access.common.K8sQueryParam;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sExtendInfoKey;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NetworkServiceApi;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述: K8sCommonServiceTest
 *
 */
public class K8sCommonServiceTest {
    private final AgentBusinessService agentBusinessService = Mockito.mock(AgentBusinessService.class);
    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private final MemberClusterService memberClusterService = Mockito.mock(MemberClusterService.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final IVpcService iVpcService = Mockito.mock(IVpcService.class);
    private final NetworkServiceApi networkServiceApi = Mockito.mock(NetworkServiceApi.class);
    private final AvailableAgentManagementDomainService domainService = Mockito.mock(AvailableAgentManagementDomainService.class);
    private final NetworkService networkService = Mockito.mock(NetworkService.class);
    private final DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);
    private final ProtectedResourceMapper protectedResourceMapper = Mockito.mock(ProtectedResourceMapper.class);
    private final K8sCommonService commonService = new K8sCommonService(agentBusinessService, agentUnifiedService,
        memberClusterService, resourceService, iVpcService, networkServiceApi, domainService, environmentService, protectedResourceMapper, networkService, deployTypeService);

    /**
     * 用例场景：检查K8S集群连通性
     * 前置条件：无
     * 检查点: 连通性检查成功，无异常
     */
    @Test
    public void test_check_k8s_cluster_connectivity_success() {
        ProtectedEnvironment k8sCluster = new ProtectedEnvironment();
        List<ProtectedEnvironment> agents = Collections.singletonList(new ProtectedEnvironment());
        AgentBaseDto successResp = new AgentBaseDto();
        successResp.setErrorCode(K8sConstant.SUCCESS);
        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
        Mockito.when(agentUnifiedService.checkApplication(k8sCluster, agents.get(0))).thenReturn(successResp);
        spyCommonService.checkConnectivity(k8sCluster);
        Mockito.verify(agentUnifiedService, Mockito.times(1)).checkApplication(k8sCluster, agents.get(0));
    }

    /**
     * 用例场景：检查K8S集群连通性
     * 前置条件：无
     * 检查点: 无可用内置代理，抛出异常
     */
    @Test
    public void should_throw_exception_when_no_available_agent() {
        ProtectedEnvironment k8sCluster = new ProtectedEnvironment();
        List<ProtectedEnvironment> agents = Collections.emptyList();
        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> spyCommonService.checkConnectivity(k8sCluster));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }

    /**
     * 用例场景：检查K8S集群连通性
     * 前置条件：无
     * 检查点: agent返回失败，抛出异常
     */
    @Test
    public void should_throw_exception_when_agent_check_failed() {
        ProtectedEnvironment k8sCluster = new ProtectedEnvironment();
        List<ProtectedEnvironment> agents = Collections.singletonList(new ProtectedEnvironment());
        AgentBaseDto failedResp = new AgentBaseDto();
        failedResp.setErrorCode("-1");
        ActionResult result = new ActionResult();
        result.setBodyErr(String.valueOf(CommonErrorCode.KUBE_CONFIG_ERROR));
        failedResp.setErrorMessage(JsonUtil.json(result));
        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
        Mockito.when(agentUnifiedService.checkApplication(k8sCluster, agents.get(0))).thenReturn(failedResp);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> spyCommonService.checkConnectivity(k8sCluster));
        Assert.assertEquals(CommonErrorCode.KUBE_CONFIG_ERROR, exception.getErrorCode());
    }

    /**
     * 用例场景：查询agent资源
     * 前置条件：无
     * 检查点: 查询成功
     */
    @Test
    public void query_k8s_resource_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        mockCanFindConnectiveInternalAgent();

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        response.getRecords().add(new ProtectedResource());
        response.getRecords().add(new ProtectedResource());
        response.setTotalCount(20);

        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentUnifiedService.getDetailPageList(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
                .thenReturn(response);

        PageListResponse<ProtectedResource> res = spyCommonService.queryResource(0, 2, new K8sQueryParam(), k8sCluster);

        Assert.assertEquals(res.getTotalCount(), 20);
        Assert.assertEquals(res.getRecords().size(), 2);
    }

    /**
     * 用例场景：查询k8s集群信息
     * 前置条件：无
     * 检查点: 查询成功
     */
    @Test
    public void query_k8s_cluster_info_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        mockCanFindConnectiveInternalAgent();

        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(K8sConstant.CLUSTER_VERSION, "1.17.0");

        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentUnifiedService.getClusterInfo(Mockito.any(), Mockito.any())).thenReturn(appEnvResponse);

        AppEnvResponse res = spyCommonService.queryClusterInfo(k8sCluster);

        Assert.assertEquals(res.getExtendInfo().get(K8sConstant.CLUSTER_VERSION), "1.17.0");
    }

    /**
     * 用例场景：能查询到与集群连通的内置agent
     * 前置条件：无
     * 检查点: 查询成功
     */
    @Test
    public void query_connective_internal_agent_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("agent1");
        List<ProtectedEnvironment> agents = new ArrayList<>(Collections.singletonList(agent1));
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
        Mockito.when(memberClusterService.clusterEstablished()).thenReturn(true);
        List<ProtectedEnvironment> internalAgentList = commonService.getConnectiveInternalAgent(k8sCluster, true);
        Assert.assertEquals(1, internalAgentList.size());
        Assert.assertEquals(internalAgentList.get(0).getUuid(),agent1.getUuid());
    }

    /**
     * 用例场景：不能查询到与集群连通的内置agent
     * 前置条件：无
     * 检查点: 查询失败，抛出异常
     */
    @Test
    public void query_connective_internal_agent_failed_and_throw_exception() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("agent2");
        List<ProtectedEnvironment> agents = new ArrayList<>(Collections.singletonList(agent1));
        K8sCommonService spyCommonService = spy(commonService);
        Mockito.doNothing().when(spyCommonService).addIpRule(k8sCluster);
        Mockito.doNothing().when(spyCommonService).deleteIpRule(k8sCluster);
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
        Mockito.when(memberClusterService.clusterEstablished()).thenReturn(true);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
                () -> spyCommonService.getConnectiveInternalAgent(k8sCluster, true));
        Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
    }

    private ProtectedEnvironment mockK8sCluster() {
        ProtectedEnvironment k8sCluster = new ProtectedEnvironment();
        k8sCluster.setExtendInfoByKey(K8sExtendInfoKey.INTERNAL_AGENT_CONNECTION_PREFIX + "agent1",
                String.valueOf(true));
        k8sCluster.setExtendInfoByKey(K8sExtendInfoKey.INTERNAL_AGENT_CONNECTION_PREFIX + "agent2",
                String.valueOf(false));

        return k8sCluster;
    }

    private void mockCanFindConnectiveInternalAgent() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("agent1");
        List<ProtectedEnvironment> agents = new ArrayList<>(Collections.singletonList(agent1));
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);
    }

    /**
     * 用例场景：查出所有的备份网络，将其添加到备份agent的高级参数中
     * 前置条件：无
     * 检查点: 添加成功
     */
    @Test
    public void fill_backup_agent_connected_ips_success() {
        Endpoint endpoint = new Endpoint();
        endpoint.setAdvanceParams(new HashMap<>());
        List<Endpoint> restoreEndPoints = Arrays.asList(endpoint);
        DeviceNetworkInfo deviceNetworkInfo = new DeviceNetworkInfo();
        deviceNetworkInfo.setBackupConfig(new ArrayList<>());

        Mockito.when(networkService.getDeviceNetworkInfo()).thenReturn(deviceNetworkInfo);
        Mockito.when(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig())).thenReturn(Arrays.asList("1.1.1.1","1.1.1.2"));

        commonService.fillBackUpAgentConnectedIps(restoreEndPoints);
        Assert.assertEquals(endpoint.getAdvanceParams().get(BackupConstant.AGENT_CONNECTED_IPS), "[\"1.1.1.1\",\"1.1.1.2\"]");
    }

    /**
     * 用例场景：查出所有的备份网络，将其添加到恢复高级参数中
     * 前置条件：无
     * 检查点: 添加成功
     */
    @Test
    public void fill_restore_agent_connected_ips_success() {
        Endpoint endpoint = new Endpoint();
        endpoint.setAdvanceParams(new HashMap<>());
        List<Endpoint> restoreEndPoints = Arrays.asList(endpoint);
        DeviceNetworkInfo deviceNetworkInfo = new DeviceNetworkInfo();
        deviceNetworkInfo.setBackupConfig(new ArrayList<>());

        Mockito.when(networkService.getDeviceNetworkInfo()).thenReturn(deviceNetworkInfo);
        Mockito.when(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig())).thenReturn(Arrays.asList("1.1.1.1","1.1.1.2"));

        commonService.fillReStoreAgentConnectedIps(restoreEndPoints);
        Assert.assertEquals(endpoint.getAdvanceParams().get(BackupConstant.AGENT_CONNECTED_IPS), "[\"1.1.1.1\",\"1.1.1.2\"]");
    }
}