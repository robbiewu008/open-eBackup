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

import openbackup.data.access.framework.livemount.controller.livemount.LiveMountPolicyInternalController;
import openbackup.data.access.framework.livemount.service.PolicyService;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doNothing;

@RunWith(SpringRunner.class)
@PrepareForTest(LiveMountPolicyInternalController.class)
@SpringBootTest(classes = LiveMountPolicyInternalController.class)
public class LiveMountPolicyInternalControllerTest {

    @Autowired
    LiveMountPolicyInternalController liveMountPolicyInternalController;

    @MockBean
    PolicyService policyService;

    @Test
    public void revokeLiveMountPolicyUserId_empty_return(){
        String userId = "";
        doNothing().when(policyService).revokeLiveMountPolicyUserId(anyString());
        liveMountPolicyInternalController.revokeLiveMountPolicyUserId(userId);
        Assert.assertNotNull(liveMountPolicyInternalController);

    }

    @Test
    public void revokeLiveMountPolicyUserId_success(){
        String userId = "yang";
        doNothing().when(policyService).revokeLiveMountPolicyUserId(anyString());
        liveMountPolicyInternalController.revokeLiveMountPolicyUserId(userId);
        Assert.assertNotNull(liveMountPolicyInternalController);
    }
}
