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

import openbackup.system.base.common.utils.files.FileZip;
import openbackup.system.base.util.ClusterFileUtils;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * ClusterFileUtils工具类测试类
 *
 * @author y30046482
 * @since 2023-05-17
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({FileZip.class, FileUtils.class, ClusterFileUtils.class})
public class ClusterFileUtilsTest {

    @Before
    public void mockStaticClass() {
        PowerMockito.mockStatic(FileZip.class);
        PowerMockito.mockStatic(FileUtils.class);
        PowerMockito.mockStatic(ClusterFileUtils.class);
    }

    @Test
    public void test_createMultipartFile() {
        ClusterFileUtils.createMultipartFile("");
        Assert.assertTrue(true);
    }

    @Test
    public void test_deleteFile() {
        ClusterFileUtils.deleteFile("");
        Assert.assertTrue(true);
    }
}