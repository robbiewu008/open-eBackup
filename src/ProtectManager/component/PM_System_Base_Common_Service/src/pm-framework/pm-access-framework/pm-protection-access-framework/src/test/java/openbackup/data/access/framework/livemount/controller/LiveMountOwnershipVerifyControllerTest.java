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

import openbackup.data.access.framework.livemount.controller.livemount.LiveMountOwnershipVerifyController;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;

import openbackup.system.base.common.exception.LegoCheckedException;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;

/**
 * Live Mount Controller Test
 *
 * @author fwx1022842
 * @since 2021-03-09
 */
@RunWith(SpringRunner.class)
@PrepareForTest(LiveMountOwnershipVerifyController.class)
@SpringBootTest
@ContextConfiguration(classes = {LiveMountOwnershipVerifyController.class})
public class LiveMountOwnershipVerifyControllerTest {
    /**
     * ExpectedException
     */
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @Autowired
    private LiveMountOwnershipVerifyController liveMountOwnershipVerifyController;

    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    /**
     * test verifyLiveMountOwnership uuid list is null
     */
    @Test
    public void verifyLiveMountOwnershipUuidListIsNull() {
        liveMountOwnershipVerifyController.verifyLiveMountOwnership("1", null);
    }

    /**
     * test verifyLiveMountOwnership size same
     */
    @Test
    public void verifyLiveMountOwnershipSizeSame() {
        List<String> uuidList = Arrays.asList("1", "2");
        PowerMockito.when(liveMountEntityDao.selectCount(any())).thenReturn(Long.valueOf(uuidList.size()));
        liveMountOwnershipVerifyController.verifyLiveMountOwnership("1", uuidList);
    }

    /**
     * test verifyLiveMountOwnership but size not same
     */
    @Test
    public void verifyLiveMountOwnershipSizeNotSame() {
        expectedException.expect(LegoCheckedException.class);
        List<String> uuidList = Arrays.asList("1", "2");
        PowerMockito.when(liveMountEntityDao.selectCount(any())).thenReturn(Long.valueOf(uuidList.size() + 1));
        liveMountOwnershipVerifyController.verifyLiveMountOwnership("1", uuidList);
    }

    /**
     * test verifyLiveMountPolicyOwnership uuid list is null
     */
    @Test
    public void verifyLiveMountPolicyOwnershipUuidListIsNull() {
        liveMountOwnershipVerifyController.verifyLiveMountPolicyOwnership("1", null);
    }

    /**
     * test verifyLiveMountPolicyOwnership size same
     */
    @Test
    public void verifyLiveMountPolicyOwnershipSizeSame() {
        List<String> uuidList = Arrays.asList("1", "2");
        PowerMockito.when(liveMountPolicyEntityDao.selectCount(any())).thenReturn(Long.valueOf(uuidList.size()));
        liveMountOwnershipVerifyController.verifyLiveMountPolicyOwnership("1", uuidList);
    }

    /**
     * test verifyLiveMountPolicyOwnership but size not same
     */
    @Test
    public void verifyLiveMountPolicyOwnershipSizeNotSame() {
        expectedException.expect(LegoCheckedException.class);
        List<String> uuidList = Arrays.asList("1", "2");
        PowerMockito.when(liveMountPolicyEntityDao.selectCount(any())).thenReturn(Long.valueOf(uuidList.size() + 1));
        liveMountOwnershipVerifyController.verifyLiveMountPolicyOwnership("1", uuidList);
    }
}
