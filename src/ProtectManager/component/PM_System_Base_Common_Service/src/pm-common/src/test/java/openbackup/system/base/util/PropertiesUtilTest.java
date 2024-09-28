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

import openbackup.system.base.util.PropertiesUtil;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.util.ResourceUtils;

import java.io.FileNotFoundException;
import java.util.Properties;

/**
 * 功能描述: PropertiesUtilTest
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class PropertiesUtilTest {
    /**
     * 用例场景：查询指定路径下配置文件
     * 前置条件：X8000、X6000运行正常
     * 检查点：查询配置文件是否成功
     */
    @Test
    public void test_get_properties() throws FileNotFoundException {
        // run the test
        Properties result = PropertiesUtil.getProperties(
            ResourceUtils.getFile("classpath:pm-system-base-pod-CyberEngine.yaml").getPath());

        // verify the results
        Assert.assertNotNull(result);
    }

    /**
     * 用例场景：查询挂载configMap指定key的value值
     * 前置条件：X8000、X6000运行正常
     * 检查点：查询挂载configMap指定key的value值是否成功
     */
    @Test
    public void test_get_conf_value() throws FileNotFoundException {
        // run the test
        String path = ResourceUtils.getFile("classpath:pm-system-base-pod-CyberEngine.yaml").getPath();
        String result = PropertiesUtil.getConfValue(
            path.replace("pm-system-base-pod-CyberEngine.yaml", StringUtils.EMPTY),
            "pm-system-base-pod-CyberEngine.yaml", "serviceName");

        // verify the results
        Assert.assertEquals("protectmanager-system-base", result);
    }

    /**
     * 用例场景：查询挂载configMap指定key的value值
     * 前置条件：无相关配置文件
     * 检查点：无相关配置文件时返回“”
     */
    @Test
    @Ignore
    public void test_file_to_string() throws FileNotFoundException {
        String path = ResourceUtils.getFile("classpath:pm-system-base-pod-CyberEngine.yaml").getPath();
        String value = PropertiesUtil.fileToString(path);
        Assert.assertEquals(value, "");
    }
}
