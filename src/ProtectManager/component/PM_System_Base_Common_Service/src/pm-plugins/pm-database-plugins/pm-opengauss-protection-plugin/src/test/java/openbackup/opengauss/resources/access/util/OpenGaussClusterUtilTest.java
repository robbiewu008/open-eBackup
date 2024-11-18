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
package openbackup.opengauss.resources.access.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.provider.OpenGaussMockData;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

import java.util.Map;

/**
 * OpenGaussClusterUtil测试类
 *
 */
public class OpenGaussClusterUtilTest {
    /**
     * 用例场景  构建环境信息
     * 前置条件：集群环境功能信息正常
     * 检查点: 环境对象扩展信息正常封装
     */
    @Test
    public void build_protected_environment_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        AppEnvResponse appEnvResponse = OpenGaussMockData.buildAppEnvResponse();
        ProtectedEnvironment protectedEnvironment1 = OpenGaussClusterUtil.buildProtectedEnvironment(
            protectedEnvironment, appEnvResponse);
        Map<String, String> extendInfo = protectedEnvironment1.getExtendInfo();
        Assert.assertEquals(5, extendInfo.size());
        Assert.assertEquals("Normal", protectedEnvironment1.getExtendInfo().get("clusterState"));
        Assert.assertNotNull(extendInfo.get(OpenGaussConstants.NODES));
    }

    /**
     * 用例场景  检查集群信息是否包含状态值
     * 前置条件：查询集群接口返回正常集群信息
     * 检查点: 查询集群状态是否为Normal
     */
    @Test
    public void should_return_success_if_clusterState_normal_when_check_cluster_stat1e() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () ->  OpenGaussClusterUtil.getContextClusterInfo(OpenGaussMockData.mockResourceCheckContext()));
        Assert.assertEquals("failed to query OpenGauss cluster nodes.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, legoCheckedException.getErrorCode());

    }

    /**
     * 用例场景  检查集群信息是否包含状态值
     * 前置条件：查询集群接口返回正常集群信息
     * 检查点: 查询集群状态是否为Normal
     */
    @Test
    public void should_return_success_if_clusterState_normal_when_check_cluster_state() {
        AppEnvResponse appEnvResponse = OpenGaussMockData.buildAppEnvResponse();
        OpenGaussClusterUtil.checkClusterState(appEnvResponse);

        appEnvResponse.getExtendInfo().put("clusterState", "Unavailable");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () ->  OpenGaussClusterUtil.checkClusterState(appEnvResponse));
        Assert.assertEquals("The open gauss cluster status is not normal", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, legoCheckedException.getErrorCode());
    }
}