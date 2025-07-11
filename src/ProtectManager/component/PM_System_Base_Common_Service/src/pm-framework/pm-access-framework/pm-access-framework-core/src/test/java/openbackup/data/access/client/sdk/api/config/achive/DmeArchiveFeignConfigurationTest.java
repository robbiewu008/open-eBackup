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
package openbackup.data.access.client.sdk.api.config.achive;

import feign.ExceptionPropagationPolicy;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * DmeArchiveFeignConfiguration LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({DmeArchiveFeignConfiguration.class})
public class DmeArchiveFeignConfigurationTest {

    private DmeArchiveFeignConfiguration feignConfiguration;

    @Before
    public void init() {
        DmeArchiveFeignConfiguration configuration = new DmeArchiveFeignConfiguration();
        feignConfiguration = PowerMockito.spy(configuration);

    }

    /**
     * 用例场景：创建feign的构建器抛出异常
     * 前置条件：
     * 检查点：抛出NullPointerException异常
     */
    @Test(expected = NullPointerException.class)
    public void should_throw_NullPointerException_if_feignClientConfig_is_null() {
        feignConfiguration.feignBuilder();
    }

    /**
     * 用例场景：ExceptionPropagationPolicy
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void do_exception_propagation_policy_success() {
        ExceptionPropagationPolicy value = feignConfiguration.policy();
        Assert.assertNotNull(value);
    }

    /**
     * 用例场景：errorDecoder
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void do_error_decoder_success() {
        ErrorDecoder value = feignConfiguration.errorDecoder();
        Assert.assertNotNull(value);
    }

    /**
     * 用例场景：decoder
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void do_decoder_success() {
        Decoder value = feignConfiguration.decoder();
        Assert.assertNotNull(value);
    }
}
