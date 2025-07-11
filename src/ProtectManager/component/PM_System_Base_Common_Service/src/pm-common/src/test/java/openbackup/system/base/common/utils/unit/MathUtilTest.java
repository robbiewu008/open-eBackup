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
package openbackup.system.base.common.utils.unit;

import junit.framework.TestCase;
import openbackup.system.base.common.utils.unit.MathUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述 MathUtil测试类
 *
 */
public class MathUtilTest extends TestCase {
    /**
     * 用例场景：输入两位数字，及保留小数位数
     * 前置条件：无
     * 检查点：返回结果是否符合保留小数要求
     */
    @Test
    public void testComputePercent() {
        String result = MathUtil.computePercentNoSignWithTwoDecimal(381.488, 41456.64, 4);
        Assert.assertEquals(result, "0.9202");
    }
}