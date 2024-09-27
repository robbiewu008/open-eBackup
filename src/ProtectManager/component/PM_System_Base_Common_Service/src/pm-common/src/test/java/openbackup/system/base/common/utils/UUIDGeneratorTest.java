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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Assert;
import org.junit.Test;

/**
 * UUIDGeneratorTest
 *
 * @author w00439064
 * @since 2021-04-05
 */
public class UUIDGeneratorTest {
    /**
     * testGetUUID
     */
    @Test
    public void testGetUUID() {
        String[] arr = UUIDGenerator.getUUID(2);
        Assert.assertFalse(arr[0].contains("-"));
        Assert.assertEquals(arr.length, 2);
    }
}
