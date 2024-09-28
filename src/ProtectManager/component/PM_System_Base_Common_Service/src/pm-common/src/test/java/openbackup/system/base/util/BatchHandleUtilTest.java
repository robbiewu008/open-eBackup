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

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.util.BatchHandleUtil;

import org.apache.commons.collections.CollectionUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {CollectionUtils.class})
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
public class BatchHandleUtilTest {
    /**
     * 测试场景：假分页
     * 前置条件：NA
     * 检查点：分页数据正确
     */
    @Test
    public void test_false_pagination_page_success() throws Exception {
        // run the test
        List<String> sourceList = Arrays.asList("a", "b", "c", "d", "e", "f", "g");
        PageListResponse<String> result = BatchHandleUtil.fakePaginationPage(sourceList, 2,3);

        // verify the results
        Assert.assertEquals("e", result.getRecords().get(1));
        Assert.assertEquals(7, result.getTotalCount());
    }

    /**
     * 测试场景：假分页
     * 前置条件：NA
     * 检查点：分页数据正确
     */
    @Test
    public void test_false_pagination_list_success() throws Exception {
        // run the test
        List<String> sourceList = Arrays.asList("a", "b", "c", "d", "e", "f", "g");
        List<String> result = BatchHandleUtil.fakePaginationList(sourceList, 1, 3);

        // verify the results
        Assert.assertEquals("b", result.get(1));

    }
}
