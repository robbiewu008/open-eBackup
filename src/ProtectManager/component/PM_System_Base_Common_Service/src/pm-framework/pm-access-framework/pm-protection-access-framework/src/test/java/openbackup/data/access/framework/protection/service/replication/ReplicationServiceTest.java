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
package openbackup.data.access.framework.protection.service.replication;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.times;

import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.system.base.sdk.anti.api.AntiRansomwareApi;
import openbackup.system.base.sdk.anti.model.AntiRansomwarePolicyRes;
import openbackup.system.base.sdk.anti.model.AntiRansomwareScheduleRes;
import openbackup.system.base.sdk.license.LicenseServiceApi;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.Date;

/**
 * ReplicationService LLT
 *
 * @since 2023-04-01
 */
@RunWith(PowerMockRunner.class)
@AutoConfigureMockMvc
public class ReplicationServiceTest {
    @Mock
    private LicenseServiceApi licenseServiceApi;

    @Mock
    private AntiRansomwareApi antiRansomwareApi;

    @InjectMocks
    private ReplicationService replicationService;

    /**
     * 用例场景：校验License
     * 前置条件：无
     * 检查点：1.复制从端未开启防勒索检测，不校验License；2.原副本不是worm，复制副本开启设置worm，开启防勒索，需要校验License；
     *       3.原副本是worm，复制副本未开启设置worm，开启防勒索，需要校验License；
     */
    @Test
    public void test_checkDetectionLicense() {
        AntiRansomwarePolicyRes antiRansomwarePolicyRes = getAntiRansomwarePolicyRes();
        replicationService.checkDetectionLicense(false, antiRansomwarePolicyRes.getSchedule());
        Mockito.verify(licenseServiceApi, times(0)).functionLicense(any(), any());
        antiRansomwarePolicyRes.getSchedule().setNeedDetect(true);
        replicationService.checkDetectionLicense(false, antiRansomwarePolicyRes.getSchedule());
        Mockito.verify(licenseServiceApi, times(1)).functionLicense(any(), any());
        antiRansomwarePolicyRes.getSchedule().setNeedDetect(false);
        antiRansomwarePolicyRes.getSchedule().setSetWorm(false);
        replicationService.checkDetectionLicense(true, antiRansomwarePolicyRes.getSchedule());
        Mockito.verify(licenseServiceApi, times(1)).functionLicense(any(), any());
    }

    @Test
    public void test_getGenerateTime() {
        Date generateTime = replicationService.getGenerateTime(2534022719990L);
        Assert.assertEquals(generateTime, new Date(System.currentTimeMillis()));
    }

    /**
     * 用例场景：查询是否配置设置防勒索&worm策略
     * 前置条件：无
     * 检查点：返回防勒索&worm策略
     */
    @Test
    public void test_getAntiRansomwareSchedule() {
        AntiRansomwarePolicyRes antiRansomwarePolicyRes = getAntiRansomwarePolicyRes();
        PowerMockito.when(antiRansomwareApi.getPolicyByResourceId(any())).thenReturn(antiRansomwarePolicyRes);
        Assert.assertNotNull(replicationService.getAntiRansomwareSchedule("803918aa-622f-40af-a5b5-7951b0d90f15"));
    }

    private AntiRansomwarePolicyRes getAntiRansomwarePolicyRes() {
        AntiRansomwarePolicyRes antiRansomwarePolicyRes = new AntiRansomwarePolicyRes();
        AntiRansomwareScheduleRes schedule = new AntiRansomwareScheduleRes();
        schedule.setSetWorm(true);
        antiRansomwarePolicyRes.setSchedule(schedule);
        return antiRansomwarePolicyRes;
    }
}
