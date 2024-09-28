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
package openbackup.goldendb.protection.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 */
public class GoldenInstanceTest {
    @Test
    public void testGoldenInstance() {
        EqualsVerifier.simple().forClass(GoldenInstance.class).verify();
        EqualsVerifier.simple().forClass(GoldenInstance.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
