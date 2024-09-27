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
package openbackup.system.base.util;

import openbackup.system.base.util.OpServiceUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * OpServiceUtilTest
 *
 * @author l30044826
 * @since 2023-08-29
 */
public class OpServiceUtilTest {
    @Test
    public void test_is_hcs_service_false(){
        Assert.assertFalse(OpServiceUtil.isHcsService());
    }
}
