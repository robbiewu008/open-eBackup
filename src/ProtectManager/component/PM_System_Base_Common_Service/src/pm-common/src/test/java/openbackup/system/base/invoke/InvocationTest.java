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
package openbackup.system.base.invoke;

import openbackup.system.base.invoke.Invocation;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.function.Function;

public class InvocationTest {
    /**
     * 用例名称：验证业务方法的拦截链方法执行顺序是否正确。
     * 前置条件：业务方法及拦截链方法就绪
     * check点：业务方法的拦截链方法执行顺序正确
     */
    @Test
    public void test_invoke() {
        String result0 =
            new Invocation<>(Integer::toHexString, Arrays.asList((invocation1, param) -> invocation1.invoke(param + 1),
                (invocation1, param) -> invocation1.invoke(param << 1))).invoke(0x8);
        Assert.assertEquals("12", result0);

        String result1 = new Invocation<String, String>(Function.identity(),
            Arrays.asList((invocation1, param) -> invocation1.invoke(param + "1"),
                (invocation1, param) -> invocation1.invoke("2" + param))).invoke("-");
        Assert.assertEquals("2-1", result1);

        String result2 = new Invocation<String, String>(Function.identity(),
            Arrays.asList((invocation1, param) -> invocation1.invoke(param + "1"),
                (invocation1, param) -> invocation1.invoke(param + "2"))).invoke("-");
        Assert.assertEquals("-12", result2);
    }
}
