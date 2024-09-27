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
package openbackup.system.base.sdk.operationlog;

import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.RequestUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.time.Instant;
import java.util.Locale;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-05-21
 */
@Component
public class ManualSendOperationLogImpl implements ManualSendOperationLog {
    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private OperationLogService operationLogService;

    @Override
    public void sendOperationLog(String sourceType, String moName, String[] params, boolean isSuccess) {
        LegoInternalEvent legoInternalEvent = getBaseLegoInternalEvent(sourceType, moName, params, isSuccess);
        operationLogService.sendEvent(legoInternalEvent);
    }

    /**
     * 手动记录失败的操作日志
     *
     * @param sourceType source type
     * @param moName mo name
     * @param params params
     * @param exception exception
     */
    @Override
    public void sendOperationFailedLog(String sourceType, String moName,
        String[] params, LegoCheckedException exception) {
        LegoInternalEvent legoInternalEvent = getBaseLegoInternalEvent(sourceType, moName, params, false);
        if (Objects.nonNull(exception)) {
            legoInternalEvent.setLegoErrorCode(String.valueOf(exception.getErrorCode()));
        }
        operationLogService.sendEvent(legoInternalEvent);
    }

    private LegoInternalEvent getBaseLegoInternalEvent(
            String sourceType, String moName, String[] params, boolean isSuccess) {
        // 获取当前request
        RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        HttpServletRequest request;
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        } else {
            throw new IllegalStateException("param type incorrect");
        }

        // 获取当前登录用户的token信息
        TokenBo tokenBo = tokenVerificationService.parsingTokenFromRequest();

        // 获取操作日志占位符所有参数
        String[] eventParams = new String[IsmNumberConstant.TWO];
        if (params != null && params.length > 0) {
            eventParams = new String[params.length + IsmNumberConstant.TWO];
            System.arraycopy(params, 0, params, IsmNumberConstant.TWO, params.length);
        }
        eventParams[0] = tokenBo.getUser().getName();
        eventParams[1] = RequestUtil.getClientIpAddress(request);

        String sourceTypeValue = sourceType;
        // 如果操作日志对象有@, 则需要去掉@
        if (sourceTypeValue.startsWith("@")) {
            sourceTypeValue = sourceTypeValue.substring(1);
        }

        LegoInternalEvent event = new LegoInternalEvent();
        event.setSourceType("operation_target_" + sourceTypeValue.toLowerCase(Locale.ENGLISH) + "_label");
        event.setMoName(moName);
        event.setEventLevel(FaultEnum.AlarmSeverity.INFO);
        event.setMoIP(RequestUtil.getClientIpAddress(request));
        event.setEventParam(eventParams);
        event.setEventId(moName);
        event.setEventTime(Instant.now().getEpochSecond());
        event.setEventSequence(Instant.now().getNano());
        event.setIsSuccess(isSuccess);
        event.setUserId(tokenBo.getUser().getId());
        return event;
    }
}
