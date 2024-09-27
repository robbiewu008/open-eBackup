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
package openbackup.data.access.framework.copy.controller;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doNothing;

import openbackup.data.access.framework.copy.controller.CopyInternalController;
import openbackup.data.access.framework.copy.controller.req.CopyVerifyRequest;
import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.protection.controller.v2.req.DownloadFilesReq;
import openbackup.data.access.framework.protection.controller.v2.resp.DownLoadResp;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.common.model.UuidObject;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;

@RunWith(SpringRunner.class)
@PrepareForTest(CopyInternalController.class)
@SpringBootTest(classes = CopyInternalController.class)
public class CopyInternalControllerTest {

    @Autowired
    CopyInternalController copyInternalController;

    @MockBean
    CopyService copyService;

    @MockBean
    UnifiedCopyIndexService unifiedCopyIndexService;

    @MockBean
    CopyVerifyTaskManager copyVerifyTaskManager;

    @Test
    public void downloadFiles_success() {
        String copyId = UUIDGenerator.getUUID();
        DownloadFilesReq downloadFilesReq = new DownloadFilesReq();
        Mockito.when(copyService.downloadFiles(any(),any(),any())).thenReturn(UUIDGenerator.getUUID());
        DownLoadResp downLoadResp = copyInternalController.downloadFiles(copyId, downloadFilesReq);
        assertThat(downLoadResp).isExactlyInstanceOf(DownLoadResp.class);

    }

    @Test
    public void deleteResourceIndex_success() {
        String resourceId = UUIDGenerator.getUUID();
        doNothing().when(unifiedCopyIndexService).deleteResourceIndexTask(anyString(), anyString());
        copyInternalController.deleteResourceIndex(resourceId, "123");
        Mockito.verify(unifiedCopyIndexService, Mockito.times(1)).deleteResourceIndexTask(anyString(), anyString());
    }

    @Test
    public void internalVerifyCopy_success() {
        String copyId = UUIDGenerator.getUUID();
        String userId = UUIDGenerator.getUUID();
        CopyVerifyRequest copyVerifyRequest = new CopyVerifyRequest();
        copyVerifyRequest.setUserId(userId);
        MockHttpServletRequest request = new MockHttpServletRequest();
        Mockito.when(copyVerifyTaskManager.init(any(), any())).thenReturn(UUIDGenerator.getUUID());
        UuidObject uuidObject = copyInternalController.internalVerifyCopy(copyId, copyVerifyRequest, request);
        assertThat(uuidObject).isExactlyInstanceOf(UuidObject.class);
    }
}
