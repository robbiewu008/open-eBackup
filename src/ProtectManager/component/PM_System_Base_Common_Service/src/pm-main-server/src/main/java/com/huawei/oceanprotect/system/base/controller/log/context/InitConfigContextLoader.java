/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/27
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
