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
package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * CopyGeneratedByEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class CopyGeneratedByEnumTest {
    @Test
    public void should_return_status_if_enum_is_exists_when_query_statu() {
        String Backup = CopyGeneratedByEnum.valueOf("BY_BACKUP").value();
        Assert.assertEquals("Backup", Backup);
    }

}
