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
package openbackup.oracle.service;

import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.oracle.constants.ScnCopy;
import openbackup.oracle.service.impl.OracleCopyServiceImpl;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;

public class OracleCopyServiceTest {
    private final DmeUnifiedRestApi dmeUnifiedRestApi = Mockito.mock(DmeUnifiedRestApi.class);
    private final OracleCopyService oracleCopyService = new OracleCopyServiceImpl(dmeUnifiedRestApi);

    /**
     * 用例场景：根据scn获取副本信息
     * 前置条件：1. 存在符合条件的副本
     * 检 查 点：1. 获取到的副本id符合预期
     */
    @Test
    public void get_scn_copy_success() {
        Mockito.when(dmeUnifiedRestApi.listCopiesInfo(any(), any(), any(), any())).thenReturn(mockCopies());
        List<ScnCopy> scnCopies = oracleCopyService.listCopiesInfo("789", "12345");
        Assert.assertEquals(scnCopies.get(0).getId(), "123");
    }

    private List<DmeCopyInfo> mockCopies() {
        List<DmeCopyInfo> copies = new ArrayList<>();
        DmeCopyInfo copyInfo = new DmeCopyInfo();
        copyInfo.setId("123");
        copyInfo.setTimestamp(168235650);
        copies.add(copyInfo);
        return copies;
    }
}