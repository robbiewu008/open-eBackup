/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.config.achive;

import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponseError;
import openbackup.system.base.common.exception.LegoCheckedException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.Optional;

/**
 * DmeResponse LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest({DmeResponse.class, Optional.class})
public class DmeResponseTest {

    private DmeResponse mock;
    @Before
    public void init() {
        DmeResponse dmeResponse = new DmeResponse();
        mock = PowerMockito.spy(dmeResponse);
    }

    /**
     * 用例场景：getCheckedData
     * 前置条件：
     * 检查点：获取的值为空
     */
    @Test
    public void get_checked_data_success() {
        Object value = mock.getCheckedData();
        Assert.assertNull(value);
    }

    /**
     * 用例场景：getCheckedData
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_isPresent_return_true() {
        DmeResponseError error = new DmeResponseError();
        error.setDescription("test");
        error.getDescription();
        error.setCode(10L);
        Whitebox.setInternalState(mock, "error", error);
        mock.checkData();
    }
}
