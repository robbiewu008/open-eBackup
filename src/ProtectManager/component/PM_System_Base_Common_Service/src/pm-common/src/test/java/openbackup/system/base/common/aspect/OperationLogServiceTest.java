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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.sdk.alarm.AlarmRestApi;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;
import static org.mockito.ArgumentMatchers.any;

/**
 * Operation Log Service Test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OperationLogService.class)
public class OperationLogServiceTest {

    @Resource
    private OperationLogService operationLogService;

    @MockBean
    private AlarmRestApi AlarmRestApi;

    /**
     * 测试场景：正确处理 <br/>
     * 前置条件：AlarmRestApi处理成功 <br/>
     * 检查点：正确处理
     */
    @Test
    public void test_send_event_success () {
        Mockito.doNothing().when(AlarmRestApi).generateSystemLog(any());
        operationLogService.sendEvent(new LegoInternalEvent());
    }

    /**
     * 测试场景：异常处理情况 <br/>
     * 前置条件：AlarmRestApi处理异常 <br/>
     * 检查点：不抛出异常
     */
    @Test
    public void test_send_event_exception () {
        Mockito.doThrow(new RuntimeException("xx")).when(AlarmRestApi).generateSystemLog(any());
        operationLogService.sendEvent(new LegoInternalEvent());
    }
}
