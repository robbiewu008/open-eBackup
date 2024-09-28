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
package openbackup.sqlserver.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * sqlserver集群 测试类
 *
 */
public class SqlServerClusterResourceProviderTest {
    private final SqlServerClusterResourceProvider sqlServerClusterResourceProvider
        = new SqlServerClusterResourceProvider();

    /**
     * 用例场景：SQL Server集群资源扫描
     * 前置条件：sql server cluster进行scan
     * 检查点：scan feature是false
     */
    @Test
    public void test_feature_params_is_false() {
        ResourceFeature resourceFeature = sqlServerClusterResourceProvider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isShouldUpdateDependencyHostInfoWhenScan());
    }

    /**
     * 用例场景：SQL Server集群资源扫描
     * 前置条件：sql server cluster进行scan
     * 检查点：进入provider
     */
    @Test
    public void test_sqlserver_cluster_resource_intercept() {
        ProtectedResource sqlServerClusterResource = new ProtectedResource();
        sqlServerClusterResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType());
        Assert.assertTrue(sqlServerClusterResourceProvider.applicable(sqlServerClusterResource));

        sqlServerClusterResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
        Assert.assertFalse(sqlServerClusterResourceProvider.applicable(sqlServerClusterResource));
    }
}
