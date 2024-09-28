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

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.FileCheckUtils;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.springframework.util.ResourceUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

/**
 * FileCheckUtils 测试类
 *
 */
public class FileCheckUtilsTest {

    @Test
    @Ignore
    public void test_check_file_nums_and_sign() throws IOException {
        File file = ResourceUtils.getFile("classpath:cms");
        Set<String> signFilePath = new HashSet<>();
        signFilePath.add(file.getCanonicalPath() + File.separator + "sha256sum_sync");
        signFilePath.add(file.getCanonicalPath() + File.separator + "sha256sum_sync_wrong");
        FileCheckUtils.checkFileNumsAndSign(file.getCanonicalPath(), signFilePath, "sha256sum_sync");
    }

    @Test
    public void test_wrong_check_file_nums_and_sign() throws IOException {
        File file = ResourceUtils.getFile("classpath:cms");
        Set<String> signFilePath = new HashSet<>();
        signFilePath.add(file.getCanonicalPath() + File.separator + "sha256sum_sync");
        signFilePath.add(file.getCanonicalPath() + File.separator + "sha256sum_sync_wrong");
        Assert.assertThrows(LegoCheckedException.class, () -> {
            FileCheckUtils.checkFileNumsAndSign(file.getCanonicalPath(), signFilePath, "sha256sum_sync_wrong");
        });
    }

    @Test
    public void test_wrong_num_check_file_nums_and_sign() throws IOException {
        File file = ResourceUtils.getFile("classpath:cms");
        Set<String> signFilePath = new HashSet<>();
        signFilePath.add(file.getCanonicalPath() + File.separator + "sha256sum_sync_wrong");
        Assert.assertThrows(LegoCheckedException.class, () -> {
            FileCheckUtils.checkFileNumsAndSign(file.getCanonicalPath(), signFilePath, "sha256sum_sync_wrong");
        });
    }
}
