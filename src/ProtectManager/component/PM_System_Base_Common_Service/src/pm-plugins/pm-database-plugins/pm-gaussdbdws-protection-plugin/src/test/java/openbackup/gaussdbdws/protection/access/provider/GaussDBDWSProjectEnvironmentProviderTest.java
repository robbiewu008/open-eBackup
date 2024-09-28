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
package openbackup.gaussdbdws.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDbdws project资源相关接口的具体实现类
 * 实现了：扫描，健康状态检查，资源浏览，环境信息检查相关等接口
 *
 */
public class GaussDBDWSProjectEnvironmentProviderTest {
    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker = Mockito.mock(
        UnifiedClusterResourceIntegrityChecker.class);

    private final GaussDBBaseService gaussDBBaseService = Mockito.mock(GaussDBBaseService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private final GaussDBDWSProjectEnvironmentProvider gaussDBDWSProjectEnvironmentProvider
        = new GaussDBDWSProjectEnvironmentProvider(providerManager, pluginConfigManager, clusterIntegrityChecker,
        gaussDBBaseService, agentUnifiedService);

    /**
     * 用例场景：GaussDB(DWS)集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            gaussDBDWSProjectEnvironmentProvider.applicable(ResourceSubTypeEnum.GAUSSDB_DWS_PROJECT.getType()));
    }

    /**
     * 用例场景：检查扫描返回的数据库对象是否是我们需要的
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void check_scan_success() {
        List<ProtectedResource> envResourceList = new ArrayList<>();
        envResourceList.add(new ProtectedResource());
        PowerMockito.when(
            gaussDBBaseService.getEnvResourceList("555555", ResourceSubTypeEnum.GAUSSDB_DWS.getType(), "autoscan"))
            .thenReturn(envResourceList);
        PowerMockito.when(gaussDBBaseService.getClusterUuid(any())).thenReturn(getProtectedEnvironment());
        PageListResponse<ProtectedResource> detailPageList = new PageListResponse<>();
        detailPageList.setTotalCount(1);
        detailPageList.setRecords(envResourceList);
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), false))
            .thenReturn(detailPageList);
        PowerMockito.when(gaussDBBaseService.getListResourceReq(any(), any(), any()))
            .thenReturn(new ListResourceV2Req());
        Assert.assertEquals(1, gaussDBDWSProjectEnvironmentProvider.scan(getProtectedEnvironment()).size());
    }

    /**
     * 用例场景：检查扫描返回的数据库对象是否是我们需要的
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void check_scan_failed() {
        List<ProtectedResource> envResourceList = new ArrayList<>();
        envResourceList.add(new ProtectedResource());
        PowerMockito.when(
            gaussDBBaseService.getEnvResourceList("555555", ResourceSubTypeEnum.GAUSSDB_DWS.getType(), "autoscan"))
            .thenReturn(envResourceList);
        PowerMockito.when(gaussDBBaseService.getClusterUuid(any())).thenReturn(getProtectedEnvironment());
        PageListResponse<ProtectedResource> detailPageList = new PageListResponse<>();
        detailPageList.setTotalCount(1);
        detailPageList.setRecords(envResourceList);
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), false))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED));
        PowerMockito.when(gaussDBBaseService.getListResourceReq(any(), any(), any()))
            .thenReturn(new ListResourceV2Req());
        Assert.assertEquals(1, gaussDBDWSProjectEnvironmentProvider.scan(getProtectedEnvironment()).size());
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：无
     * 检查点: 成功
     */
    @Test
    public void check_dws_environment_success() {
        checkSuccess();
        gaussDBDWSProjectEnvironmentProvider.register(getProtectedEnvironment());
    }

    /**
     * 用例场景：健康检查reportJobByLabelContinue
     * 前置条件：无
     * 检查点: 检查成功,不报错
     */
    @Test
    public void health_check_success() {
        checkSuccess();
        gaussDBDWSProjectEnvironmentProvider.validate(getProtectedEnvironment());
    }

    private void checkSuccess() {
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = new CheckResult<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo().put(DwsConstant.COORDINATOR_IP, "8.40.102.115");
        nodeInfo.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_VERSION, DwsConstant.HOST_NO_ADD_VERSION);
        nodes.add(nodeInfo);
        appEnvResponse.setNodes(nodes);
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put("cn_nodes", "8.40.102.115");
        appEnvResponseCheckResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(appEnvResponseCheckResult);
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("555555");
        protectedEnvironment.setName("qqqqqq");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DwsConstant.TERMINAL_NODE, "8.40.102.100");
        extendInfo.put(DwsConstant.IAM_USER_ACCOUNT, "root");
        protectedEnvironment.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11111111");
        resources.add(protectedResource);
        resourceMap.put(DwsConstant.DWS_CLUSTER_AGENT, resources);
        protectedEnvironment.setDependencies(resourceMap);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        authentication.setAuthPwd("Huawei@123");
        protectedEnvironment.setAuth(authentication);
        return protectedEnvironment;
    }
}
