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
package openbackup.system.base.common.utils.json;

import static org.assertj.core.api.BDDAssertions.then;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JSONObjectCovert;

import org.junit.Test;

/**
 * JSONObjectCovert工具类测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/28
 **/
public class JSONObjectCovertTest {

    /**
     * 用例名称：验证JSONObject中key从小写下划线转为小写驼峰成功<br/>
     * 前置条件：无<br/>
     * check点：1.转换对象不为空 2.对应属性转换成功<br/>
     */
    @Test
    public void should_success_when_test_covertLowerUnderscoreKeyToLowerCamel_given_lower_underscore_data(){
        // Given
        JSONObject mockSource = new JSONObject();
        mockSource.put("resource_id", "aaaaa");
        mockSource.put("resource_name", "bbbbb");
        // When
        final JSONObject target = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(mockSource);
        // Then
        then(target).isNotNull()
            .isNotEmpty()
            .hasFieldOrPropertyWithValue("resourceId", "aaaaa")
            .hasFieldOrPropertyWithValue("resourceName", "bbbbb");
    }

}