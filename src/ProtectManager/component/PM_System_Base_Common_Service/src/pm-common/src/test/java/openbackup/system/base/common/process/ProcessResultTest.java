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

import openbackup.system.base.common.process.ProcessResult;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 测试ProcessResult
 *
 * @author w00493811
 * @since 2021-08-17
 */
@RunWith(SpringRunner.class)
public class ProcessResultTest {
    @Test
    public void test_init() {
        ProcessResult processResult = new ProcessResult();
        Assert.assertFalse(processResult.isOk());
        Assert.assertTrue(processResult.isNok());
        Assert.assertTrue(processResult.getErrorsList().size() == 0);
        Assert.assertTrue(processResult.getOutputList().size() == 0);
    }

    @Test
    public void test_add_max_errors() {
        ProcessResult processResult = new ProcessResult();
        Assert.assertFalse(processResult.isOk());
        Assert.assertTrue(processResult.isNok());
        Assert.assertTrue(processResult.getErrorsList().size() == 0);
        Assert.assertTrue(processResult.getOutputList().size() == 0);
        for (int i = 0; i < ProcessResult.CACHE_MAX_SIZE; i++) {
            processResult.addErrors(String.valueOf(i));
            Assert.assertTrue(processResult.getErrorsList().size() == i + 1);
        }
        for (int i = 0; i < 10; i++) {
            processResult.addErrors(String.valueOf(i));
            Assert.assertTrue(processResult.getErrorsList().size() == ProcessResult.CACHE_MAX_SIZE);
        }
    }

    @Test
    public void test_add_max_output() {
        ProcessResult processResult = new ProcessResult();
        Assert.assertFalse(processResult.isOk());
        Assert.assertTrue(processResult.isNok());
        Assert.assertTrue(processResult.getErrorsList().size() == 0);
        Assert.assertTrue(processResult.getOutputList().size() == 0);
        for (int i = 0; i < ProcessResult.CACHE_MAX_SIZE; i++) {
            processResult.addOutput(String.valueOf(i));
            Assert.assertTrue(processResult.getOutputList().size() == i + 1);
        }
        for (int i = 0; i < 10; i++) {
            processResult.addOutput(String.valueOf(i));
            Assert.assertTrue(processResult.getOutputList().size() == ProcessResult.CACHE_MAX_SIZE);
        }
    }
}
