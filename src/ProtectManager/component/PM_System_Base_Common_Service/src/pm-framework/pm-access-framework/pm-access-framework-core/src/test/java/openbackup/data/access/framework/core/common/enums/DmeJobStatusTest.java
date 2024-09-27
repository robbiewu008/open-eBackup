/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums;


import openbackup.data.access.framework.core.common.enums.DmcJobStatus;

import org.junit.Assert;
import org.junit.Test;

/**
 * DmeJobStatus LLT
 *
 * @author z30027603
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-07-26
 */
public class DmeJobStatusTest {
    @Test
    public void get_type_success() {
        Assert.assertEquals(DmcJobStatus.getByStatus(3).isSuccess(), true);
        Assert.assertEquals(DmcJobStatus.getByStatus(4).isSuccess(), false);
        Assert.assertEquals(DmcJobStatus.getByStatus(6).isSuccess(), false);
        Assert.assertEquals(DmcJobStatus.getByStatus(13).isSuccess(), true);
        Assert.assertEquals(DmcJobStatus.getByStatus(15).isSuccess(), false);
    }

    @Test
    public void get_name_success() {
        Assert.assertEquals(DmcJobStatus.getByName("SUCCESS").isSuccess(), true);
        Assert.assertEquals(DmcJobStatus.getByName("ABORTED").isSuccess(), false);
        Assert.assertEquals(DmcJobStatus.getByName("FAIL").isSuccess(), false);
        Assert.assertEquals(DmcJobStatus.getByName("PARTIAL_SUCCESS").isSuccess(), true);
        Assert.assertEquals(DmcJobStatus.getByName("ABORT_FAILED").isSuccess(), false);
    }
    @Test
    public void get_protection_status() {
        Assert.assertEquals(DmcJobStatus.getByStatus(3).getProtectionStatus(), 1);
        Assert.assertEquals(DmcJobStatus.getByStatus(4).getProtectionStatus(), 3);
        Assert.assertEquals(DmcJobStatus.getByStatus(6).getProtectionStatus(), 0);
        Assert.assertEquals(DmcJobStatus.getByStatus(13).getProtectionStatus(), 2);
        Assert.assertEquals(DmcJobStatus.getByStatus(15).getProtectionStatus(), 0);
    }
}
