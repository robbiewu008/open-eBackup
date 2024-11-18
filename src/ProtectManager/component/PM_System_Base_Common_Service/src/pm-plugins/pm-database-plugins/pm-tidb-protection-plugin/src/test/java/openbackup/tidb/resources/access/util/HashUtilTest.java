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
package openbackup.tidb.resources.access.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.tidb.resources.access.util.HashUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 *  HashUtil测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(HashUtil.class)
@Slf4j
public class HashUtilTest {
    /**
     * 用例场景：测试digest()
     * 检查点：返回的结果符合预期
     */
    @Test
    public void test_digest() {
        String digestBytes = HashUtil.digest("plain");
        log.info(digestBytes);
        Assert.assertEquals(digestBytes, "a116c9ed46d6207734a43317d30fd88f52ac8634c37d904bbf4e41d865f90475");
    }
}