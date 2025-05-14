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

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ModifyLogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.model.LogicPortFilterParam;
import com.huawei.oceanprotect.system.base.model.ModifyLogicPortRouteRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.FailoverGroupResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.impl.strategy.deploy.InitDeployTypeStrategyContext;
import com.huawei.oceanprotect.system.base.service.strategy.deploy.InitDeployTypeStrategyService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.CommonOperationCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import org.apache.ibatis.annotations.Param;
import org.hibernate.validator.constraints.Length;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotBlank;

/**
 * DM端口相关信息对外接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/system")
@AllArgsConstructor
public class PortController {
    private static final long LLD_SIZE = 2 * 1024 * 1024L;

    @Autowired
    private InitializePortService initializePortService;

    @Autowired
    private InitDeployTypeStrategyContext initDeployTypeStrategyContext;

    @Autowired
    private NetWorkPortService netWorkPortService;

    @Autowired
    private ClusterBasicService clusterBasicService;


    /**
     * 查询所有端口信息
     *
     * @param condition 查询参数，目前支持按照端口名称和id过滤
     * @return 端口信息
     */
    @ExterAttack
    @GetMapping("/ports")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public AllPortListResponseDto getAllPorts(@Validated LogicPortFilterParam condition) {
        try {
            return initializePortService.getPorts(condition);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getCode());
            throw e.toLegoException();
        }
    }

    /**
     * 新增逻辑端口
     *
     * @param addLogicPortRequest 用户信息和逻辑端口信息
     */
    @ExterAttack
    @PostMapping("/ports")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.ADD_LOGIC_PORT, target = EventTarget.SYSTEM, details = {"$1.name",
            "$initConfigMap.role", "$1.ip", "$1.mask", "$1.gateWay", "$initConfigMap.homePortType",
            "$initConfigMap.homePortId"}, context = {
                    @Context(name = "initConfigMap",
                            statement = "@init_config_context_loader_get_init_config_map.call($1)", required = true)})
    public void addPorts(@RequestBody @Valid LogicPortDto addLogicPortRequest) {
        try {
            log.info("start to create logic port.");
            initializePortService.handleLogicPort(addLogicPortRequest);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode() error.", ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 删除端口
     *
     * @param name 逻辑端口名称
     */
    @ExterAttack
    @DeleteMapping("/ports")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.DELETE_LOGIC_PORT, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM, details = {"$1"})
    public void deletePorts(@Param("name") String name) {
        try {
            initializePortService.deleteLogicPort(name);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode() error.", ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 修改端口信息
     *
     * @param modifyLogicPortRequest 用户信息和逻辑端口信息
     * @param name 逻辑端口名称
     * @return 端口信息
     */
    @ExterAttack
    @PutMapping("/ports")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.MODIFY_LOGIC_PORT, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM, details = {
            "$2", "$1.name", "$1.ip", "$1.mask", "$1.gateWay"})
    public BondPort modifyPorts(@RequestBody @Valid ModifyLogicPortDto modifyLogicPortRequest,
            @Param("name") String name) {
        try {
            return initializePortService.modifyLogicPort(name, modifyLogicPortRequest);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getDesc(), ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 添加端口路由
     *
     * @param request 添加请求
     * @return 路由信息
     */
    @ExterAttack
    @PostMapping("/ports/route")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.ADD_PORT_ROUTE, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM, details = {
            "$1.portName", "$1.route.destination", "$1.route.mask", "$1.route.gateway"})
    public PortRouteInfo addPortRoute(@RequestBody @Valid ModifyLogicPortRouteRequest request) {
        try {
            return initializePortService.addPortRoute(request);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getDesc(), ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 删除端口路由
     *
     * @param request 删除请求
     */
    @ExterAttack
    @DeleteMapping("/ports/route")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.DELETE_PORT_ROUTE, rank = FaultEnum.AlarmSeverity.INFO,
            target = EventTarget.SYSTEM, details = {
            "$1.portName", "$1.route.destination"})
    public void deletePortRoute(@RequestBody @Valid ModifyLogicPortRouteRequest request) {
        try {
            initializePortService.deletePortRoute(request);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getDesc(), ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 获取所有端口组
     *
     * @param portName 逻辑端口名称
     * @return 路由信息
     */
    @ExterAttack
    @GetMapping("/ports/routes")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public List<PortRouteInfo> getPortRoutes(@RequestParam @NotBlank @Length(max = 256) String portName) {
        try {
            return initializePortService.getPortRouteInfo(portName);
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getDesc(), ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * 获取所有漂移组
     *
     * @return 漂移组信息
     */
    @ExterAttack
    @GetMapping("/failover-group")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public List<FailoverGroupResponse> getFailoverGroup() {
        try {
            return netWorkPortService
                    .queryFailovergroup(clusterBasicService.getCurrentClusterEsn(), UserUtils.getBusinessUsername())
                    .getData();
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.decode(): {} ", e.getDesc(), ExceptionUtil.getErrorMessage(e));
            throw e.toLegoException();
        }
    }

    /**
     * lld配置解析
     *
     * @param lld MulipartFile
     * @return 初始化配置 请求体
     */
    @ExterAttack
    @PostMapping("/lld/action/upload")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = CommonOperationCode.INITIALIZE_UPLOAD_LLD, rank = FaultEnum.AlarmSeverity.INFO,
        target = EventTarget.SYSTEM)
    public InitNetworkBody checkAndReturnInitXls(@RequestParam("lld") MultipartFile lld) {
        InitNetworkBody initNetworkBody;
        checkLldSize(lld);
        try {
            log.info("start to upload lld file.");
            InitDeployTypeStrategyService strategyService = initDeployTypeStrategyContext.getStrategyService();
            initNetworkBody = strategyService.getInitNetworkBodyByLLD(lld);
        } catch (IllegalArgumentException e) {
            log.error("The format of lld file is illegal.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_VALIDATE_FAILED, "Illegal lld file.");
        } catch (DeviceManagerException e) {
            log.error("DeviceManagerException.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(e.getCode(), new String[] {e.getErrorParam()});
        }
        return initNetworkBody;
    }

    private void checkLldSize(MultipartFile lld) {
        if (lld.getSize() > LLD_SIZE) {
            log.error("The lld file size: {} is larger than 2G.", lld.getSize());
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, new String[] {"lld size"},
                    "net plane name: lld size no require");
        }
    }
}
