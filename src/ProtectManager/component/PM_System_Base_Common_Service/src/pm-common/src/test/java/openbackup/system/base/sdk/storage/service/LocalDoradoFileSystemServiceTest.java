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
package openbackup.system.base.sdk.storage.service;

import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;
import openbackup.system.base.sdk.storage.StorageService;
import openbackup.system.base.sdk.storage.api.LocalDoradoRestApi;
import openbackup.system.base.sdk.storage.model.DoradoResponse;
import openbackup.system.base.sdk.storage.model.FileSystemScrubResponse;
import openbackup.system.base.sdk.storage.service.LocalDoradoFileSystemService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: LocalDoradoFileSystemServiceTest
 *
 */
public class LocalDoradoFileSystemServiceTest {
    private static final String DEVICE_ID = "Test-DeviceId";

    private static LocalDoradoRestApi localDoradoRestApi;

    private static StorageService storageService;

    private static LocalDoradoFileSystemService fileSystemService;

    @BeforeClass
    public static void init() {
        localDoradoRestApi = Mockito.mock(LocalDoradoRestApi.class);
        storageService = Mockito.mock(StorageService.class);
        fileSystemService = new LocalDoradoFileSystemService(localDoradoRestApi, storageService);
    }

    @Before
    public void setUp() {
        StorageResponse<StorageSession> response = new StorageResponse<>();
        StorageSession storageSession = new StorageSession();
        storageSession.setDeviceid(DEVICE_ID);
        response.setData(storageSession);
        Mockito.when(storageService.getStorageSession()).thenReturn(response);
    }

    /**
     * 用例名称：校验文件系统数据完整性成功
     * 前置条件：无
     * 检查点：文件系统数据一致性是否为一致
     */
    @Test
    public void test_check_file_system_consistent_status_success() {
        DoradoResponse<FileSystemScrubResponse> response = new DoradoResponse<>();
        response.setData(mockScrubResponse("1"));
        Mockito.when(localDoradoRestApi.queryFileSystemScrub(DEVICE_ID, "test-fs-id-1")).thenReturn(response);

        ConsistentStatusEnum consistentStatus = fileSystemService.checkFsConsistentStatus("test-fs-id-1");
        Assert.assertEquals(ConsistentStatusEnum.CONSISTENT, consistentStatus);
    }

    /**
     * 用例名称：重复开启或关闭文件系统扫描任务时，校验文件系统数据完整性成功
     * 前置条件：无
     * 检查点：文件系统数据一致性是否为一致
     */
    @Test
    public void test_check_file_system_consistent_status_success_when_scrub_exist() {
        LegoCheckedException exception = new LegoCheckedException(1073844240L, "Scrub already exist");
        Mockito.when(localDoradoRestApi.changeFileSystemScrub(any(), any())).thenThrow(exception);

        DoradoResponse<FileSystemScrubResponse> response = new DoradoResponse<>();
        response.setData(mockScrubResponse("1"));
        Mockito.when(localDoradoRestApi.queryFileSystemScrub(DEVICE_ID, "test-fs-id-2")).thenReturn(response);

        ConsistentStatusEnum consistentStatus = fileSystemService.checkFsConsistentStatus("test-fs-id-2");
        Assert.assertEquals(ConsistentStatusEnum.CONSISTENT, consistentStatus);
    }

    /**
     * 用例名称：校验文件系统数据完整性失败
     * 前置条件：无
     * 检查点：文件系统数据一致性是否为不一致
     */
    @Test
    public void test_check_file_system_consistent_status_failed() {
        DoradoResponse<FileSystemScrubResponse> response = new DoradoResponse<>();
        response.setData(mockScrubResponse("1", 1, 0));
        Mockito.when(localDoradoRestApi.queryFileSystemScrub(DEVICE_ID, "test-id-3")).thenReturn(response);
        Assert.assertEquals(ConsistentStatusEnum.INCONSISTENT, fileSystemService.checkFsConsistentStatus("test-id-3"));

        response.setData(mockScrubResponse("3"));
        Assert.assertEquals(ConsistentStatusEnum.INCONSISTENT, fileSystemService.checkFsConsistentStatus("test-id-3"));

        LegoCheckedException exception = new LegoCheckedException(1073844234L, "Scrub not exist");
        Mockito.when(localDoradoRestApi.queryFileSystemScrub(DEVICE_ID, "test-id-4")).thenThrow(exception);
        Assert.assertEquals(ConsistentStatusEnum.INCONSISTENT, fileSystemService.checkFsConsistentStatus("test-id-4"));
    }

    private FileSystemScrubResponse mockScrubResponse(String runningStatus) {
        return mockScrubResponse(runningStatus, 0, 0);
    }

    private FileSystemScrubResponse mockScrubResponse(String runningStatus, long mediumErrors, long otherErrors) {
        FileSystemScrubResponse response = new FileSystemScrubResponse();
        response.setRunningStatus(runningStatus);
        response.setMediumErrors(mediumErrors);
        response.setOtherErrors(otherErrors);
        return response;
    }
}