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
package openbackup.tpops.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.jupiter.api.Test;
import org.junit.rules.ExpectedException;

import java.util.ArrayList;
import java.util.List;

/**
 * 功能描述
 *
 * @author t30021437
 * @since 2023-02-07
 */
public class TpopsGaussDbValidatorTest {
    @Rule
    private ExpectedException expectedException = ExpectedException.none();

    @Test
    public void check_gauss_db_count() {
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        for (int i = 0; i < 9; i++) {
            existingEnvironments.add(protectedEnvironment);
        }
        Assert.assertThrows(LegoCheckedException.class, () -> TpopsGaussDBValidator.checkGaussDbCount(existingEnvironments));
    }
}