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
package openbackup.clickhouse.plugin.provider;

import static org.junit.Assert.assertEquals;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;

/**
 * ClickHouseResourceProvider Test
 *
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseResourceProviderTest {
    @InjectMocks
    private ClickHouseResourceProvider clickHouseResourceProvider;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：ClickHouse类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("hello");
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Assert.assertTrue(clickHouseResourceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：创建节点成功
     * 前置条件：创建前检查
     * 检查点: 设置成功
     */
    @Test
    public void beforeCreate_Node_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("hello");
        clickHouseResourceProvider.beforeCreate(protectedResource);
    }

    /**
     * 用例场景：创建ClickHouse集群成功
     * 前置条件：创建前检查
     * 检查点: 设置成功
     */
    @Test
    public void beforeCreate_Cluster_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("hello");
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        clickHouseResourceProvider.beforeCreate(protectedResource);
    }

    /**
     * 用例场景：测试beforeUpdate在名字异常时错误
     * 前置条件：输入名字不符合标准
     * 检查点：是否抛出异常，且错误码符合预期。
     */
    @Test
    public void should_throw_LegoCheckedException_if_name_is_illegal_when_beforeUpdate() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("$$");
        try {
            clickHouseResourceProvider.beforeUpdate(resource);
        } catch (LegoCheckedException e) {
            assertEquals(e.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }
    }

    /**
     * 用例场景：更新ClickHosue集群成功
     * 前置条件：更新前检查
     * 检查点: 设置成功
     */
    @Test
    public void beforeUpdate_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("hello");
        clickHouseResourceProvider.beforeUpdate(resource);
    }
}