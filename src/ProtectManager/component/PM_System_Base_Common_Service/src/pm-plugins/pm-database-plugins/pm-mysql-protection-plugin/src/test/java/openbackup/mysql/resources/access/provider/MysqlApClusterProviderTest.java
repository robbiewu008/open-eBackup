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
package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ap集群校验测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class MysqlApClusterProviderTest {

    private ResourceService resourceService;

    private MysqlApClusterProvider mysqlApClusterProvider;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.mysqlApClusterProvider = new MysqlApClusterProvider(this.resourceService);
    }

    /**
     * 用例场景：mysql集群类型检查类provider过滤
     * 前置条件：无
     * 检查点：集群类型类过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mysqlApClusterProvider.applicable(MysqlConstants.MYSQL_AP));
    }

    /**
     * 用例场景：mysql主备集群校验
     * 前置条件：集群节点数量和实例数量一致
     * 检查点：检验成功
     */
    @Test
    public void check_cluster_success() {
        ProtectedResource clusterEnvironment = getProtectedResource(2,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType());

        ProtectedResource clusterInstance = getProtectedResource(2,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(java.util.Optional.of(clusterEnvironment));
        Assert.assertTrue(mysqlApClusterProvider.checkIsCluster(clusterInstance));
    }

    @Test
    public void check_role_condition_success() {
        List<Boolean> rolesList = Arrays.asList(Boolean.TRUE, Boolean.FALSE, Boolean.FALSE);
        mysqlApClusterProvider.checkNodeRoleCondition(rolesList);
        List<Boolean> notLegalList = Arrays.asList(Boolean.TRUE, Boolean.TRUE, Boolean.FALSE);
        Assert.assertThrows(LegoCheckedException.class, () -> {
            mysqlApClusterProvider.checkNodeRoleCondition(notLegalList);
        });
    }

    /**
     * 用例场景：mysql主备集群校验
     * 前置条件：集群节点数量和实例数量不一致
     * 检查点：检验失败
     */
    @Test
    public void check_cluster_failed_when_nodes_not_match() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource clusterEnvironment = getProtectedResource(2,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType());

        ProtectedResource clusterInstance = getProtectedResource(1,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(java.util.Optional.of(clusterEnvironment));
        mysqlApClusterProvider.checkIsCluster(clusterInstance);
    }

    /**
     * 用例场景：mysql主备集群校验
     * 前置条件：clusterInstance集群实例下节点为空
     * 检查点：检验失败
     */
    @Test
    public void check_cluster_failed_when_nodes_is_null() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource clusterEnvironment = getProtectedResource(2,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType());

        ProtectedResource clusterInstance = getProtectedResource(0,
            MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(java.util.Optional.of(clusterEnvironment));
        mysqlApClusterProvider.checkIsCluster(clusterInstance);
    }

    /**
     * 构建资源
     *
     * @return 资源
     */
    private ProtectedResource getProtectedResource(int agentsNum, String subType) {
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();

        ProtectedResource resourceItem = new ProtectedResource();
        resourceItem.setParentUuid("11111");
        ArrayList<ProtectedResource> agentsList = new ArrayList<>();
        for (int i = 0; i < agentsNum; i++) {
            agentsList.add(resourceItem);
        }
        dependency.put(DatabaseConstants.AGENTS, agentsList);
        dependency.put(DatabaseConstants.CHILDREN, agentsList);
        protectedResource.setDependencies(dependency);
        protectedResource.setSubType(subType);
        protectedResource.setRootUuid("11111");
        Map<String, List<ProtectedResource>> itemDependency = new HashMap<>();
        itemDependency.put(DatabaseConstants.AGENTS, Collections.singletonList(new ProtectedResource()));
        resourceItem.setDependencies(itemDependency);
        return protectedResource;
    }
}
