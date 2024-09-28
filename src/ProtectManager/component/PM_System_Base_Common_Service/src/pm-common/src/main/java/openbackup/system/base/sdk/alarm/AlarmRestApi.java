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
package openbackup.system.base.sdk.alarm;

import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.alarm.model.InternalAlarm;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 功能描述
 *
 */
@FeignClient(name = "AlarmRestApi", url = "${service.url.alarm-manager}",
    configuration = CommonFeignConfiguration.class)
public interface AlarmRestApi {
    /**
     * 内部操作日志
     *
     * @param event 事件对象
     */
    @PutMapping("/v1/internal/alarms/log")
    void generateSystemLog(@RequestBody LegoInternalEvent event);

    /**
     * 内部告警请求
     *
     * @param internalAlarm 告警请求
     */
    @PostMapping("/v1/internal/alarms")
    void sendInternalAlarm(@RequestBody InternalAlarm internalAlarm);


    /**
     * 内部告警请求
     *
     * @param internalAlarm 告警请求
     */
    @PutMapping("/v1/internal/alarms/action/clear/entity")
    void clearInternalAlarm(@RequestBody InternalAlarm internalAlarm);
}
