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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.files.FileZip;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.util.ResourceUtils;

import java.io.File;
import java.io.FileNotFoundException;

/**
 * 文件压缩测试类
 *
 * @author w00504341
 * @since 2023-03-25
 */
public class FileZipTest {
    /**
     * 用例场景：压缩文件
     * 前置条件：文件存在
     * 检查点：压缩成功
     */
    @Test
    public void test_zip_success() throws FileNotFoundException {
        String filePath = StringUtils.join(new String[]{"classpath:files", "test1"}, File.separator);
        String tmpPath = ResourceUtils.getFile(filePath).getPath();
        File file1 = new File(tmpPath);
        File parent = file1.getParentFile();
        String zipFilePath = parent.getPath();
        String baseName = "test1.zip";
        FileZip.zip(tmpPath, zipFilePath + File.separator + baseName, baseName);
        File rsFile = new File(zipFilePath + File.separator + baseName);
        Assert.assertTrue(rsFile.exists());
        rsFile.delete();
    }
}
