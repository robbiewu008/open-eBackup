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
package com.huawei.oceanprotect.system.base.controller;

import com.huawei.oceanprotect.system.base.model.DaylightSavingTimeStamp;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.permission.Permission;

import org.apache.ibatis.annotations.Param;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.time.LocalDate;
import java.time.LocalTime;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.util.ArrayList;

/**
 * 冬夏令时控制器，提供冬夏令时时间戳相关接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/system")
public class DaylightSavingTimeController {
    /**
     * 提供冬夏令时一天对应的时间戳接口
     *
     * @param date 日期
     * @return DaylightSavingTimeStamp
     */
    @ExterAttack
    @GetMapping("/timeline")
    @Permission(roles = {
            Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR,
            Constants.Builtin.ROLE_DEVICE_MANAGER}, enableCheckAuth = false)
    public DaylightSavingTimeStamp getDaylightSavingTimestamp(@Param("date") String date) {
        ArrayList<String> timeStr = new ArrayList<>();
        ArrayList<String> timeStamp = new ArrayList<>();
        DaylightSavingTimeStamp daylightSavingTimeStamp = new DaylightSavingTimeStamp();

        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        LocalDate localDate;
        try {
            localDate = LocalDate.parse(date);
        } catch (DateTimeParseException e) {
            log.error("request param date parse error");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "request param date parse error");
        }
        LocalTime localTime = LocalTime.parse("00:00:00");

        ZonedDateTime startOfDay = ZonedDateTime.of(localDate, localTime, ZoneId.systemDefault());
        ZonedDateTime hourlyDateTime = startOfDay;

        while (hourlyDateTime.compareTo(startOfDay.plusDays(1)) <= 0) {
            timeStr.add(hourlyDateTime.format(formatter));
            timeStamp.add(Long.toString(hourlyDateTime.toInstant().getEpochSecond()));
            hourlyDateTime = hourlyDateTime.plusHours(1);
        }
        daylightSavingTimeStamp.setTimeStr(timeStr);
        daylightSavingTimeStamp.setTimeStamp(timeStamp);
        return daylightSavingTimeStamp;
    }
}
