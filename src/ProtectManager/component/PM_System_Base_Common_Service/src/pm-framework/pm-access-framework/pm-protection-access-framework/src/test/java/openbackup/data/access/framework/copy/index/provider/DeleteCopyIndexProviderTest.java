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
package openbackup.data.access.framework.copy.index.provider;

import openbackup.data.access.framework.copy.index.provider.DeleteCopyIndexProvider;
import openbackup.data.access.framework.copy.mng.handler.v1.CopyDeleteCompleteHandler;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;

/**
 * DeleteCopyIndex LLT
 *
 * @author zwx1010134
 * @since 2021-07-01
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyDeleteCompleteHandler.class)
@AutoConfigureMockMvc
public class DeleteCopyIndexProviderTest {
    @Mock
    private NotifyManager notifyManager;

    @Mock
    private CopyRestApi copyRestApi;

    @InjectMocks
    private DeleteCopyIndexProvider deleteCopyIndexProvider;

    @Test
    public void testSendDeleteCopyIndexMessage() {
        String requestId = "123";
        String copyId = "321";
        Copy copy = new Copy();
        copy.setResourceSubType("vim.VirtualMachine");
        copy.setIndexed("Indexed");
        copy.setChainId(UUID.randomUUID().toString());
        copy.setGn(0);
        copy.setPrevCopyGn(-1);
        copy.setNextCopyGn(-1);
        copy.setDeviceEsn("123");
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);
        PowerMockito.when(
                copyRestApi.queryCopies(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(mockCopies());
        deleteCopyIndexProvider.sendDeleteCopyIndexMessage(requestId, copyId);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }

    private BasePage<Copy> mockCopies(){
        BasePage<Copy> page = new BasePage<>();
        Copy copy = new Copy();
        copy.setResourceSubType("vim.VirtualMachine");
        copy.setIndexed("Indexed");
        copy.setChainId(UUID.randomUUID().toString());
        copy.setGn(0);
        copy.setPrevCopyGn(-1);
        copy.setNextCopyGn(-1);
        copy.setDeviceEsn("123");
        List<Copy> copies = new ArrayList<>();
        copies.add(copy);
        page.setItems(copies);
        return page;
    }
}