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
package openbackup.data.access.client.sdk.api;

import openbackup.data.access.client.sdk.api.AccessPointRestApiFeignClientFactoryBean;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * AccessPointRestApiFeignClientFactoryBean LLT
 *
 */

@RunWith(PowerMockRunner.class)
public class AccessPointRestApiFeignClientFactoryBeanTest {

    /**
     * 用例场景：测试默认构造函数
     * 前置条件：
     * 检查点：创建实例成功
     */
    @Test
    public void should_default_construct_create_instance_success() {
        AccessPointRestApiFeignClientFactoryBean bean = new AccessPointRestApiFeignClientFactoryBean();
        Assert.assertNotNull(bean);
    }
}
