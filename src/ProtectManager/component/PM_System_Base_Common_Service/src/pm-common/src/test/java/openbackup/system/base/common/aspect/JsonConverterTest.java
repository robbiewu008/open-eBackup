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

import com.alibaba.fastjson.JSON;

import openbackup.system.base.common.aspect.JsonConverter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;
import javax.annotation.Resource;
import java.util.ArrayList;
import java.util.List;

/**
 * Json Converter Test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = JsonConverter.class)
public class JsonConverterTest {

    @Resource
    private JsonConverter jsonConverter;

    /**
     * 测试场景：当输入参数为null时，是否能正确处理 <br/>
     * 前置条件：入参为null <br/>
     * 检查点：返回null
     */
    @Test
    public void should_return_null_when_param_is_null () {
        Assert.assertNull(jsonConverter.cast(null));
    }

    /**
     * 测试场景：当输入参数为集合时，是否能正确处理 <br/>
     * 前置条件：入参为集合 <br/>
     * 检查点：正确返回JSONArray
     */
    @Test
    public void should_return_json_array_when_param_is_collection () {
        List<String> lists = new ArrayList<>();
        lists.add("1");
        lists.add("2");
        Assert.assertEquals("[\"1\",\"2\"]", jsonConverter.cast(lists).toString());
    }

    /**
     * 测试场景：当输入参数不为集合或则数组时，是否能正确处理 <br/>
     * 前置条件：入参不为集合 <br/>
     * 检查点：正确返回JSONArray
     */
    @Test
    public void should_return_json_object_when_param_is_not_collection () {
        MyJson myJson = new MyJson("1", 2);
        Assert.assertEquals(JSON.toJSONString(myJson), jsonConverter.cast(myJson).toString());
    }


    static class MyJson{
        private String name;

        private Integer age;

        public MyJson(String name, Integer age) {
            this.name = name;
            this.age = age;
        }
    }
}
