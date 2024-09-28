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
package openbackup.tdsql.resources.access.dto.instance;

/**
 * 功能描述
 *
 */

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述 测试TdsqlGroup类
 *
 */
public class TdsqlGroupTest {
    @Test
    public void test_tdsql_group() {
        EqualsVerifier.simple().forClass(TdsqlGroup.class).verify();
        EqualsVerifier.simple().forClass(TdsqlGroup.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
