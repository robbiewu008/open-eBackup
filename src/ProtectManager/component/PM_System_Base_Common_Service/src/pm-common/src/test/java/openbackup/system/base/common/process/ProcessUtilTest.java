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
package openbackup.system.base.common.process;

import openbackup.system.base.common.os.OsTypeHelper;
import openbackup.system.base.common.os.OsTypeUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.TimeUnit;

/**
 * 测试ProcessUtil
 *
 * @author w00493811
 * @since 2021-08-17
 */
@RunWith(SpringRunner.class)
public class ProcessUtilTest {
    @Test
    public void test_execute_in_seconds_success() throws ProcessException {
        ProcessResult processResult = new ProcessResult();
        OsTypeHelper.modifyOsTypeUtilOsName(System.getProperty("os.name").toLowerCase(Locale.ENGLISH));
        if (OsTypeUtil.isWindows()) {
            List<String> command = new ArrayList<>();
            command.add("cmd.exe");
            command.add("/c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            processResult = ProcessUtil.executeInSeconds(command, 5);
            System.out.println("--------");
            System.out.println(processResult);
            System.out.println("--------");
        } else if (OsTypeUtil.isLinux()) {
            List<String> command = new ArrayList<>();
            command.add("bash");
            command.add("-c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            processResult = ProcessUtil.executeInSeconds(command, 5);
            System.out.println("--------");
            System.out.println(processResult);
            System.out.println("--------");
        }
        Assert.assertTrue(processResult.isOk());
        Assert.assertFalse(processResult.isNok());
        Assert.assertTrue(processResult.getErrorsList().size() == 0);
        Assert.assertTrue(processResult.getOutputList().size() != 0);
    }

    /**
     * 用例场景：命令正确，命令参数单独输入
     * 前置条件: Liunx环境
     * 检查点：执行结果正确
     */
    @Test
    public void test_process_separate_args() throws Exception {
        ProcessResult processResult = new ProcessResult();
        OsTypeHelper.modifyOsTypeUtilOsName(System.getProperty("os.name").toLowerCase(Locale.ENGLISH));
        if (OsTypeUtil.isLinux()) {
            List<String> args = new ArrayList<>();
            args.add("-lha");
            args.add(System.getProperty("user.home"));
            processResult = ProcessUtil.execute("ls", args, 5, TimeUnit.SECONDS);
            Assert.assertTrue(processResult.isOk());
            Assert.assertFalse(processResult.isNok());
            Assert.assertEquals(0, processResult.getErrorsList().size());
            Assert.assertTrue(processResult.getOutputList().size() != 0);
        }
    }

    /**
     * 用例场景：命令错误,执行失败
     * 前置条件：发送错误的命令
     * 检查点：抛出ProcessException
     */
    @Test(expected = ProcessException.class)
    public void test_execute_in_seconds_exception() throws ProcessException {
        if (OsTypeUtil.isWindows()) {
            List<String> command = new ArrayList<>();
            command.add("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            command.add("/c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            ProcessUtil.executeInSeconds(command, 5);
        } else if (OsTypeUtil.isLinux()) {
            List<String> command = new ArrayList<>();
            command.add("ttttttttttttttttttttttttttttttttttt");
            command.add("-c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            ProcessUtil.executeInSeconds(command, 5);
        }
    }

    @Test
    public void test_execute_in_seconds_failure() {
        ProcessResult processResult = new ProcessResult();
        if (OsTypeUtil.isWindows()) {
            List<String> command = new ArrayList<>();
            command.add("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            command.add("/c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            try {
                processResult = ProcessUtil.executeInSeconds(command, 5);
            } catch (ProcessException e) {
                e.printStackTrace();
            }
            System.out.println("--------");
            System.out.println(processResult);
            System.out.println("--------");
            Assert.assertFalse(processResult.isOk());
            Assert.assertTrue(processResult.isNok());
            Assert.assertTrue(processResult.getErrorsList().size() == 0);
            Assert.assertTrue(processResult.getOutputList().size() == 0);
        } else if (OsTypeUtil.isLinux()) {
            List<String> command = new ArrayList<>();
            command.add("ttttttttttttttttttttttttttttttttttt");
            command.add("-c");
            command.add("dir");
            command.add(System.getProperty("user.home"));
            try {
                processResult = ProcessUtil.executeInSeconds(command, 5);
            } catch (ProcessException e) {
                e.printStackTrace();
            }
            System.out.println("--------");
            System.out.println(processResult);
            System.out.println("--------");
            Assert.assertFalse(processResult.isOk());
            Assert.assertTrue(processResult.isNok());
            Assert.assertTrue(processResult.getErrorsList().size() == 0);
            Assert.assertTrue(processResult.getOutputList().size() == 0);
        }
    }
}
