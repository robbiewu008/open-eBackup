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
package com.huawei.oceanprotect.system.base.service.impl.dorado;

import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.BINDING;
import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.ETHERNETPORT;
import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.VLAN;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitType;
import com.huawei.oceanprotect.system.base.model.ModifyLogicPortRouteRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortType;
import com.huawei.oceanprotect.system.base.service.InitConfigMethodService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * lld初始化特有方法
 *
 */
@Slf4j
@AllArgsConstructor
@Service
public class LldInitServiceImpl implements InitConfigMethodService {
    // 以太网端口
    private static final String PORT_NAME_LIST = "portNameList";

    // 逻辑端口类型
    private static final String HOME_PORT_TYPE = "homePortType";

    // 绑定端口id
    private static final String BOND_PORT_ID = "bondPortId";

    // vlan端口类型
    private static final String PORT_TYPE = "portType";

    private final InitNetworkConfigMapper initNetworkConfigMapper;
    private final InitializePortService initializePortService;

    @Override
    public boolean applicable(InitType initType) {
        return initType.isLldInit();
    }

    @Override
    public void addLogicPort(InitNetworkBody initNetworkBody) {
        // 已手动添加逻辑端口
        if (VerifyUtil.isEmpty(initNetworkBody.getBackupNetworkConfig().getLogicPorts().get(0).getIp())) {
            return;
        }
        log.info("Start to create logic ports and add route to port.");
        // LLD自动配置
        List<LogicPortDto> logicPortDtos = new ArrayList<>(initNetworkBody.getBackupNetworkConfig().getLogicPorts());
        if (!VerifyUtil.isEmpty(initNetworkBody.getCopyNetworkConfig())
                && !VerifyUtil.isEmpty(initNetworkBody.getCopyNetworkConfig().getLogicPorts())) {
            logicPortDtos.addAll(initNetworkBody.getCopyNetworkConfig().getLogicPorts());
        }
        if (!VerifyUtil.isEmpty(initNetworkBody.getArchiveNetworkConfig())
                && !VerifyUtil.isEmpty(initNetworkBody.getArchiveNetworkConfig().getLogicPorts())) {
            logicPortDtos.addAll(initNetworkBody.getArchiveNetworkConfig().getLogicPorts());
        }

        logicPortDtos.forEach(port -> {
            LogicPortDto logicPortDto = new LogicPortDto();
            BeanUtils.copyProperties(port, logicPortDto);
            // 复用已有端口信息
            reuseExistPortInfo(logicPortDto);
            // 创建逻辑端口
            initializePortService.handleLogicPort(logicPortDto);
            // 该端口添加路由
            addPortRoute(logicPortDto);
        });
    }

    private void addPortRoute(LogicPortDto logicPortDto) {
        if (CollectionUtils.isEmpty(logicPortDto.getRoute())) {
            log.info("No route for this logic port: {}", logicPortDto.getName());
            return;
        }
        for (PortRouteInfo route : logicPortDto.getRoute()) {
            ModifyLogicPortRouteRequest request = new ModifyLogicPortRouteRequest();
            PortRouteInfo routeInfo = new PortRouteInfo();
            BeanUtils.copyProperties(route, routeInfo);
            routeInfo.setPortType(PortType.LOGICAL);
            routeInfo.setPortName(logicPortDto.getName());
            request.setRoute(routeInfo);
            request.setPortName(logicPortDto.getName());
            initializePortService.addPortRoute(request);
        }
        log.info("Add port route success.");
    }

    private void reuseExistPortInfo(LogicPortDto logicPortDto) {
        HomePortType portType = logicPortDto.getHomePortType();
        if (ETHERNETPORT.equalsHomePortType(portType.getHomePortType())) {
            log.info("Logic port type is based on ethernet port.");
            return;
        }

        List<InitConfigInfo> existLogicPorts = initNetworkConfigMapper.queryInitConfig("logicPorts");
        if (VerifyUtil.isEmpty(existLogicPorts)) {
            log.info("No existing logic ports in database.");
            return;
        }
        JSONArray jsonArray = JSONArray.fromObject(existLogicPorts.get(0).getInitValue());
        for (Object object : jsonArray) {
            JSONObject existLogicPort = JSONObject.fromObject(
                    initNetworkConfigMapper.queryInitConfig(object.toString()).get(0).getInitValue());
            boolean isReuse = false;
            switch (portType) {
                case BINDING:
                    isReuse = reuseBondPortInfo(logicPortDto, existLogicPort);
                    break;
                case VLAN:
                    isReuse = reuseVlanPortInfo(logicPortDto, existLogicPort);
                    break;
            }
            if (isReuse) {
                log.info("Reuse exist port info success.");
                return;
            }
        }
    }

