/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author z00633516
 * @since 2022-03-17
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
