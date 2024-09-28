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
package openbackup.data.access.framework.core.common.enums.v2;


import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * DmeCopyTypeEnum LLT
 *
 */
public class CopyTypeEnumTest {
    @Test
    public void test() {
        Assert.assertEquals(CopyTypeEnum.getCopyType("full").getCopyType(), "full");
        Assert.assertEquals(CopyTypeEnum.getCopyType("increment").getCopyType(), "difference_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("foreverIncrement").getCopyType(), "permanent_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("diff").getCopyType(), "cumulative_increment");
        Assert.assertEquals(CopyTypeEnum.getCopyType("log").getCopyType(), "log");
        Assert.assertEquals(CopyTypeEnum.getCopyType("s3Archive").getCopyType(), "s3Archive");
        Assert.assertEquals(CopyTypeEnum.getCopyType("replication").getCopyType(), "replication");
        Assert.assertEquals(CopyTypeEnum.getCopyType("tapeArchive").getCopyType(), "tapeArchive");
        Assert.assertEquals(CopyTypeEnum.getCopyType("nativeSnapshot").getCopyType(), "snapshot");
    }
}
