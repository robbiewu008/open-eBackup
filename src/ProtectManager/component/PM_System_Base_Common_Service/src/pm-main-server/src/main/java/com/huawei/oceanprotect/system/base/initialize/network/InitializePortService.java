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
package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.BondPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ModifyLogicPortDto;
import com.huawei.oceanprotect.system.base.model.LogicPortFilterParam;
import com.huawei.oceanprotect.system.base.model.ModifyLogicPortRouteRequest;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;

import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import java.util.List;

/**
 * 添加DM端口
 *
 */
public interface InitializePortService {
    /**
     * 添加绑定端口列表
     *
     * @param deviceId 设备id
     * @param username 用户名
     * @param bondPortList 绑定端口列表
     */
    void addBondPort(String deviceId, String username, List<BondPortDto> bondPortList);

    /**
     * 添加逻辑端口列表
     *
     * @param logicPort 逻辑端口
     * @return 是否是复用的逻辑端口
     */
    boolean addLogicPort(LogicPortDto logicPort);

    /**
     * 获取调用osa接口的用户名
     *
     * @return 调用osa接口的用户名
     */
    String getUsername();

    /**
     * 获取调用osa接口的设备id
     *
     * @return 调用osa接口的设备id
     */
    String getDeviceId();

    /**
     * 处理逻辑端口：创建和保存
     *
     * @param logicPort 逻辑端口
     */
    void handleLogicPort(LogicPortDto logicPort);

    /**
     * 获取逻辑端口列表
     *
     * @return 逻辑端口列表
     */
    List<LogicPortAddRequest> getLogicPort();

    /**
     * 修改逻辑端口
     *
     * @param name 待修改逻辑端口名称
     * @param logicPortAddRequest 待修改逻辑端口请求体
     * @return 逻辑端口列表
     */
    BondPort modifyLogicPort(String name, ModifyLogicPortDto logicPortAddRequest);

    /**
     * 删除逻辑端口
     *
     * @param name    待删除逻辑端口名称
     */
    void deleteLogicPort(String name);

    /**
     * 根据过滤条件获取端口列表
     *
     * @param condition 过滤条件
     * @return 所有的端口信息
     */
    AllPortListResponseDto getPorts(LogicPortFilterParam condition);

    /**
     * 添加端口route信息
     *
     * @param request request
     * @return 修改后端口route信息
     */
    PortRouteInfo addPortRoute(ModifyLogicPortRouteRequest request);

    /**
     * 修改端口route信息
     *
     * @param request request
     */
    void deletePortRoute(ModifyLogicPortRouteRequest request);

    /**
     * 获取逻辑端口route信息
     *
     * @param portName portName
     * @return 修改后端口route信息
     */
    List<PortRouteInfo> getPortRouteInfo(String portName);

    /**
     * 获取用户创建的逻辑端口
     *
     * @return 用户创建的逻辑端口
     */
    List<LogicPortDto> getLogicPortsCreatedByUser();

    /**
     * 获取用户创建的逻辑端口
     *
     * @param dmLogicPortList 底座逻辑端口列表
     * @return 用户创建的逻辑端口
     */
    List<LogicPortDto> getLogicPortsCreatedByUser(List<LogicPortAddRequest> dmLogicPortList);

    /**
     * 根据类型和esn查询初始化配置信息
     *
     * @param initType 初始化类型
     * @param esn ens
     * @return 初始化配置信息
     */
    InitConfigInfo queryInitConfigByTypeAndEsn(String initType, String esn);

    /**
     * 删除底座逻辑端口
     *
     * @param name 要删除的逻辑端口名称
     * @param existServicePort existServicePort
     */
    void deleteSingleLogicPortOfDm(String name, ServicePortPo existServicePort);

    /**
     * 添加底座逻辑端口
     *
     * @param logicPortDto 逻辑端口
     */
    void addSingleLogicPortOfDm(LogicPortDto logicPortDto);

    /**
     * 校验绑定端口名称和端口列表，要么都相同，是复用；要么都不同，是新建；其他情况合理报错
     *
     * @param newLogicPort 新创建的逻辑端口
     * @param existLogicPort 已有的逻辑端口
     */
    void checkIsValidPortNameAndPortNameList(LogicPortDto newLogicPort, ServicePortPo existLogicPort);

    /**
     * 升级场景，判断数据库是否存在初始化配置信息
     * 1.如果有初始化配置信息，直接返回
     * 2.如果没有，会写入db
     */
    void writePortsToDbFromConfigWhenUpdateBeforeSix();
}
