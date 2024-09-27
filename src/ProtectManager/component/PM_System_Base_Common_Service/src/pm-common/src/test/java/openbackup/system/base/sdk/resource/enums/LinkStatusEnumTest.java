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
package openbackup.system.base.sdk.resource.enums;

import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * LinkStatusEnum test
 *
 * @author jwx701567
 * @since 2021-03-17
 */
public class LinkStatusEnumTest {


    @Test
    public void getByStatus() {
        String ONLINE = LinkStatusEnum.getByStatus(1).name();
        Assert.assertEquals("ONLINE", ONLINE);
    }

}
