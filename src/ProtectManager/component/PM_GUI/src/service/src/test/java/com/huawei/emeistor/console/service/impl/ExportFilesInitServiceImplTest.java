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
package com.huawei.emeistor.console.service.impl;

import static org.mockito.ArgumentMatchers.anyString;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RList;
import org.redisson.api.RedissonClient;

/**
 * ExportFilesInitServiceImpl Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {ExportFilesInitServiceImpl.class})
public class ExportFilesInitServiceImplTest {
    @Mock
    protected RedissonClient redissonClient;

    @InjectMocks
    private ExportFilesInitServiceImpl exportFilesInitService;

    @Test
    public void should_clear_read_export_file_list() {
        RList readFileIdList = PowerMockito.mock(RList.class);
        PowerMockito.when(redissonClient.getList(anyString())).thenReturn(readFileIdList);
        exportFilesInitService.run();
    }
}
