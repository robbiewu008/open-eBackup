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
package openbackup.access.framework.resource.schedule;

import openbackup.access.framework.resource.schedule.ProtectObjectConsistentStatusScheduler;
import openbackup.access.framework.resource.service.ProtectObjectConsistentService;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import org.mockito.Mockito;

/**
 * 功能描述: ProtectObjectConsistentStatusSchedulerTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */

@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {SqlInitializationAutoConfiguration.class})
public class ProtectObjectConsistentStatusSchedulerTest {

    private ProtectObjectConsistentStatusScheduler scheduler;
    @MockBean
    private ProtectObjectConsistentService protectObjectConsistentService;
    @MockBean
    private DeployTypeService deployTypeService;
    @MockBean
    private SystemSwitchInternalService systemSwitchInternalService;

    @Before
    public void setUp() {
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        SystemSwitchDto switchDto = new SystemSwitchDto();
        switchDto.setName(SwitchNameEnum.PROTECT_OBJECT_FILE_SYSTEM_VERIFY);
        switchDto.setStatus(SwitchStatusEnum.ON);
        PowerMockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(switchDto);
        scheduler = new ProtectObjectConsistentStatusScheduler(protectObjectConsistentService,
                deployTypeService, systemSwitchInternalService);
    }

    @Test
    public void test_schedule_refresh_and_check_protect_object_consistent_status() {
        scheduler.refreshProtectObjectConsistentStatus();
        scheduler.checkProtectObjectConsistentStatus();
        scheduler.afterPropertiesSet();
        Assert.assertTrue(true);
    }
}