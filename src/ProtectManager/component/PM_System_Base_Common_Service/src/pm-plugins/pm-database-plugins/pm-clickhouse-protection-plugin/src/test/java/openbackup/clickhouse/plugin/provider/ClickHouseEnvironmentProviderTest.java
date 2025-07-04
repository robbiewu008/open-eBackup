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

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.Collections;
import java.util.HashMap;

/**
 * ClickHouseEnvironmentProvider Test
 *
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseEnvironmentProviderTest {
    @InjectMocks
    private ClickHouseEnvironmentProvider clickHouseEnvironmentProvider;

    @Mock
    private ClickHouseService clickHouseService;

    @Mock
    private ResourceService resourceService;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void setUp() throws Exception {
        // @Mock注不进去clickHouseService，手动设置下
        ReflectionTestUtils.setField(clickHouseEnvironmentProvider, "clickHouseService", clickHouseService);
    }

    /**
     * 用例场景：redis类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Assert.assertTrue(clickHouseEnvironmentProvider.applicable(protectedEnvironment.getSubType()));
    }

    /**
     * 用例场景：集群参数填写正确
     * 前置条件：无
     * 检查点: 校验成功
     */
    @Test
    public void check_success() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setName("T192_168");
        resource.setType(ClickHouseConstant.CLUSTER_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        resource.setDependencies(
            Collections.singletonMap(ResourceConstants.CHILDREN, Lists.newArrayList(new ProtectedResource())));
        clickHouseEnvironmentProvider.register(resource);
    }

    /**
     * 用例场景：扫描成功
     * 前置条件：无
     * 检查点: 无
     */
    @Test
    public void scanSuccess() {
        clickHouseEnvironmentProvider.scan(new ProtectedEnvironment());
    }
}