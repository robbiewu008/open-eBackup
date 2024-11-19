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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.alarm.AlarmRestApi;
import openbackup.system.base.sdk.auth.api.DmeTokenApi;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.Objects;

import javax.servlet.http.HttpServletRequest;

/**
 * Operation Log Service
 *
 */
@Component
@Slf4j
public class OperationLogService {
    @Autowired
    AlarmRestApi alarmRestApi;

    @Autowired
    private DmeTokenApi dmeTokenApi;

    /**
     * send event
     *
     * @param event event
     */
    public void sendEvent(LegoInternalEvent event) {
        syncSendEvent(event, true);
    }

    /**
     * send event
     *
     * @param event event
     * @param isSync 是否同步发送
     */
    public void syncSendEvent(LegoInternalEvent event, boolean isSync) {
        try {
            updateDmeToken(event);
            log.info("send event thread start, isSync: {}", isSync);
            if (isSync) {
                alarmRestApi.generateSystemLog(event);
            } else {
                ThreadPoolTool.getPool().execute(() -> alarmRestApi.generateSystemLog(event));
            }
        } catch (Throwable e) {
            log.error("record log failed", ExceptionUtil.getErrorMessage(e));
        }
    }

    private void updateDmeToken(LegoInternalEvent event) {
        try {
            HttpServletRequest request =
                ((ServletRequestAttributes) Objects.requireNonNull(RequestContextHolder.getRequestAttributes()))
                    .getRequest();
            event.setDmeToken(request.getHeader(Constants.DME_AUTH_TOKEN));
        } catch (Exception ex) {
            log.info("Invalid dme request. no need record dme operation log.", ExceptionUtil.getErrorMessage(ex));
        }
    }
}
