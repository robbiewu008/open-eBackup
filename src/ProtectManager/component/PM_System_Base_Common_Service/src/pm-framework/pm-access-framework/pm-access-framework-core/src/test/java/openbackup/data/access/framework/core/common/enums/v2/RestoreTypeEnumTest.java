/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums.v2;


import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * RestoreTypeEnum LLT
 *
 * @author z30027603
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-07-26
 */
public class RestoreTypeEnumTest {
    @Test
    public void get_type_success() {
        Assert.assertEquals(RestoreTypeEnum.getByType("normalRestore").getType(), "normalRestore");
        Assert.assertEquals(RestoreTypeEnum.getByType("instantRestore").getType(), "instantRestore");
        Assert.assertEquals(RestoreTypeEnum.getByType("fineGrainedRestore").getType(), "fineGrainedRestore");
    }
}
