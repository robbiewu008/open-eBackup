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

import openbackup.system.base.common.aspect.I18nConverter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

/**
 * I18n Converter Test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = I18nConverter.class)
public class I18nConverterTest {

    @Resource
    private I18nConverter i18nConverter;

    /**
     * 测试场景：获取名称 <br/>
     * 前置条件：无 <br/>
     * 检查点：正确返回名称
     */
    @Test
    public void test_get_name () {
        Assert.assertEquals("i18n", i18nConverter.getName());
    }

    /**
     * 测试场景：当输入前缀和后缀时能正常拼接前后缀 <br/>
     * 前置条件：无 <br/>
     * 检查点：正确返回拼接值
     */
    @Test
    public void test_convert () {
        String suffix = "xx";
        String prefix = "yy";
        ArrayList<String> spaceThreshold = new ArrayList<>();
        spaceThreshold.add(suffix);
        Collection<?> convert = i18nConverter.convert(spaceThreshold);
        Iterator<?> iterator = convert.iterator();
        while(iterator.hasNext()){
            I18nConverter.I18nObject next = (I18nConverter.I18nObject) iterator.next();
            I18nConverter.I18nObject i18nObject = next.get("yy");
            Assert.assertEquals(prefix + "_" + suffix + "_label", i18nObject.toString());
        }
    }
}
