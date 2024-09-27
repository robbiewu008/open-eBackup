/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.util.EnumUtil;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

/**
 * EnumUtil Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
public class EnumUtilTest {
    /**
     * 测试get方法
     */
    @Test
    public void testGet() {
        IllegalArgumentException exception = Assertions.assertThrows(IllegalArgumentException.class,
            () -> EnumUtil.get(JobLogLevelEnum.class, JobLogLevelEnum::getValue, "name", false));
        Assertions.assertTrue(exception.getMessage().contains("name"));
    }
}
