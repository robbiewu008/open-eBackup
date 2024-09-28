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
package openbackup.system.base.sdk.accesspoint.model.enums;

import openbackup.system.base.sdk.accesspoint.model.enums.InitializeResultCode;

import org.junit.Assert;
import org.junit.Test;

/**
 * InitializeResultCode test
 *
 */
public class InitializeResultCodeTest {

    @Test
    public void get_initialize_result_code_success() {
        String zero = InitializeResultCode.forValues("0").toString();
        Assert.assertEquals("0", zero);

        String SUCCESS = InitializeResultCode.forValues("0").name();
        Assert.assertEquals("SUCCESS", SUCCESS);

    }

    @Test
    public void should_return_true_if_code_is_zero_when_is_ok() {
        InitializeResultCode SUCCESS = InitializeResultCode.forValues("0");
        boolean successOk = SUCCESS.isOk();
        Assert.assertTrue(successOk);
    }

    @Test
    public void should_return_false_if_code_is_not_zero_when_is_ok() {
        InitializeResultCode ERROR_NO_NODE = InitializeResultCode.forValues("10000");
        boolean falseCode = ERROR_NO_NODE.isOk();
        Assert.assertFalse(falseCode);
    }
}
