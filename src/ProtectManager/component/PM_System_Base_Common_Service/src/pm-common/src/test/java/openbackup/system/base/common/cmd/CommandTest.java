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
package openbackup.system.base.common.cmd;

import openbackup.system.base.common.cmd.Command;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * Command Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(Command.class)
public class CommandTest {
    @Rule
    public final ExpectedException exception = ExpectedException.none();

    /**
     * 用例场景：涉及敏感信息的参数传递，通过process的输出流来传参，但是敏感信息null，则抛错
     * 前置条件：敏感信息为空
     * 检查点: 是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_sensitive_param_is_null_if_run_with_sensitive_params() {
        exception.expect(LegoCheckedException.class);
        exception.expectMessage("sensitive params is null.");
        int code = Command.runWithSensitiveParams(null, "sh", "xxx.sh", "-passin", "SensitiveParams");
        Assert.assertEquals(0, code);
    }

    /**
     * 用例场景：涉及敏感信息的参数传递，通过process的输出流来传参，但是敏感信息没有，则抛错
     * 前置条件：敏感信息size为0
     * 检查点: 是否报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_sensitive_param_size_is_zero_if_run_with_sensitive_params() {
        exception.expect(LegoCheckedException.class);
        exception.expectMessage("sensitive params is null.");
        String[] sensitiveParams = {};
        int code = Command.runWithSensitiveParams(sensitiveParams, "sh", "xxx.sh", "-passin", "SensitiveParams");
        Assert.assertEquals(0, code);
    }

    /**
     * 用例场景：涉及非敏感信息的参数传递，当输入中含有任意1个参数黑名单，抛出异常。
     * 前置条件：输入参数含有非法字符\
     * 检查点：抛出非法参数异常。
     */
    @Test
    public void should_throw_LegoCheckedException_if_items_is_in_black_list_when_run() {
        exception.expect(LegoCheckedException.class);
        exception.expectMessage("Illegal param!");
        String[] sensitiveParams = {"pwd"};
        Command.runWithSensitiveParams(sensitiveParams, "sh", "abc.sh", "\\cc", "cdh");
    }

    /**
     * 用例场景：输入参数不含参数黑名单情况，不抛出异常。
     * 前置条件：输入参数无非法字符
     * 检查点：不抛出异常。
     */
    @Test
    @Ignore
    public void run_command_success() {
        Command.run("openssl", "pkcs8", "-topk8"
                , "-in", "/opt/OceanProtect/protectmanager/cert/pm.store.p12.cnf", "-nocrypt"
                , "-out", "/opt/OceanProtect/protectmanager/cert/external/EXTERNAL_STORAGE/Huawei_IT_Product.crt.pem");
    }
}
