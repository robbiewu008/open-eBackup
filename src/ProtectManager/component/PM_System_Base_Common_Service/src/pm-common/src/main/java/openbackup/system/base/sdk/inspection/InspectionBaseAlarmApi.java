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
package openbackup.system.base.sdk.inspection;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.alarm.bo.AlarmVO;
import openbackup.system.base.sdk.alarm.model.AlarmStatisticResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 功能描述
 *
 */
@FeignClient(name = "InspectionBaseAlarmApi", url = "${pm-system-base.url}/v1",
        configuration = CommonFeignConfiguration.class)
public interface InspectionBaseAlarmApi {
    /**
     * Get alarms summary
     *
     * @param token 认证token
     * @return AlarmStatisticResponse obj
     */
    @ExterAttack
    @GetMapping(value = "/alarms/count")
    AlarmStatisticResponse queryAlarmSummary(@RequestHeader(name = "x-auth-token") String token);

    /**
     * 分页获取告警信息
     *
     * @param token 认证token
     * @param language 使用语言
     * @return 告警信息分页响应信息
     */
    @ExterAttack
    @GetMapping(value = "/alarms?severities=3&severities=4&pageSize=100")
    PageListResponse<AlarmVO> queryAlarmSeverities(@RequestHeader(name = "x-auth-token") String token,
                                                    @RequestParam("language") String language);
}
