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


import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * RestoreTypeEnum LLT
 *
 */
public class RestoreTypeEnumTest {
    @Test
    public void get_type_success() {
        Assert.assertEquals(RestoreTypeEnum.getByType("normalRestore").getType(), "normalRestore");
        Assert.assertEquals(RestoreTypeEnum.getByType("instantRestore").getType(), "instantRestore");
        Assert.assertEquals(RestoreTypeEnum.getByType("fineGrainedRestore").getType(), "fineGrainedRestore");
    }
}
