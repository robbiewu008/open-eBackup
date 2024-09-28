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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.NumberConverter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;
import javax.annotation.Resource;

/**
 * Number Converter Test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = NumberConverter.class)
public class NumberConverterTest {

    @Resource
    private NumberConverter numberConverter;


    /**
     * 测试场景：能正确处理入参为空，包含数字和字符串等情况 <br/>
     * 前置条件：入参为null <br/>
     * 检查点：正确处理，如果不能转换为字符串返回null
     */
    @Test
    public void test_cast () {
        Assert.assertNull(numberConverter.cast(null));
        Assert.assertEquals(12L, numberConverter.cast("12"));
        Assert.assertEquals(12D, numberConverter.cast("12.0"));
        Assert.assertNull(numberConverter.cast("12.0CCCC"));
        Assert.assertNull(numberConverter.cast(new Object()));
    }
}
