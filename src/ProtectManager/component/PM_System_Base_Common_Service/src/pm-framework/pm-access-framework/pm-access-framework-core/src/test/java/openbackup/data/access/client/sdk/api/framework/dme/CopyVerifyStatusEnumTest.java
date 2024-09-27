/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * CopyVerifyStatusEnum LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest({CopyVerifyStatusEnum.class})
public class CopyVerifyStatusEnumTest {

    /**
     * 用例场景：根据副本校验状态值获取枚举类
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void create_CopyVerifyStatusEnum_success() {
        CopyVerifyStatusEnum notVerify = CopyVerifyStatusEnum.NOT_VERIFY;
        CopyVerifyStatusEnum mock = PowerMockito.spy(notVerify);
        CopyVerifyStatusEnum value = mock.getByStatus("3");
        Assert.assertNotNull(value);
    }
}
