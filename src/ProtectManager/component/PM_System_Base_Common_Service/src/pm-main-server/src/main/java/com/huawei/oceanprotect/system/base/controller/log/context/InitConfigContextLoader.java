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
package com.huawei.oceanprotect.system.base.controller.log.context;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * 初始化上下文，用于操作日志记录
 *
 */
@Component
@CalleeMethods(name = "init_config_context_loader", value = {@CalleeMethod(name = "getInitConfigMap")})
@Slf4j
public class InitConfigContextLoader {
    private final Map<HomePortType, String> homePortTypeMap = new HashMap<HomePortType, String>() {
        {
            put(HomePortType.ETHERNETPORT, "ETHERNETPORT");
            put(HomePortType.BINDING, "BINDING");
            put(HomePortType.VLAN, "VLAN");
        }
    };

    private final Map<PortRole, String> roleMap = new HashMap<PortRole, String>() {
        {
            put(PortRole.SERVICE, "BACKUP");
            put(PortRole.TRANSLATE, "Copy");
            put(PortRole.ARCHIVE, "ARCHIVE");
        }
    };

    /**
     * 从逻辑端口查询homePortId,role,homePortType
     *
     * @param logicPort 逻辑端口
     * @return 初始化上下文结果
     */
    public InitConfigContextResult getInitConfigMap(LogicPortDto logicPort) {
        log.info("Start to get init config map, logic port:{}", logicPort.getName());
        InitConfigContextResult result = new InitConfigContextResult();
        switch (logicPort.getHomePortType()) {
            case ETHERNETPORT:
                result.setHomePortId(Collections.singletonList(logicPort.getHomePortName()).toString());
                break;
            case BINDING:
                result.setHomePortId(logicPort.getBondPort().getPortNameList().toString());
                break;
            case VLAN:
                result.setHomePortId(logicPort.getVlan().getPortNameList().toString());
                break;
            default:
                log.error("Create logic port: {} failed, home port type error", logicPort.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                        "Create logic port failed, home port type error");
        }
        result.setRole(roleMap.get(logicPort.getRole()));
        result.setHomePortType(homePortTypeMap.get(logicPort.getHomePortType()));
        return result;
    }
}
