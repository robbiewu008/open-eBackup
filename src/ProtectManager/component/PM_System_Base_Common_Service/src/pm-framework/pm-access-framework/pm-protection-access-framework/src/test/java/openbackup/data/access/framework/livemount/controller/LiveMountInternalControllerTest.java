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
package openbackup.data.access.framework.livemount.controller;

import openbackup.data.access.framework.livemount.controller.livemount.LiveMountInternalController;
import openbackup.data.access.framework.livemount.service.LiveMountService;

import openbackup.system.base.sdk.job.model.JobBo;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;

import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doNothing;

@RunWith(SpringRunner.class)
@PrepareForTest(LiveMountInternalController.class)
@SpringBootTest(classes = LiveMountInternalController.class)
public class LiveMountInternalControllerTest {


    @MockBean
    private LiveMountService liveMountService;
    @Resource
    LiveMountInternalController liveMountInternalController;

    @Test
    public void cancelJob_success(){
        JobBo jobBo = new JobBo();
        doNothing().when(liveMountService).cancelLiveMount(anyString());
        liveMountInternalController.cancelJob(jobBo);
        Assert.assertNotNull(liveMountInternalController);

    }

    @Test
    public void revokeLiveMountUserId_empty_return() {
        String userId = "";
        doNothing().when(liveMountService).revokeLiveMountUserId(anyString());
        liveMountInternalController.revokeLiveMountUserId(userId);
        Assert.assertNotNull(liveMountInternalController);
    }

    @Test
    public void revokeLiveMountUserId_success() {
        String userId = "yang";
        doNothing().when(liveMountService).revokeLiveMountUserId(anyString());
        liveMountInternalController.revokeLiveMountUserId(userId);
        Assert.assertNotNull(liveMountInternalController);
    }

}
