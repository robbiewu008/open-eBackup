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

import openbackup.system.base.sdk.accesspoint.model.InitNetworkResult;
import openbackup.system.base.sdk.accesspoint.model.enums.InitNetworkResultCode;

import org.junit.Assert;
import org.junit.Test;

/**
 * InitNetworkResult test
 *
 * @author jwx701567
 * @since 2021-03-15
 */
public class InitNetworkResultTest {

    @Test
    public void init_network_result_success() {
        InitNetworkResult init_success = new InitNetworkResult();
        InitNetworkResult success =
                init_success.addInitBackActionResult(InitNetworkResultCode.SUCCESS, "init success");
        boolean okay = success.isOkay();

        Assert.assertTrue(okay);
    }

    @Test
    public void should_return_false_if_code_is_failure_when_is_ok() {
        InitNetworkResult init_failure = new InitNetworkResult(InitNetworkResultCode.FAILURE, "INIT FAILURE");
        boolean okay = init_failure.isOkay();
        Assert.assertFalse(okay);
    }

    @Test
    public void add_init_back_action_result_success() {
        InitNetworkResult init_failure = new InitNetworkResult(InitNetworkResultCode.FAILURE, "INIT FAILURE");

        InitNetworkResult initNetworkResult = init_failure.addInitBackActionResult(init_failure);
        Assert.assertFalse(initNetworkResult.isOkay());
    }
}
