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

import openbackup.system.base.util.AdapterUtils;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.net.URL;
import java.util.List;

/**
 * The AdapterUtilsTest
 *
 */
class AdapterUtilsTest {
    /**
     * 用例场景：测试获取对象的Classloader；测试在输入为null情况下的classloader返回情况
     * 前置条件：无
     * 检 查 点：都能够获取到ClassLoader对象，不为空
     */
    @Test
    public void get_classloader_success() {
        ClassLoader cl = AdapterUtils.getClassLoader(Object.class);
        Assertions.assertNotNull(cl);

        ClassLoader cl1 = AdapterUtils.getClassLoader(null);
        Assertions.assertNotNull(cl1);
    }

    /**
     * 用例场景：AdapterUtils中的各种获取资源的方法，看是否能够返回相应的信息
     * 前置条件：需要将代码所需的json文件放到classpath中
     * 检 查 点：所有相关函数都能够获取到相应的数据，不为空
     */
    @Test
    public void test_classpath_utils_success() {
        List<URL> allClassPathEntries =
                AdapterUtils.getAllClassPathEntries(
                        "conf/alarmI18nE/*.json");
        Assertions.assertNotNull(allClassPathEntries);

        List<URL> allClassPathAlarmDefineUrls =
                AdapterUtils.getAllClassPathAlarmDefineUrls();
        Assertions.assertNotNull(allClassPathAlarmDefineUrls);

        List<URL> allClassPathEntriesAlarmEn =
                AdapterUtils.getAllClassPathEntriesAlarmEn();
        Assertions.assertNotNull(allClassPathEntriesAlarmEn);

        List<URL> allOperationTargetClassPathEntriesAlarmEn =
                AdapterUtils.getAllOperationTargetClassPathEntriesAlarmEn();
        Assertions.assertNotNull(allOperationTargetClassPathEntriesAlarmEn);

        List<URL> anyBackUpClassPathEntriesAlarmEn =
                AdapterUtils.getAnyBackUpClassPathEntriesAlarmEn();
        Assertions.assertNotNull(anyBackUpClassPathEntriesAlarmEn);

        List<URL> allClassPathEntriesAlarmZh =
                AdapterUtils.getAllClassPathEntriesAlarmZh();
        Assertions.assertNotNull(allClassPathEntriesAlarmZh);

        List<URL> secondaryParamInternalEn =
                AdapterUtils.getSecondaryParamInternalEn();
        Assertions.assertNotNull(secondaryParamInternalEn);

        List<URL> secondaryParamInternalZh =
                AdapterUtils.getSecondaryParamInternalZh();
        Assertions.assertNotNull(secondaryParamInternalZh);

        List<URL> allOperationTargetClassPathEntriesAlarmZh =
                AdapterUtils.getAllOperationTargetClassPathEntriesAlarmZh();
        Assertions.assertNotNull(allOperationTargetClassPathEntriesAlarmZh);

        List<URL> anyBackUpClassPathEntriesAlarmZh =
                AdapterUtils.getAnyBackUpClassPathEntriesAlarmZh();
        Assertions.assertNotNull(anyBackUpClassPathEntriesAlarmZh);
    }
}