    private boolean reuseBondPortInfo(LogicPortDto logicPortDto, JSONObject existLogicPort) {
        String existPortType = existLogicPort.getString(HOME_PORT_TYPE);
        if (ETHERNETPORT.equalsHomePortType(existPortType)) {
            return false;
        }

        String portName = String.join(",", logicPortDto.getBondPort().getPortNameList());
        if (BINDING.equalsHomePortType(existPortType)) {
            JSONObject existBondPort = existLogicPort.getJSONObject("bondPort");
            String existBondPortName = String.join(",",
                    JSONArray.fromObject(existBondPort.getString(PORT_NAME_LIST)));
            // 新创的绑定口和已有绑定口相同，复用
            if (existBondPortName.contains(portName)) {
                String bondPortId = existBondPort.getString(BOND_PORT_ID);
                logicPortDto.setHomePortId(bondPortId);
                logicPortDto.getBondPort().setId(bondPortId);
                return true;
            }
        }

        if (VLAN.equalsHomePortType(existPortType)) {
            JSONObject existVlan = existLogicPort.getJSONObject("vlan");
            String existVlanPortName = String.join(",",
                    JSONArray.fromObject(existVlan.getString(PORT_NAME_LIST)));
            // 新创绑定口与已有vlan的绑定口相同，复用
            if (existVlanPortName.contains(portName)
                    && BINDING.equalsHomePortType(existVlan.getString(PORT_TYPE))) {
                String existBondId = existVlan.getString(BOND_PORT_ID);
                logicPortDto.getBondPort().setId(existBondId);
                logicPortDto.setHomePortId(existBondId);
                return true;
            }
        }
        return false;
    }

    private boolean reuseVlanPortInfo(LogicPortDto logicPortDto, JSONObject existLogicPort) {
        String existingPortType = existLogicPort.getString(HOME_PORT_TYPE);
        if (ETHERNETPORT.equalsHomePortType(existingPortType)) {
            return false;
        }

        String portName = String.join(",", logicPortDto.getVlan().getPortNameList());
        if (BINDING.equalsHomePortType(existingPortType)) {
            JSONObject existBondPort = existLogicPort.getJSONObject("bondPort");
            String existBondPortName = String.join(",",
                    JSONArray.fromObject(existBondPort.getString(PORT_NAME_LIST)));
            if (existBondPortName.contains(portName)) {
                String bondPortId = existBondPort.getString(BOND_PORT_ID);
                logicPortDto.getBondPort().setId(bondPortId);
                return true;
            }
        }

        if (VLAN.equalsHomePortType(existingPortType)) {
            JSONObject existVlan = existLogicPort.getJSONObject("vlan");
            String existVlanPortName = String.join(",",
                    JSONArray.fromObject(existVlan.getString(PORT_NAME_LIST)));
            if (existVlanPortName.contains(portName)) {
                String vlanType = logicPortDto.getVlan().getPortType().getVlanPortType();
                String vlanTag = logicPortDto.getVlan().getTags().get(0).toString();
                String existVlanType = existVlan.getString(PORT_TYPE);
                String existVlanTag = existVlan.getString("tags");
                if (!vlanType.equals(existVlanType)) {
                    log.info("Vlan port type: {} is not the same as exist vlan port type: {}", vlanType, existVlanType);
                    return false;
                }

                // vlan端口已创好，直接复用
                if (existVlanTag.contains(vlanTag)) {
                    String id = existVlan.getString("id");
                    logicPortDto.setHomePortId(id);
                    logicPortDto.getVlan().setId(id);
                    return true;
                }

                // 绑定端口相同，复用该绑定口，但vlan口需创建
                if (BINDING.equalsHomePortType(vlanType)) {
                    String bondPortId = existVlan.getString(BOND_PORT_ID);
                    logicPortDto.getVlan().setBondPortId(bondPortId);
                    return true;
                }
            }
        }
        return false;
    }
}
