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
package openbackup.system.base.util;

import openbackup.system.base.util.ThreadCache;

import org.junit.Assert;
import org.junit.Test;

/**
 * Thread Cache Test
 * 
 */
public class ThreadCacheTest {
    @Test
    public void test() {
        ThreadCache<Object> cache = new ThreadCache<>();
        Object[] data = new Object[] {new Object(), new Object()};
        cache.run(() -> {
            Assert.assertSame(data[0], cache.get());
            cache.run(() -> Assert.assertSame(data[1], cache.get()), data[1]);
            Assert.assertSame(data[0], cache.get());
        }, data[0]);
        Assert.assertNull(cache.get());
    }
}
