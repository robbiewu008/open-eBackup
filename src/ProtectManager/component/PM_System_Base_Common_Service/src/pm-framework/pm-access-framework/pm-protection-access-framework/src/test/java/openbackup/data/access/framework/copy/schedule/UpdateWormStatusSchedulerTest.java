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
package openbackup.data.access.framework.copy.schedule;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.dao.CopyMapper;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述 UpdateWormStatusScheduler Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {UpdateWormStatusScheduler.class})
public class UpdateWormStatusSchedulerTest {
    @InjectMocks
    private UpdateWormStatusScheduler updateWormStatusScheduler;

    @Mock
    private CopyMapper copyMapper;

    @Test
    public void test_monitor_worm_expired_status_should_success() {
        updateWormStatusScheduler.monitor();
        Mockito.verify(copyMapper, Mockito.times(1)).updateWormCopyExpiredStatus(any());
    }
}
