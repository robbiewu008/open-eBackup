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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;

import openbackup.system.base.common.enums.PMModuleEnum;

import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * VariableParameterService测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ConfigMapUtil.class})
public class VariableParameterServiceImplTest {

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取String类cm成功。<br/>
     * 检查点：获取cm成功。
     */
    @Test
    void test_get_config_value_or_default_string() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";  // 假设的模块名
        String defaultValue = "default";
        String correctParamName = moduleName + "_" + paramName;

        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn("123");
        String result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertEquals("123", result);
    }

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取cm失败。<br/>
     * 检查点：输出默认值。
     */
    @Test
    void test_get_config_value_or_default_string_default() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";
        String defaultValue = "default";
        String correctParamName = moduleName + "_" + paramName;

        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn(null);
        String result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertEquals(defaultValue, result);
    }

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取int类cm成功。<br/>
     * 检查点：输出cm值。
     */
    @Test
    void test_get_config_value_or_default_int() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";
        int defaultValue = 10;
        String correctParamName = moduleName + "_" + paramName;

        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn("20");
        int result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertEquals(20, result);
    }

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取int类cm失败。<br/>
     * 检查点：输出默认值。
     */
    @Test
    void test_get_config_value_or_default_int_parse_exception() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";
        int defaultValue = 10;
        String correctParamName = moduleName + "_" + paramName;
        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn("InvalidValue");
        int result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertEquals(defaultValue, result);
    }

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取boolean类cm。<br/>
     * 检查点：输出cm值。
     */
    @Test
    void test_get_config_value_or_default_boolean() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";
        boolean defaultValue = true;
        PowerMockito.mockStatic(ConfigMapUtil.class);
        String correctParamName = moduleName + "_" + paramName;
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn("false");
        boolean result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertFalse(result);
    }

    /**
     * 用例名称：是否能通过cm的值修改获取参数。<br/>
     * 前置条件：获取boolean类cm失败。<br/>
     * 检查点：输出默认值。
     */
    @Test
    void test_GetConfigValueOrDefaultBooleanParseException() {
        String paramName = "Tag_MAXIMUM_LABEL_RESOURCE_SIZE";
        String moduleName = "ModuleName";
        boolean defaultValue = true;
        String correctParamName = moduleName + "_" + paramName;
        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap("variable-parameter-conf", correctParamName))
            .thenReturn("InvalidValue");
        boolean result = VariableParameterUtil.getConfigValueOrDefault(PMModuleEnum.valueOf(moduleName), paramName,
            defaultValue);
        assertEquals(defaultValue, result);
    }
}
