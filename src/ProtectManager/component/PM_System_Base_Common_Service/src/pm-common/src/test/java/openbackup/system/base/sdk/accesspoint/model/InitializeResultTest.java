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
package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.accesspoint.model.InitializeResult;
import openbackup.system.base.sdk.accesspoint.model.InitializeResultDesc;
import openbackup.system.base.sdk.accesspoint.model.enums.InitializeResultCode;
import org.junit.Assert;
import org.junit.Test;

/**
 * InitializeResult test
 *
 */
public class InitializeResultTest {

    @Test
    public void init_initialize_result_success() {
        InitializeResult result = new InitializeResult();
        result.addActionResultDesc(
                new InitializeResultDesc(InitializeResultCode.SUCCESS, "Expand volume pool OK"));
        boolean ok = result.isOk();
        Assert.assertTrue(ok);

    }


    @Test
    public void should_return_false_if_code_is_not_zero_when_is_ok() {
        InitializeResult result = new InitializeResult(
                new InitializeResultDesc(
                        InitializeResultCode.ERROR_NO_NODE, "Query nodes failed or no any node"));
        boolean isOk = result.isOk();
        Assert.assertFalse(isOk);
    }

    @Test
    public void add_action_error() {
        InitializeResult result = new InitializeResult();
        InitializeResult initializeResultDesc = new InitializeResult(
                new InitializeResultDesc(InitializeResultCode.ERROR_MOUNT_FAILED, "Mount path(NFS) failed"));
        result.addActionError(initializeResultDesc);
        boolean isOk = result.isOk();
        Assert.assertFalse(isOk);
    }
}
