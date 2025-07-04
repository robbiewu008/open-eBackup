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
package openbackup.database.base.plugin.service.impl;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.IsmNumberConstant;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link InstanceProtectionServiceImpl} 测试类
 *
 */
public class InstanceProtectionServiceImplTest {
    private final static String endpoint = "8.40.99.125";

    private final static Integer port = 54321;

    private final InstanceProtectionServiceImpl instanceProtectionService = new InstanceProtectionServiceImpl();

    /**
     * 用例场景：获取单实例节点信息
     * 前置条件：无
     * 检查点：节点信息是否获取成功
     */
    @Test
    public void execute_get_single_instance_nodes_success() {
        List<TaskEnvironment> nodes = instanceProtectionService.extractEnvNodesBySingleInstance(mockResource());
        Assert.assertEquals(IsmNumberConstant.ONE, nodes.size());
    }

    /**
     * 用例场景：获取集群实例节点信息
     * 前置条件：无
     * 检查点：节点信息是否获取成功
     */
    @Test
    public void execute_get_cluster_instance_nodes_success() {
        List<TaskEnvironment> nodes = instanceProtectionService.extractEnvNodesByClusterInstance(mockClusterResource());
        Assert.assertEquals(IsmNumberConstant.TWO, nodes.size());
    }

    private ProtectedResource mockResource() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint(endpoint);
        protectedEnvironment.setPort(port);
        protectedEnvironment.setExtendInfo(new HashMap<>());
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(protectedEnvironment);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(dependencies);
        resource.setExtendInfo(new HashMap<>());
        return resource;
    }

    private ProtectedResource mockClusterResource() {
        ProtectedResource protectedResourceOne = mockResource();
        ProtectedResource protectedResourceTwo = mockResource();
        List<ProtectedResource> children = new ArrayList<>();
        children.add(protectedResourceOne);
        children.add(protectedResourceTwo);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(dependencies);
        return environment;
    }
}