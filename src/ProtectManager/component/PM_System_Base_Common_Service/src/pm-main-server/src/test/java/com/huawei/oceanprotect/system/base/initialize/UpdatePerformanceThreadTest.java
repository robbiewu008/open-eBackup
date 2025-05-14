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
package com.huawei.oceanprotect.system.base.initialize;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @since 2024-01-20
 */

@RunWith(PowerMockRunner.class)
public class UpdatePerformanceThreadTest {

    @InjectMocks
    UpdatePerformanceThread updatePerformanceThread;

    @Mock
    ClusterStorageService clusterStorageService;

    @Test
    public void test_update_performance_thread(){
        new UpdatePerformanceThread(null);
        Assert.assertTrue(true);
    }

    @Test
    public void test_run(){
        PowerMockito.doNothing().when(clusterStorageService).updatePerformance(true,true);
        updatePerformanceThread.run();
        Assert.assertTrue(true);
    }
}
