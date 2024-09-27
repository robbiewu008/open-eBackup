/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.aspect;

import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.alarm.AlarmRestApi;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * Operation Log Service
 *
 * @author l00272247
 * @since 2021-01-16
 */
@Component
@Slf4j
public class OperationLogService {
    @Autowired
    AlarmRestApi alarmRestApi;

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
}
