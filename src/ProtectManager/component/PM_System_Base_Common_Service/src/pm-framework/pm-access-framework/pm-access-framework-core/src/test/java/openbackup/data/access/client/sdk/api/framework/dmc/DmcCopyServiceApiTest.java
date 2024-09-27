/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dmc;

import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.framework.dmc.DmcCopyServiceApi;
import openbackup.data.access.client.sdk.api.framework.dmc.model.CopyDetail;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import static org.assertj.core.api.AssertionsForClassTypes.assertThat;
import static org.mockito.ArgumentMatchers.any;

/**
 * DmcCopyServiceApi LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest({DmcCopyServiceApi.class})
public class DmcCopyServiceApiTest {

    /**
     * 用例场景：查询备份产生的副本详情
     * 前置条件：mock
     * 检查点：查询返回成功
     */
    @Test
    public void should_query_copy_by_id_success() {
        DmcCopyServiceApi mock = PowerMockito.mock(DmcCopyServiceApi.class);
        DmeResponse<CopyDetail> value = mock.queryCopyById(any(), any());
        PowerMockito.when(mock.queryCopyById(any(), any())).thenCallRealMethod();
        assertThat(mock.queryCopyById(any(), any())).isEqualTo(value);
    }
}
