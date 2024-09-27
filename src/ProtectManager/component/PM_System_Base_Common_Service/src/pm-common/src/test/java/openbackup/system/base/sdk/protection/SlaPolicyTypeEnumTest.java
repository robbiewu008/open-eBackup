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
package openbackup.system.base.sdk.protection;

import openbackup.system.base.sdk.protection.emuns.SlaPolicyActionEnum;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import org.junit.Assert;
import org.junit.Test;

/**
 * SlaPolicyTypeEnum test
 *
 * @author jwx701567
 * @since 2021-03-16
 */
public class SlaPolicyTypeEnumTest {
    @Test
    public void getTypeByName() {
        String BACKUP = SlaPolicyTypeEnum.getTypeByName("backup").name();
        Assert.assertEquals("BACKUP", BACKUP);
    }

    @Test
    public void getActionByName() {
        String FULL = SlaPolicyActionEnum.getActionByName("full").name();
        Assert.assertEquals("FULL", FULL);
    }
}
