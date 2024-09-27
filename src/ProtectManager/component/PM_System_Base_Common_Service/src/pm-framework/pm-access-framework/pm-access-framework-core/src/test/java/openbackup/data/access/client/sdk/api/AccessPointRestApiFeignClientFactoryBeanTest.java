/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
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
