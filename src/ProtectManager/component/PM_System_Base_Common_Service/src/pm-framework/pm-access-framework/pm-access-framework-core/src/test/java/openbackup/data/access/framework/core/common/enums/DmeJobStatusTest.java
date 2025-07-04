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
package openbackup.data.access.framework.core.common.enums;


import openbackup.data.access.framework.core.common.enums.DmcJobStatus;

import org.junit.Assert;
import org.junit.Test;

/**
 * DmeJobStatus LLT
 *
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
