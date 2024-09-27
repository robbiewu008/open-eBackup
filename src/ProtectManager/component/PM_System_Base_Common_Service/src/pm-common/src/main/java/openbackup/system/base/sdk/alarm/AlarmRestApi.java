/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
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
