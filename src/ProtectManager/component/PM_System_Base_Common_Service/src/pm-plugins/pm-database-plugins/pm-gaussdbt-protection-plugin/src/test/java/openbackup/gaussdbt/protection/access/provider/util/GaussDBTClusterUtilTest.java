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
package openbackup.gaussdbt.protection.access.provider.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTClusterStateEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDBT集群工具测试
 *
 */
public class GaussDBTClusterUtilTest {
    /**
     * 用例场景：设置集群的信息
     * 前置条件：集群查询回来的数据为正常
     * 检查点：集群在线，集群状态正常
     */
    @Test
    public void set_cluster_info_success_when_cluster_status_is_online() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("node01");
        AppEnvResponse appEnvResponse = buildAppResponse();
        ResourceCheckContext checkContext = buildResourceCheckContext(environment);
        GaussDBTClusterUtil.setClusterInfo(environment, appEnvResponse, checkContext);
        Assert.assertEquals("127.0.0.1;127.0.0.2", environment.getEndpoint());
        Assert.assertEquals("127.0.0.1;127.0.0.2", environment.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
        Assert.assertNotNull(environment.getExtendInfoByKey(GaussDBTConstant.NODES_KEY));
        Assert.assertEquals("1", environment.getExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals(GaussDBTClusterStateEnum.NORMAL.getState(),
            environment.getExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY));
    }

    /**
     * 用例场景：设置集群的信息
     * 前置条件：集群查询回来的数据为异常
     * 检查点：集群离线，集群状态未不可用
     */
    @Test
    public void set_cluster_info_success_when_cluster_status_is_offline() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("node01");
        AppEnvResponse appEnvResponse = buildAppResponse();
        appEnvResponse.getExtendInfo()
            .put(GaussDBTConstant.CLUSTER_STATE_KEY, GaussDBTClusterStateEnum.ABNORMAL.getState());
        ResourceCheckContext checkContext = buildResourceCheckContext(environment);
        GaussDBTClusterUtil.setClusterInfo(environment, appEnvResponse, checkContext);
        Assert.assertEquals("127.0.0.1;127.0.0.2", environment.getEndpoint());
        Assert.assertEquals("127.0.0.1;127.0.0.2", environment.getPath());
        Assert.assertEquals(LinkStatusEnum.OFFLINE.getStatus().toString(), environment.getLinkStatus());
        Assert.assertNotNull(environment.getExtendInfoByKey(GaussDBTConstant.NODES_KEY));
        Assert.assertEquals(GaussDBTClusterStateEnum.ABNORMAL.getState(),
            environment.getExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY));
    }

    private AppEnvResponse buildAppResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> appExtendInfo = new HashMap<>();
        appExtendInfo.put(GaussDBTConstant.CLUSTER_VERSION_KEY, "GaussDB_T_1.2.1");
        appExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, "1");
        appExtendInfo.put(GaussDBTConstant.CLUSTER_STATE_KEY, GaussDBTClusterStateEnum.NORMAL.getState());
        appEnvResponse.setExtendInfo(appExtendInfo);
        Map<String, String> nodeExtendInfo = new HashMap<>();
        nodeExtendInfo.put(DatabaseConstants.ROLE, "1");
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("node01");
        nodeInfo.setEndpoint("127.0.0.1");
        nodeInfo.setExtendInfo(nodeExtendInfo);
        NodeInfo nodeInfo2 = new NodeInfo();
        nodeInfo2.setName("node02");
        nodeInfo2.setEndpoint("127.0.0.2");
        nodeInfo2.setExtendInfo(nodeExtendInfo);
        List<NodeInfo> nodeInfos = Arrays.asList(nodeInfo, nodeInfo2);
        appEnvResponse.setNodes(nodeInfos);
        return appEnvResponse;
    }

    private ResourceCheckContext buildResourceCheckContext(ProtectedEnvironment environment) {
        ResourceCheckContext resourceCheckContext = new ResourceCheckContext();
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        List<ProtectedEnvironment> environments = new ArrayList<>();
        environments.add(environment);
        map.put(environment, environments);
        resourceCheckContext.setResourceConnectableMap(map);
        return resourceCheckContext;
    }
}
