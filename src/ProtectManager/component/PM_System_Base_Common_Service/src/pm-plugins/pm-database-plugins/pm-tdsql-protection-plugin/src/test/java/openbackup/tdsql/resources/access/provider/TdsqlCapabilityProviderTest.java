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
package openbackup.tdsql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * TdsqlCapabilityProviderTest
 *
 */
public class TdsqlCapabilityProviderTest {
    /**
     * 用例场景：测试获取Tdsql副本支持的能力
     * 前置条件：无
     * 检 查 点：获取Tdsql副本支持的能力正确
     */
    @Test
    public void test_support_features() {
        TdsqlCapabilityProvider provider = new TdsqlCapabilityProvider();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()));
        List<CopyFeatureEnum> copyFeatureEnums = provider.supportFeatures();
        Assert.assertTrue(copyFeatureEnums.containsAll(Arrays.asList(CopyFeatureEnum.RESTORE, CopyFeatureEnum.MOUNT)));
    }

}