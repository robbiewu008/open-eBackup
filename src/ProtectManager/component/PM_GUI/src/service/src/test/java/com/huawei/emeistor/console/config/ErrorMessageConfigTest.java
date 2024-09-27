/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.context.support.ReloadableResourceBundleMessageSource;

/**
 * 功能描述: ErrorMessageConfigTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-2-28
 */
public class ErrorMessageConfigTest {
    @Test
    public void testMessageSource() {
        ErrorMessageConfig config = new ErrorMessageConfig();
        ReloadableResourceBundleMessageSource messageSource = config.messageSource();
        Assert.assertNotNull(messageSource);
    }
}