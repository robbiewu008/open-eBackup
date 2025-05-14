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
package com.huawei.oceanprotect.system.base.initialize.network.ability;

import static com.huawei.oceanprotect.system.base.constant.InitConfigErrorCode.RETURN_OBJ_NOT_EXIST;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.ARCHIVE_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.BACKUP_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.REPLICATION_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM;
import static openbackup.system.base.common.constants.CommonErrorCode.IP_NETWORK_SEGMENT_LIMIT;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.constant.InitConfigErrorCode;
import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.BondPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.EthPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ModifyLogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.LogicPortFilterParam;
import com.huawei.oceanprotect.system.base.model.ModifyLogicPortRouteRequest;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.BondPortRes;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.ModifyBondPortInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.FailoverGroupResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.Failovergroup;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.FailovergroupMember;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vlan.ModifyVlanInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vlan.VlanInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.FailovergroupMemberType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.BondPortServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.SystemService;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Sets;

import feign.codec.DecodeException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.bean.NetWorkIpRoute;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.bean.NetWorkRouteInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.InitConstants;
import openbackup.system.base.common.constants.StorageCommonErrorCode;
import openbackup.system.base.common.enums.AddressFamily;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.enums.IpType;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.netplane.NetPlaneInfoReq;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.sdk.system.PortService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

/**
 * 添加DM端口
 *
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-03
 */
@Service
@Slf4j
public class InitializePortServiceAbility implements InitializePortService, PortService {
    private static final String COMMA = ",";

    private static final String SEMICOLONS = ";";

    private static final Integer MAX_RETRY_COUNT = 3;

    private static final Set<Long> CONVERT_TO_OP_BUSY_SET = Sets.newHashSet(
        StorageCommonErrorCode.SYSTEM_BUSY_RETRY_LATER, StorageCommonErrorCode.SYSTEM_BUSY,
        StorageCommonErrorCode.MESSAGE_TIME_OUT);

    private static final int FAILOVERGROUP_TYPE = 3;

    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private BondPortServiceApi bondPortServiceApi;

    @Autowired
    private NetWorkPortService netWorkPortService;

    @Autowired
    private SystemService systemService;

    @Autowired
    private NetworkService networkService;

    @Autowired
    private PortFactory portFactory;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 能够修改逻辑端口控制器的部署类型
     */
    private final ImmutableList<String> deployTypeList = ImmutableList.of(DeployTypeEnum.X9000.getValue());

    /**
     * 添加绑定端口列表
     *
     * @param deviceId 设备id
     * @param username 用户名
     * @param bondPortList 绑定端口列表
     */
    @Override
    public void addBondPort(String deviceId, String username, List<BondPortDto> bondPortList) {
        bondPortList.stream()
            .map(bondPort -> new BondPortAddRequest(bondPort.getName(), bondPort.getPortIdList()))
            .forEach(bondPortAddRequest -> bondPortServiceApi.addBondPort(deviceId, username, bondPortAddRequest));
    }

    /**
     * 添加逻辑端口列表
     *
     * @param addLogicPortRequest 逻辑端口
     * @return 是否是复用的逻辑端口
     */
    @Override
    public boolean addLogicPort(LogicPortDto addLogicPortRequest) {
        if (!InitNetworkConfigConstants.serviceRoles.contains(addLogicPortRequest.getRole())) {
            log.error("Create logic port: {} failed. port role: {} error", addLogicPortRequest.getName(),
                    addLogicPortRequest.getRole());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Create logic port failed. port role error");
        }
        if (addLogicPortRequest.getHomePortType().isBondPort()) {
            log.info("Add logic port, id:{}, name:{}, homePortName:{}, homePortType:{}, homePortId:{}, role:{},"
                    + " HomeControllerId:{}" + " bondPortName:{}" + " bondPortNameList:{} +  BondPort:{}",
                addLogicPortRequest.getId(), addLogicPortRequest.getName(), addLogicPortRequest.getHomePortName(),
                addLogicPortRequest.getHomePortType(), addLogicPortRequest.getHomePortId(),
                addLogicPortRequest.getRole(), addLogicPortRequest.getHomeControllerId(),
                addLogicPortRequest.getBondPort().getName(), addLogicPortRequest.getBondPort().getPortNameList(),
                addLogicPortRequest.getBondPort());
        }
        checkLogicPortSettings(addLogicPortRequest);
        createLogicPortByHomePortType(addLogicPortRequest);
        return false;
    }

    private void createLogicPortByHomePortType(LogicPortDto addLogicPortRequest) {
        BondPort addLogicPortRes;
        // 复用已有物理端口
        if (InitNetworkConfigConstants.MULTIPLEXING_PHYSICAL_PORT_TYPES.contains(addLogicPortRequest.getHomePortType())
                && !VerifyUtil.isEmpty(addLogicPortRequest.getHomePortId())) {
            // 校验IP地址相关信息
            addLogicPortRes = createLogicPortByRole(addLogicPortRequest);
        } else {
            // 新增逻辑端口同时创建物理端口
            switch (addLogicPortRequest.getHomePortType()) {
                case ETHERNETPORT:
                    // 增加以太网口的逻辑端口
                    addLogicPortRes = addEthTypeLogicPort(addLogicPortRequest);
                    break;
                case BINDING:
                    // 增加绑定端口的逻辑端口
                    addLogicPortRes = addBondTypeLogicPort(addLogicPortRequest);
                    break;
                case VLAN:
                    // 增加Vlan的逻辑端口
                    addLogicPortRes = addVlanTypeLogicPort(addLogicPortRequest);
                    break;
                default:
                    log.error("Create logic port: {} failed, home port type error", addLogicPortRequest.getName());
                    throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                            "Create logic port failed, home port type error");
            }
        }
        log.info("End to create logic port by homePortType, ID is : {}", addLogicPortRes.getId());
    }

    private void checkLogicPortSettings(LogicPortDto addLogicPortRequest) {
        String currentEsn = getDeviceId();
        Map<String, LogicPortAddRequest> dmPortsMap = getLogicPort().stream().collect(
            Collectors.toMap(LogicPortAddRequest::getName, logicPort -> logicPort));
        Map<String, VlanInfo> vlanMap = Optional.ofNullable(
            netWorkPortService.queryVlan(currentEsn, getUsername()).getData())
            .orElse(new ArrayList<>())
            .stream()
            .collect(Collectors.toMap(VlanInfo::getId, vlan -> vlan));
        List<InitConfigInfo> logicPortsOfDb = Optional.ofNullable(initNetworkConfigMapper.queryInitConfigByEsnAndType(
            Constants.LOGIC_PORTS_CREATED_BY_USER, currentEsn)).orElse(new ArrayList<>());
        if (CollectionUtils.isEmpty(logicPortsOfDb)) {
            return;
        }
        List<String> dbLogicPortNames = JSONArray.fromObject(logicPortsOfDb.get(0).getInitValue()).toBean(String.class);
        for (String dbLogicPortName : dbLogicPortNames) {
            LogicPortAddRequest logicPortOfDm = dmPortsMap.get(dbLogicPortName);
            if (logicPortOfDm == null
                || logicPortOfDm.getRole() != addLogicPortRequest.getRole()
                || !isSameNetworkSegment(addLogicPortRequest, logicPortOfDm)) {
                continue;
            }
            // 不能存在同网段且vlan id不同的逻辑端口
            if ((HomePortType.VLAN.equals(addLogicPortRequest.getHomePortType())
                && HomePortType.VLAN.equals(logicPortOfDm.getHomePortType()))) {
                String newVlanId = addLogicPortRequest.getVlan().getTags().get(0);
                String dmVlanId = vlanMap.get(logicPortOfDm.getHomePortId()).getTag();
                if (!StringUtils.equals(newVlanId, dmVlanId)) {
                    log.error("The new vlan logic port vlan id : {} is not same as the exist vlan logic port id : {}.",
                        newVlanId, dmVlanId);
                    throw new LegoCheckedException(InitConfigErrorCode.VLAN_ID_DIFFERENT_ERROR, "Illegal vlan id");
                }
                continue;
            }
            // 不能存在同网段且vlan、非vlan类型的共存的逻辑端口
            if (HomePortType.VLAN.equals(addLogicPortRequest.getHomePortType())
                || HomePortType.VLAN.equals(logicPortOfDm.getHomePortType())) {
                log.error("Vlan logic port can not coexist with non-vlan logic port.");
                throw new LegoCheckedException(InitConfigErrorCode.VLAN_AND_NON_VLAN_COEXIST_ERROR,
                    "Illegal logic port type");
            }
        }
    }

    private boolean isSameNetworkSegment(LogicPortDto newLogicPort, LogicPortAddRequest dmLogicPort) {
        if (IpType.IPV4.getValue().equals(newLogicPort.getIpType())
            && StringUtils.isNotEmpty(dmLogicPort.getIpv4Addr())) {
            return networkService.isIpv4SameNetworkSegment(newLogicPort.getIp(), newLogicPort.getMask(),
                dmLogicPort.getIpv4Addr(), dmLogicPort.getIpv4Mask());
        }
        return networkService.isIpv6SameNetworkSegment(newLogicPort.getIp(), newLogicPort.getMask(),
            dmLogicPort.getIpv6Addr(), dmLogicPort.getIpv6Addr());
    }

    @Override
    public String getUsername() {
        return systemService.getDeviceInfo().getUsername();
    }

    @Override
    public String getDeviceId() {
        return systemService.getDeviceInfo().getEsn();
    }

    @Override
    public void handleLogicPort(LogicPortDto logicPort) {
        checkLogicPortNum(logicPort);
        // 添加逻辑端口
        boolean isReuseLogicPort;
        try {
            isReuseLogicPort = addLogicPort(logicPort);
        } catch (LegoCheckedException e) {
            if (CONVERT_TO_OP_BUSY_SET.contains(e.getErrorCode())) {
                throw new LegoCheckedException(InitConfigErrorCode.PORT_ADD_FAILED_CASE_DEVICE_BUSY,
                    "DeviceManager is busy");
            }
            throw e;
        }
        // 保存逻辑端口
        saveLogicPortToDb(logicPort, isReuseLogicPort);
    }

    private void checkModifyIpNetwork(ModifyLogicPortDto modifyLogicPortRequest) {
        List<LogicPortAddRequest> logicPortAddRequests = getLogicPort();
        if (VerifyUtil.isEmpty(logicPortAddRequests) || logicPortAddRequests.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The logical port to be modified is not found.");
        }
        LogicPortAddRequest targetLogicPortAddRequest = logicPortAddRequests
            .stream()
            .filter(logicPortRequest -> modifyLogicPortRequest.getId().equals(logicPortRequest.getId()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The logical port to be modified is not found."));
        // 获取该绑定端口或以太网卡下所有的逻辑端口列表，除了待修改逻辑端口本身
        List<LogicPortAddRequest> logicPortList = getAllLogicPortList(targetLogicPortAddRequest);
        logicPortList.remove(targetLogicPortAddRequest);
        // 若只待修改逻辑端口本身则修改端口信息不进行网段校验
        if (VerifyUtil.isEmpty(logicPortList) || logicPortList.isEmpty()) {
            log.info("Only one logical port exists.");
            return;
        }
        // 校验需要修改网络协议类型与已存在逻辑端口网络协议类型是否相同
        for (LogicPortAddRequest logicPort : logicPortList) {
            if (!logicPort.getAddressFamily().equals(modifyLogicPortRequest.getAddressFamily())) {
                throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                    new String[] {modifyLogicPortRequest.getIp(), logicPort.getIpv4Addr()},
                    "ipType is different");
            }
            LogicPortDto logicPortDto = new LogicPortDto();
            BeanUtils.copyProperties(modifyLogicPortRequest, logicPortDto);
            // 根据IP网络协议类型进行对应的校验
            if (AddressFamily.IPV4.equals(modifyLogicPortRequest.getAddressFamily())) {
                checkIpv4NetworkSegment(logicPortDto, logicPort);
            } else {
                checkIpv6NetworkSegment(logicPortDto, logicPort);
            }
        }
    }

    private List<LogicPortAddRequest> getAllLogicPortList(LogicPortAddRequest logicPortAddRequest) {
        switch (logicPortAddRequest.getHomePortType()) {
            case ETHERNETPORT:
                return queryAllEthLogicPortByHomePortName(logicPortAddRequest.getHomePortName());
            case BINDING:
                return queryAllBindLogicPortByBindId(logicPortAddRequest.getHomePortId());
            case VLAN:
                return fillVlanPortList(logicPortAddRequest);
            default:
                log.error("Create logic port: {} failed, home port type error", logicPortAddRequest.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Create logic port failed, home port type error");
        }
    }

    private List<LogicPortAddRequest> getAllLogicPortList(LogicPortDto logicPortDto) {
        switch (logicPortDto.getHomePortType()) {
            case ETHERNETPORT:
                return queryAllEthLogicPortByHomePortName(logicPortDto.getHomePortName());
            case BINDING:
                // 增加绑定端口的逻辑端口
                return queryAllBindLogicPortByPortNameList(logicPortDto.getBondPort().getPortNameList());
            case VLAN:
                return fillVlanPortList(logicPortDto);
            default:
                log.error("Create logic port: {} failed, home port type error", logicPortDto.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Create logic port failed, home port type error");
        }
    }

    private List<LogicPortAddRequest> queryAllBindLogicPortByBindId(String homePortId) {
        // 查出绑定端口信息
        BondPortDto bondPortDto = getHomeBindByHomePortId(homePortId);
        // 根据绑定端口信息查出通过该绑定端口查出的Vlan信息
        List<VlanPo> vlanPoList = getHomeVlanListByPortId(bondPortDto.getId());
        // 查出所有该绑定端口相关的逻辑端口
        return getAllLogicPort(bondPortDto.getId(), vlanPoList);
    }

    private List<LogicPortAddRequest> queryAllBindLogicPortByPortNameList(List<String> portNameList) {
        // 查出绑定端口信息
        BondPortDto bondPortDto = getHomeBindByPortNameList(portNameList);
        if (VerifyUtil.isEmpty(bondPortDto)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The Binding port" + portNameList + " is not found.");
        }
        // 根据绑定端口信息查出通过该绑定端口查出的Vlan信息
        List<VlanPo> vlanPoList = getHomeVlanListByPortId(bondPortDto.getId());
        // 查出所有该绑定端口相关的逻辑端口
        return getAllLogicPort(bondPortDto.getId(), vlanPoList);
    }

    private List<LogicPortAddRequest> queryAllEthLogicPortByHomePortName(String homePortName) {
        // 查出以太网端口信息
        EthPortDto ethPortDto = getHomeEthByHomePortName(homePortName);
        // 根据端口信息查出通过该以太网端口查出的Vlan信息
        List<VlanPo> vlanPoList = getHomeVlanListByPortId(ethPortDto.getId());
        // 查出所有该以太网口相关的逻辑端口
        return getAllLogicPort(ethPortDto.getId(), vlanPoList);
    }

    private List<LogicPortAddRequest> fillVlanPortList(LogicPortAddRequest logicPortAddRequest) {
        VlanPo targetVlanPo = getHomeVlanListById(logicPortAddRequest.getHomePortId());
        switch (targetVlanPo.getPortType()) {
            case ETH:
                return queryAllEthLogicPortByHomePortName(removeSuffix(targetVlanPo.getName()));
            case BOND:
                return queryAllBindLogicPortByBindId(targetVlanPo.getPortId());
            default:
                log.error("Modify logic port: {} failed, home port type error", logicPortAddRequest.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Modify logic port failed, home port type error");
        }
    }

    private List<LogicPortAddRequest> fillVlanPortList(LogicPortDto logicPortDto) {
        switch (logicPortDto.getVlan().getPortType()) {
            case ETH:
                // 首次创建VLAN
                if (!VerifyUtil.isEmpty(logicPortDto.getVlan().getPortNameList().get(0))) {
                    return queryAllEthLogicPortByHomePortName(logicPortDto.getVlan().getPortNameList().get(0));
                }
                // 复用VLAN
                VlanPo targetVlanPo = getHomeVlanListById(logicPortDto.getHomePortId());
                return queryAllEthLogicPortByHomePortName(removeSuffix(targetVlanPo.getName()));
            case BOND:
                // 首次创建绑定端口并且创建VLAN
                if (VerifyUtil.isEmpty(logicPortDto.getVlan().getBondPortId())) {
                    return new ArrayList<>();
                }
                    return queryAllBindLogicPortByPortNameList(logicPortDto.getVlan().getPortNameList());
            default:
                log.error("Create logic port: {} failed, home port type error", logicPortDto.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Create logic port failed, home port type error");
        }
    }

    private BondPortDto getHomeBindByHomePortId(String homePortId) {
        Optional<BondPortDto> bondPortDto = getBondPort()
            .stream()
            .filter(port -> port.getId().equals(homePortId))
            .findFirst();
        if (!bondPortDto.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The Binding port" + homePortId + " is not found.");
        }
        return bondPortDto.get();
    }

    private BondPortDto getHomeBindByPortNameList(List<String> portNameList) {
        HashSet<String> portNameHashSet = new HashSet<>(portNameList);
        Optional<BondPortDto> bondPortDto = getBondPort()
            .stream()
            .filter(port -> portNameHashSet
                .equals(new HashSet<>(Arrays.asList(port.getBondInfo().split(",")))))
            .findFirst();
        if (!bondPortDto.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The Binding port" + portNameList + " is not found.");
        }
        return bondPortDto.get();
    }

    private EthPortDto getHomeEthByHomePortName(String homePortName) {
        Optional<EthPortDto> targetEthPortDto = netWorkPortService.queryEthPorts(getDeviceId(), getUsername())
            .getData()
            .stream()
            .filter(ethPort -> ethPort.getLocation().equals(homePortName))
            .map(ethPort -> {
                EthPortDto ethPortDto = new EthPortDto();
                BeanUtils.copyProperties(ethPort, ethPortDto);
                return ethPortDto;
            })
            .findFirst();
        if (!targetEthPortDto.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The ETH port" + homePortName + " is not found.");
        }
        return targetEthPortDto.get();
    }

    private List<VlanPo> getHomeVlanListByPortId(String portId) {
        List<VlanPo> vlanPoList = netWorkPortService.queryVlan(getDeviceId(), getUsername()).getData()
            .stream()
            .filter(vlanInfo -> vlanInfo.getPortId().equals(portId))
            .map(vlanInfo -> {
                VlanPo vlanPo = new VlanPo();
                BeanUtils.copyProperties(vlanInfo, vlanPo);
                return vlanPo;
            }).collect(Collectors.toList());
        if (VerifyUtil.isEmpty(vlanPoList)) {
            log.info("vlanPoList create by portId{} is empty", portId);
            return new ArrayList<>();
        }
        return vlanPoList;
    }

    private VlanPo getHomeVlanListById(String id) {
        Optional<VlanPo> targetVlanPo = netWorkPortService.queryVlan(getDeviceId(), getUsername()).getData()
            .stream()
            .filter(vlanInfo -> vlanInfo.getId().equals(id))
            .map(vlanInfo -> {
                VlanPo vlanPo = new VlanPo();
                BeanUtils.copyProperties(vlanInfo, vlanPo);
                return vlanPo;
            }).findFirst();
        if (!targetVlanPo.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The VLAN is empty");
        }
        return targetVlanPo.get();
    }

    private List<LogicPortAddRequest> getAllLogicPort(String id, List<VlanPo> vlanPoList) {
        // 查询出所有逻辑端口中，使用该以太网口或绑定口创建的除vlan的逻辑端口
        List<LogicPortAddRequest> logicPortAddRequests = new ArrayList<>(getLogicPortByHomePortId(id));
        // 查询出所有逻辑端口中，使用该以太网口或绑定口创建的vlan类型逻辑端口
        if (!VerifyUtil.isEmpty(vlanPoList)) {
            for (VlanPo vlanPo : vlanPoList) {
                logicPortAddRequests.addAll(getLogicPortByHomePortId(vlanPo.getId()));
            }
        }
        return logicPortAddRequests;
    }

    private void checkAddIpNetwork(LogicPortDto addLogicPortRequest) {
        // 获取该绑定端口、以太网口或VLAN下所有的逻辑端口列表
        List<LogicPortAddRequest> logicPortList = getAllLogicPortList(addLogicPortRequest);
        if (logicPortList.isEmpty()) {
            log.info("The logical port under the physical port is empty, ip {}.", addLogicPortRequest.getIp());
            return;
        }
        // 根据IP网络协议类型进行对应的校验
        for (LogicPortAddRequest logicPort : logicPortList) {
            if (!logicPort.getAddressFamily().equals(AddressFamily.fromString(addLogicPortRequest.getIpType()))) {
                throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                    new String[] {addLogicPortRequest.getIp(), logicPort.getIpv4Addr()},
                    "ipType is different");
            }
            if (AddressFamily.IPV4.equals(AddressFamily.fromString(addLogicPortRequest.getIpType()))) {
                checkIpv4NetworkSegment(addLogicPortRequest, logicPort);
            } else {
                checkIpv6NetworkSegment(addLogicPortRequest, logicPort);
            }
        }
    }

    private String removeSuffix(String name) {
        if (VerifyUtil.isEmpty(name)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The name to be deleted does not exist.");
        }
        int lastIndex = name.lastIndexOf(".");
        return name.substring(0, lastIndex);
    }

    private List<LogicPortAddRequest> getLogicPortByHomePortId(String homePortId) {
        List<LogicPortAddRequest> logicPortAddRequests = getLogicPort();
        if (logicPortAddRequests.isEmpty()) {
            return new ArrayList<>();
        }
        return logicPortAddRequests
            .stream()
            .filter(logicPortAddRequest -> homePortId.equals(logicPortAddRequest.getHomePortId()))
            .collect(Collectors.toList());
    }

    private void checkIpv4NetworkSegment(LogicPortDto addLogicPortRequest, LogicPortAddRequest logicPort) {
        if (logicPort.getAddressFamily() != AddressFamily.IPV4) {
            log.error("Check ip type failed, {} not same to {}.", logicPort.getAddressFamily(), AddressFamily.IPV4);
            throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                new String[] {addLogicPortRequest.getIp(), logicPort.getIpv6Addr()},
                "ipType is different");
        }

        try {
            byte[] taskIp = InetAddress.getByName(addLogicPortRequest.getIp()).getAddress();
            byte[] taskMask = InetAddress.getByName(addLogicPortRequest.getMask()).getAddress();
            byte[] existIp = InetAddress.getByName(logicPort.getIpv4Addr()).getAddress();
            byte[] existMask = InetAddress.getByName(logicPort.getIpv4Mask()).getAddress();
            for (int i = 0; i < taskIp.length; i++) {
                if ((taskIp[i] & taskMask[i]) != (existIp[i] & existMask[i])) {
                    throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                        new String[] {addLogicPortRequest.getIp(), logicPort.getIpv4Addr()},
                        "The IPv4 addresses are not in the same network segment.");
                }
            }
        } catch (UnknownHostException e) {
            log.error("check isValidIPV4 failed.");
        }
    }

    private void checkIpv6NetworkSegment(LogicPortDto addLogicPortRequest, LogicPortAddRequest logicPort) {
        if (logicPort.getAddressFamily() != AddressFamily.IPV6) {
            log.error("Check ip type failed, {} not same to {}.", logicPort.getAddressFamily(), AddressFamily.IPV6);
            throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                new String[] {addLogicPortRequest.getIp(), logicPort.getIpv4Addr()},
                "ipType is different");
        }
        try {
            BigInteger taskIp = new BigInteger(1, InetAddress.getByName(addLogicPortRequest.getIp()).getAddress());
            BigInteger taskMask = new BigInteger(1, InetAddress.getByName(addLogicPortRequest.getMask()).getAddress());
            BigInteger existIp = new BigInteger(1, InetAddress.getByName(logicPort.getIpv6Addr()).getAddress());
            BigInteger existMask = new BigInteger(1, InetAddress.getByName(logicPort.getIpv6Mask()).getAddress());
            if (!taskIp.and(taskMask).equals(existIp.and(existMask))) {
                throw new LegoCheckedException(IP_NETWORK_SEGMENT_LIMIT,
                    new String[] {addLogicPortRequest.getIp(), logicPort.getIpv4Addr()},
                    "The IPv6 addresses are not in the same network segment.");
            }
        } catch (UnknownHostException e) {
            log.error("check isValidIPV6 failed.");
        }
    }

    private void checkLogicPortNum(LogicPortDto logicPortDto) {
        InitConfigInfo logicPortsInitConfig = initNetworkConfigMapper
            .queryInitConfigByEsnAndType(Constants.LOGIC_PORTS_CREATED_BY_USER, getDeviceId()).stream().findFirst()
            .orElse(new InitConfigInfo());
        List<String> logicPortNames = JSONArray.fromObject(logicPortsInitConfig.getInitValue()).toBean(String.class);
        int logicPortLimit = PortRole.TRANSLATE.equals(logicPortDto.getRole())
                ? SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM
                : SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
        int roleNum = 0;
        for (String logicPortName : logicPortNames) {
            ServicePortPo existLogicPort = getServicePort(logicPortName);
            if (logicPortDto.getRole().equals(existLogicPort.getRole())
                && StringUtils.equals(logicPortDto.getCurrentControllerId(), existLogicPort.getCurrentControllerId())) {
                roleNum++;
            }
            if (roleNum >= logicPortLimit) {
                String portTypeLabel;
                switch (logicPortDto.getRole()) {
                    case SERVICE:
                        portTypeLabel = BACKUP_SERVICE_PORT_LABEL;
                        break;
                    case TRANSLATE:
                        portTypeLabel = REPLICATION_SERVICE_PORT_LABEL;
                        break;
                    case ARCHIVE:
                        portTypeLabel = ARCHIVE_SERVICE_PORT_LABEL;
                        break;
                    default:
                        log.error("The logic port role: {} is illegal.", logicPortDto.getRole());
                        throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal port role.");
                }
                log.error("The service: {} port exceed limit: {}.", logicPortDto.getRole(), logicPortLimit);
                throw new LegoCheckedException(CommonErrorCode.SERVICE_PORT_NUM_EXCEED_LIMIT,
                    new String[] {portTypeLabel, String.valueOf(logicPortLimit)}, "Illegal logic port num");
            }
        }
    }

    private void saveLogicPortToDb(LogicPortDto logicPort, boolean isReuseLogicPort) {
        saveReuseLogicPort(logicPort.getName(), isReuseLogicPort);
        ServicePortPo servicePortPo = new ServicePortPo();
        BeanUtils.copyProperties(logicPort, servicePortPo);
        servicePortPo.setDmRole(logicPort.getRole());
        insertInitConfig(logicPort.getName(), JSONObject.writeValueAsString(servicePortPo), getDeviceId());
        if (!VerifyUtil.isEmpty(logicPort.getFailoverGroupId())) {
            insertInitConfig(InitNetworkConfigConstants.FAIL_OVER_GROUP + logicPort.getName(),
                    logicPort.getFailoverGroupId(), getDeviceId());
        }
        upsertConfig(logicPort.getName(), Constants.LOGIC_PORTS_CREATED_BY_USER);
    }

    private void upsertConfig(String portName, String initType) {
        InitConfigInfo needToSaveOrUpdateConfigParam = new InitConfigInfo();
        needToSaveOrUpdateConfigParam.setEsn(getDeviceId());
        needToSaveOrUpdateConfigParam.setInitType(initType);
        InitConfigInfo logicPortsInitConfig = initNetworkConfigMapper
                .queryInitConfigByEsnAndType(initType, getDeviceId()).stream().findFirst()
                .orElse(null);
        if (VerifyUtil.isEmpty(logicPortsInitConfig)) {
            // 如果数据库没有logicPorts，则为第一次创建逻辑端口，走新增逻辑
            needToSaveOrUpdateConfigParam
                    .setInitValue(JSONObject.writeValueAsString(Collections.singletonList(portName)));
            initNetworkConfigMapper.insertInitConfig(needToSaveOrUpdateConfigParam);
        } else {
            // 如果数据库有logicPorts,走更新逻辑
            List<String> initValue = JSONArray.fromObject(logicPortsInitConfig.getInitValue())
                .toBean(String.class);
            initValue.add(portName);
            needToSaveOrUpdateConfigParam.setInitValue(JSONObject.writeValueAsString(initValue));
            initNetworkConfigMapper.updateInitConfigByEsnAndType(needToSaveOrUpdateConfigParam);
        }
    }

    private void saveReuseLogicPort(String name, boolean isReuseLogicPort) {
        if (!isReuseLogicPort) {
            return;
        }
        upsertConfig(name, Constants.REUSE_LOGIC_PORTS);
    }

    private void insertInitConfig(String initType, String initValue, String esn) {
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitType(initType);
        initConfigInfo.setInitValue(initValue);
        initConfigInfo.setEsn(esn);
        initNetworkConfigMapper.insertInitConfig(initConfigInfo);
    }

    // 如果需要新创绑定端口会在这个方法里新创
    private BondPort addVlanTypeLogicPort(LogicPortDto logicPort) {
        VlanPo vlan = logicPort.getVlan();
        if (VerifyUtil.isEmpty(vlan) || isVlanInfoInValid(vlan)) {
            log.error("Create vlan type logic port: {} failed. param error", logicPort.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Create vlan type logic port failed, param error");
        }
        log.info("Add vlan type logic port, vlan tags:{}, portType:{}, bondPortId:{}, portNameList:{}, mtu:{}",
                vlan.getTags(), vlan.getPortType(), vlan.getBondPortId(), vlan.getPortNameList(), vlan.getMtu());

        addVlanByType(logicPort);
        // 修改mtu
        ModifyVlanInfo modifyVlanInfo = new ModifyVlanInfo();
        modifyVlanInfo.setMtu(vlan.getMtu());
        BondPort logicPortRes;
        try {
            netWorkPortService.modifyVlan(getDeviceId(), getUsername(), vlan.getId(), modifyVlanInfo);
            // 创建逻辑端口
            logicPort.setHomePortId(vlan.getId());
            logicPortRes = createLogicPortByRole(logicPort);
        } catch (LegoCheckedException e) {
            // 创建逻辑端口失败,删除创建的vlan和绑定端口
            log.error("Create vlan type logic port:{} failed.", logicPort.getName(), ExceptionUtil.getErrorMessage(e));
            netWorkPortService.deleteVlan(getDeviceId(), getUsername(), vlan.getId());
            deleteVlanBondPort(vlan);
            throw e;
        }
        return logicPortRes;
    }

    private void deleteVlanBondPort(VlanPo vlan) {
        if (vlan.getPortType().equalsVlanPortType(VlanPortType.ETH.getVlanPortType())) {
            return;
        }
        netWorkPortService.deleteBondPort(getDeviceId(), getUsername(), vlan.getBondPortId());
    }

    private boolean isExistBondPortName(LogicPortDto logicPort) {
        if (VerifyUtil.isEmpty(logicPort.getBondPort())) {
            return false;
        }
        return !VerifyUtil.isEmpty(logicPort.getBondPort().getName());
    }

    private void addVlanByType(LogicPortDto logicPort) {
        VlanPo vlan = logicPort.getVlan();
        // 根据端口类型判断需要给以太网口创建vlan还是给绑定端口创建vlan
        VlanInfo addVlanParam = new VlanInfo();
        if (vlan.getPortType().equalsVlanPortType(VlanPortType.ETH.getVlanPortType())) {
            addVlanParam.setPortId(
                portFactory.createPort(HomePortType.ETHERNETPORT).queryHomePortId(vlan.getPortNameList().get(0)));
        } else {
            // 分别处理2种情况：需要新创绑定端口(会新创)和复用已有的绑定端口(不需要新创)
            handleAddVlanBond(logicPort, vlan, addVlanParam);
        }
        addVlanParam.setTag(String.valueOf(vlan.getTags().get(0)));
        addVlanParam.setPortType(vlan.getPortType());
        VlanInfo vlanRes = netWorkPortService.addVlan(getDeviceId(), getUsername(), addVlanParam).getData();
        saveVlanId(vlan, vlanRes);
    }

    private void handleAddVlanBond(LogicPortDto logicPort, VlanPo vlan, VlanInfo addVlanParam) {
        if (VerifyUtil.isEmpty(vlan.getBondPortId())) {
            String bondPortName;
            bondPortName = isExistBondPortName(logicPort)
                ? logicPort.getBondPort().getName()
                : InitNetworkConfigConstants.BOND_PORT_PREFIX + System.currentTimeMillis();
            logicPort.getBondPort().setName(bondPortName);
            checkBondLogicName(logicPort);
            // 如果绑定端口id为空，新创建绑定端口
            BondPortRes bondPortRes = userCreateBondPort(bondPortName,
                batchConvertEthPortNameToId(vlan.getPortNameList()));
            // 等待绑定端口是已连接状态
            waitBondPortLinkup(bondPortRes);
            vlan.setBondPortId(bondPortRes.getId());
            addVlanParam.setPortId(bondPortRes.getId());
        } else {
            // 如果绑定端口id不为空，则复用已有的绑定端口
            addVlanParam.setPortId(vlan.getBondPortId());
        }
    }

    private void saveVlanId(VlanPo vlan, VlanInfo vlanRes) {
        vlan.setId(vlanRes.getId());
    }

    private List<String> batchConvertEthPortNameToId(List<String> portNameList) {
        return portNameList.stream().map(name -> portFactory.createPort(HomePortType.ETHERNETPORT)
                .queryHomePortId(name)).collect(Collectors.toList());
    }

    private BondPort addBondTypeLogicPort(LogicPortDto logicPort) {
        BondPortPo bondPortInfo = logicPort.getBondPort();
        if (VerifyUtil.isEmpty(bondPortInfo) || isBondPortInValid(bondPortInfo)) {
            log.error("Create bond type logic port: {} failed. param error", logicPort.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "Create bond type logic port failed, param error");
        }
        log.info("Add bond type logic port, bond port portNameList:{}, mtu:{}", bondPortInfo.getPortNameList(),
            bondPortInfo.getMtu());

        String bondPortName = isExistBondPortName(logicPort)
            ? logicPort.getBondPort().getName()
            : InitNetworkConfigConstants.BOND_PORT_PREFIX + System.currentTimeMillis();
        bondPortInfo.setName(bondPortName);

        // 判断底座是否存在同名的绑定端口名称
        checkBondLogicName(logicPort);

        // 创建绑定端口
        BondPortRes bondPortRes = userCreateBondPort(bondPortName,
            batchConvertEthPortNameToId(bondPortInfo.getPortNameList()));

        // 等待绑定端口是已连接状态
        waitBondPortLinkup(bondPortRes);
        BondPort logicPortRes;
        try {
            modifyBondPort(bondPortInfo, bondPortRes);
            logicPort.setHomePortId(bondPortRes.getId());
            logicPortRes = createLogicPortByRole(logicPort);
        } catch (LegoCheckedException e) {
            // 创建逻辑端口失败,删除创建的绑定端口
            log.error("Create bond port type logic port:{} failed.", logicPort.getName(),
                ExceptionUtil.getErrorMessage(e));
            netWorkPortService.deleteBondPort(getDeviceId(), getUsername(), bondPortRes.getId());
            throw e;
        }
        return logicPortRes;
    }

    private void modifyBondPort(BondPortPo bondPortInfo, BondPortRes bondPortRes) {
        // 保存绑定端口的ID
        bondPortInfo.setId(bondPortRes.getId());
        // 修改绑定端口的最大传输单元
        ModifyBondPortInfo modifyBondPortInfo = new ModifyBondPortInfo();
        modifyBondPortInfo.setMtu(bondPortInfo.getMtu());
        netWorkPortService.modifyBondPort(getDeviceId(), getUsername(), bondPortRes.getId(),
                modifyBondPortInfo);
    }

    /**
     * 检查传入的绑定端口名字在底座是否存在
     *
     * @param newLogicPort 新创的逻辑端口
     */
    public void checkBondLogicName(LogicPortDto newLogicPort) {
        // 调底座查询绑定端口名称的接口
        List<BondPortRes> dmBondPortList = netWorkPortService.getBondPort(clusterBasicService.getCurrentClusterEsn(),
            InitConstants.ADMIN).getData().stream().collect(Collectors.toList());

        if (CollectionUtils.isEmpty(dmBondPortList)) {
            log.info("The bond port is not exist in DeviceManager.");
            return;
        }
        for (BondPortRes bondPortRer : dmBondPortList) {
            ServicePortPo existServicePortPo = new ServicePortPo();
            BondPortPo existBondPortPo = new BondPortPo();
            existBondPortPo.setName(bondPortRer.getName());
            existBondPortPo.setPortNameList(Arrays.asList(bondPortRer.getBondInfo().split(COMMA)));
            existServicePortPo.setBondPort(existBondPortPo);
            checkIsValidPortNameAndPortNameList(newLogicPort, existServicePortPo);
        }
    }

    /**
     * 校验绑定端口名称和端口列表，要么都相同，是复用会直接返回；要么都不同，是新建；其他情况合理报错
     *
     * @param newLogicPort 新创建的逻辑端口
     * @param existLogicPort 已有的逻辑端口
     */
    @Override
    public void checkIsValidPortNameAndPortNameList(LogicPortDto newLogicPort, ServicePortPo existLogicPort) {
        List<String> existBondPortNameList = existLogicPort.getBondPort().getPortNameList();
        String existBondPortName = existLogicPort.getBondPort().getName();
        List<String> newPortNameList = newLogicPort.getBondPort().getPortNameList();
        String newBondPortName = newLogicPort.getBondPort().getName();
        log.info("Start to check new add logic bond port name : {} with exist bond port name : {} , "
                + "and new add logic port name list: {} with exist logic port name list : {}", newBondPortName,
            existBondPortName, newPortNameList, existBondPortNameList);

        // 绑定端口列表相同，绑定端口名称不同，报 选择的以太网口{0}已被其他绑定端口占用
        if (isSamePortList(existBondPortNameList, newPortNameList) && !isSameBondPortName(existLogicPort.getBondPort(),
            newLogicPort.getBondPort())) {
            log.error("The new add logic port {} has the same bond port list {} but different bond port name {} "
                    + "with exist bond port name: {} and bond port list: {}.", newLogicPort.getName(), newPortNameList,
                newBondPortName, existBondPortName, existBondPortNameList);
            throw new LegoCheckedException(CommonErrorCode.ERROR_DIFFERENT_BOND_PORT_NAME,
                new String[] {newBondPortName},
                "The new add logic port has the same port list but different port name with "
                    + "exist port list and bond port name.");
        }
        // 绑定端口列表不同，绑定端口名称相同，报 绑定端口名称在底座已存在；
        if (!isSamePortList(existBondPortNameList, newPortNameList) && isSameBondPortName(existLogicPort.getBondPort(),
            newLogicPort.getBondPort())) {
            log.error("The new add logic port {} has the same bond port name {} but different bond port list {} "
                    + "with exist bond port name: {} and bond port list: {}.", newLogicPort.getName(), newBondPortName,
                newPortNameList, existBondPortName, existBondPortNameList);
            throw new LegoCheckedException(CommonErrorCode.ERROR_BOND_PORT_NAME,
                "The new add logic port has the same bond port name but different bond port list with "
                    + "exist bond port name and bond port list.");
        }
    }

    private boolean isSameBondPortName(BondPortPo existBondPort, BondPortPo newBondPort) {
        return StringUtils.equals(existBondPort.getName(), newBondPort.getName());
    }

    private boolean isSamePortList(List<String> existPortNameList, List<String> newPortNameList) {
        if (existPortNameList.size() != newPortNameList.size()) {
            return false;
        }
        for (String portName : newPortNameList) {
            if (!existPortNameList.contains(portName)) {
                return false;
            }
        }
        return true;
    }

    private void waitBondPortLinkup(BondPortRes bondPortRes) {
        // 等待6s 尝试获取底座绑定端口是否已连接
        CommonUtil.sleep(6, TimeUnit.SECONDS);
        int count = 0;
        while (!isBondPortLinkup(bondPortRes)) {
            CommonUtil.sleep(2, TimeUnit.SECONDS);
            count++;
            log.info("retry wait bond port linkup count:{}", count);
            if (count > MAX_RETRY_COUNT) {
                log.error("wait bond port Linkup time out");
                break;
            }
        }
    }

    private boolean isBondPortLinkup(BondPortRes bondPortRes) {
        DeviceManagerResponse<List<BondPortRes>> bondPortResList = netWorkPortService.getBondPort(getDeviceId(),
            getUsername());
        BondPortRes existBondPort = bondPortResList.getData()
            .stream()
            .filter(bondPort -> bondPort.getId().equals(bondPortRes.getId()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "bond port not exist"));
        return existBondPort.getRunningStatus() == RunningStatus.LINKUP;
    }

    private BondPortRes userCreateBondPort(String bondPortName, List<String> portIdList) {
        BondPortAddRequest bondPortAddRequest = new BondPortAddRequest(bondPortName, portIdList);
        return netWorkPortService.addBondPort(getDeviceId(), getUsername(), bondPortAddRequest).getData();
    }

    private BondPort addEthTypeLogicPort(LogicPortDto logicPort) {
        // 配置逻辑端口的端口id
        logicPort.setHomePortId(portFactory.createPort(logicPort.getHomePortType())
                .queryHomePortId(logicPort.getHomePortName()));
        return createLogicPortByRole(logicPort);
    }

    private BondPort createLogicPortByRole(LogicPortDto logicPort) {
        if (VerifyUtil.isEmpty(logicPort.getDmRole())) {
            logicPort.setDmRole(logicPort.getRole());
        }
        // 增加 归档的逻辑端口
        BondPort result;
        switch (logicPort.getDmRole()) {
            case SERVICE:
                // 增加 角色为数据-备份，数据协议为NFS+CIFS的逻辑端口;
                log.info("Create backup logic port info : {}.",
                    JSONObject.writeValueAsString(logicPort.copyToBackupLogicPort()));
                result = netWorkPortService.addLogicPort(getDeviceId(), getUsername(),
                    logicPort.copyToBackupLogicPort()).getData();
                break;
            case TRANSLATE:
                // 增加 角色为复制，数据协议为--的逻辑端口;
                log.info("Create replication logic port info : {}.",
                    JSONObject.writeValueAsString(logicPort.copyToDuplicateLogicPort()));
                result = netWorkPortService.addLogicPort(getDeviceId(), getUsername(),
                    logicPort.copyToDuplicateLogicPort()).getData();
                break;
            case ARCHIVE:
                // 增加 角色为归档，数据协议为--的逻辑端口;
                log.info("Create archive logic port info: {}.",
                    JSONObject.writeValueAsString(logicPort.copyToArchiveLogicPort()));
                result = netWorkPortService.addLogicPort(getDeviceId(), getUsername(),
                    logicPort.copyToArchiveLogicPort()).getData();
                break;
            case MANAGEMENT_SERVICE:
                // op服务化场景，会有类型是3(管理+数据)的角色，单独分析角色为3的逻辑端口
                result = handleHcsServiceManagementService(logicPort);
                break;
            default:
                log.error("Create logic port: {} failed, port role error.", logicPort.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Create logic port failed, port role error.");
        }
        resetX9000ControllerId(logicPort, result);
        return result;
    }

    private BondPort handleHcsServiceManagementService(LogicPortDto logicPort) {
        if (OpServiceUtil.isHcsService()) {
            log.info("Create management service role logic port info : {}.",
                JSONObject.writeValueAsString(logicPort.copyToBackupLogicPort()));
            return netWorkPortService.addLogicPort(getDeviceId(), getUsername(), logicPort.copyToBackupLogicPort())
                .getData();
        } else {
            log.error("Create logic port: {} failed, port role error.", logicPort.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Create logic port failed, port role error.");
        }
    }

    private void resetX9000ControllerId(LogicPortDto logicPort, BondPort result) {
        // 升级场景，由于 X9000 具有网卡共享特性，端口创建之后还需确认一下是否是之前的控制器id
        logicPort.setId(result.getId());
        if (deployTypeList.contains(deployTypeService.getDeployType().getValue())
            && InitNetworkConfigConstants.modifyControllerRoles.contains(logicPort.getRole())) {
            LogicPortAddRequest modifyLogicPortInfo = new LogicPortAddRequest();
            modifyLogicPortInfo.setHomeControllerId(logicPort.getHomeControllerId());
            log.info("The current modify logic port id is : {}, new set HomeControllerId is : {}", logicPort.getId(),
                modifyLogicPortInfo.getHomeControllerId());
            netWorkPortService.modifyLogicPortById(getDeviceId(), getUsername(), logicPort.getId(),
                modifyLogicPortInfo);
        }
    }

    private void addMemberToFailOverGroup(String associateObjId, FailovergroupMemberType associateObjType,
            String failOverGroupId) {
        FailovergroupMember failovergroupMember = new FailovergroupMember();
        failovergroupMember.setAssociateObjId(associateObjId);
        failovergroupMember.setAssociateObjType(associateObjType);
        failovergroupMember.setId(failOverGroupId);
        netWorkPortService.addMemberOfFailovergroup(getDeviceId(), getUsername(), failovergroupMember);
    }

    private FailoverGroupResponse createFailOverGroup() {
        Failovergroup failovergroup = new Failovergroup();
        failovergroup.setFailovergroupType(FAILOVERGROUP_TYPE);
        String failovergroupName = InitNetworkConfigConstants.FAIL_OVER_GROUP_PREFIX + System.currentTimeMillis();
        failovergroup.setName(failovergroupName);
        return netWorkPortService.createFailovergroup(getDeviceId(), getUsername(), failovergroup).getData();
    }

    /**
     * 获取逻辑端口列表
     *
     * @return 逻辑端口列表
     */
    @Override
    public List<LogicPortAddRequest> getLogicPort() {
        return netWorkPortService.queryLogicPorts(getDeviceId(), getUsername()).getData();
    }

    /**
     * 修改逻辑端口
     *
     * @param name 待修改逻辑端口名称
     * @param modifyLogicPortRequest 待修改逻辑端口请求体
     * @return bondPort 绑定端口的返回
     */
    @Override
    public BondPort modifyLogicPort(String name, ModifyLogicPortDto modifyLogicPortRequest) {
        log.info("Modify logic port:{}, id:{}, name:{}, ip:{}, mask:{}, gateway:{}, addressFamily:{}", name,
                modifyLogicPortRequest.getId(), modifyLogicPortRequest.getName(), modifyLogicPortRequest.getIp(),
                modifyLogicPortRequest.getMask(), modifyLogicPortRequest.getGateWay(),
                modifyLogicPortRequest.getAddressFamily());
        savePortRole(modifyLogicPortRequest.getId(), name);
        LogicPortAddRequest modifyLogicParam = new LogicPortAddRequest();
        BondPort result = new BondPort();
        if (VerifyUtil.isEmpty(modifyLogicPortRequest.getId())) {
            return result;
        }
        fillModifyLogicParam(name, modifyLogicPortRequest, modifyLogicParam);
        result = netWorkPortService.modifyLogicPortById(getDeviceId(), getUsername(), modifyLogicPortRequest.getId(),
            modifyLogicParam).getData();
        updateDb(name, modifyLogicPortRequest);
        return result;
    }

    private void savePortRole(String id, String name) {
        if (isDbExistPortRole(name)) {
            log.info("No need to save port role, isDbExistPortRole: {}", isDbExistPortRole(name));
            return;
        }
        LogicPortAddRequest logicPort = getLogicPort().stream()
                .filter(port -> StringUtils.equals(port.getId(), id))
                .findFirst()
                .orElseThrow(() -> {
                    log.error("Port: {} not exist, system error.", id);
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Port not exist, system error.");
                });
        ServicePortPo servicePort = getServicePort(name);
        servicePort.setRole(getPortRole(logicPort));
        updateInitConfigByTypeAndEsn(name, JSONObject.writeValueAsString(servicePort), getDeviceId());
    }

    private ServicePortPo getServicePort(String name) {
        InitConfigInfo initConfigInfo = queryInitConfigByTypeAndEsn(name, getDeviceId());
        return JSONObject.fromObject(initConfigInfo.getInitValue())
                        .toBean(ServicePortPo.class);
    }

    private PortRole getPortRole(LogicPortAddRequest logicPort) {
        PortRole result = null;
        // 从根据基础设施中的Role设置 -> 从数据库中获取Role
        if (getBackupIps().contains(getLogicPortIp(logicPort))) {
            result = PortRole.SERVICE;
        }
        if (getArchiveIps().contains(getLogicPortIp(logicPort))) {
            result = PortRole.ARCHIVE;
        }
        if (getReplicationIps().contains(getLogicPortIp(logicPort))) {
            result = PortRole.TRANSLATE;
        }
        return result;
    }

    private String getLogicPortIp(LogicPortAddRequest logicPort) {
        if (AddressFamily.IPV6.equals(logicPort.getAddressFamily())) {
            return logicPort.getIpv6Addr();
        }
        return logicPort.getIpv4Addr();
    }

    private boolean isDbExistPortRole(String name) {
        return !VerifyUtil.isEmpty(getServicePort(name).getRole());
    }

    private void fillModifyLogicParam(String name, ModifyLogicPortDto modifyLogicPortRequest,
        LogicPortAddRequest modifyLogicParam) {
        modifyLogicParam.setName(modifyLogicPortRequest.getName());
        if (VerifyUtil.isEmpty(modifyLogicPortRequest.getName())
            || StringUtils.equals(name, modifyLogicPortRequest.getName())) {
            modifyLogicParam.setName(null);
        }
        modifyLogicParam.setId(modifyLogicPortRequest.getId());
        if (PortRole.SERVICE.equals(modifyLogicPortRequest.getRole())) {
            modifyLogicParam.setIsFailOver(modifyLogicPortRequest.getIsFailOver());
            if (Boolean.TRUE.equals(modifyLogicPortRequest.getIsFailOver())) {
                modifyLogicParam.setFailoverGroupId(modifyLogicPortRequest.getFailoverGroupId());
            }
        }
        if (AddressFamily.IPV4.equals(modifyLogicPortRequest.getAddressFamily())) {
            modifyLogicParam.setIpv4Addr(modifyLogicPortRequest.getIp());
            modifyLogicParam.setIpv4Mask(modifyLogicPortRequest.getMask());
            modifyLogicParam.setIpv4Gateway(modifyLogicPortRequest.getGateWay());
        } else {
            modifyLogicParam.setIpv6Addr(modifyLogicPortRequest.getIp());
            modifyLogicParam.setIpv6Mask(modifyLogicPortRequest.getMask());
            modifyLogicParam.setIpv6Gateway(modifyLogicPortRequest.getGateWay());
        }
        modifyLogicParam.setAddressFamily(modifyLogicPortRequest.getAddressFamily());
    }

    private void updateDb(String name, ModifyLogicPortDto logicPortAddRequest) {
        InitConfigInfo needToModifyInitConfig = queryInitConfigByTypeAndEsn(name, getDeviceId());
        ServicePortPo needToModifyServicePort = JSONObject.fromObject(needToModifyInitConfig.getInitValue())
                .toBean(ServicePortPo.class);
        String logicPortName = VerifyUtil.isEmpty(logicPortAddRequest.getName()) ? name : logicPortAddRequest.getName();
        needToModifyServicePort.setName(logicPortName);
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(name, getDeviceId());
        insertInitConfig(needToModifyServicePort.getName(),
                JSONObject.writeValueAsString(needToModifyServicePort), getDeviceId());
        InitConfigInfo queryLogicPortNameListConfig = queryInitConfigByTypeAndEsn(
                Constants.LOGIC_PORTS_CREATED_BY_USER, getDeviceId());
        List<String> logicPortNameList = JSONArray.fromObject(queryLogicPortNameListConfig.getInitValue())
                .toBean(String.class);
        logicPortNameList.remove(name);
        logicPortNameList.add(needToModifyServicePort.getName());
        updateInitConfigByTypeAndEsn(Constants.LOGIC_PORTS_CREATED_BY_USER,
                JSONObject.writeValueAsString(logicPortNameList), getDeviceId());
    }
    @Override
    public InitConfigInfo queryInitConfigByTypeAndEsn(String initType, String esn) {
        return initNetworkConfigMapper.queryInitConfigByEsnAndType(initType, esn).stream().findFirst()
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                        "Query init config failed, config is deleted or not exist."));
    }

    private void updateInitConfigByTypeAndEsn(String name, String needToModifyServicePort, String esn) {
        InitConfigInfo updateInitConfigParam = new InitConfigInfo();
        updateInitConfigParam.setInitType(name);
        updateInitConfigParam.setInitValue(needToModifyServicePort);
        updateInitConfigParam.setEsn(esn);
        initNetworkConfigMapper.updateInitConfigByEsnAndType(updateInitConfigParam);
    }

    /**
     * 删除逻辑端口
     *
     * @param name 待删除逻辑端口名称
     */
    @Override
    public void deleteLogicPort(String name) {
        if (isOnlyDeleteLogicPort(name)) {
            log.info("Only need to delete db.");
            deleteReuseLogicPortFromDb(name);
            return;
        }
        // 从数据库中查询逻辑端口信息
        InitConfigInfo logicPortConfig = queryInitConfigByTypeAndEsn(name, getDeviceId());
        ServicePortPo existServicePort = JSONObject.fromObject(logicPortConfig.getInitValue())
                .toBean(ServicePortPo.class);
        log.info("Delete logic port, name:{}, id:{}, homePortType:{}", existServicePort.getName(),
                existServicePort.getId(), existServicePort.getHomePortType());
        if (!VerifyUtil.isEmpty(existServicePort.getId())) {
            deletePortOfDm(name, existServicePort);
        }
        deletePmCreateLogicPortFromDb(name);
    }

    private void deletePmCreateLogicPortFromDb(String name) {
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(name, getDeviceId());
        deleteObjectFromSpecifyInitConfig(name, Constants.LOGIC_PORTS_CREATED_BY_USER);
    }

    private void deleteObjectFromSpecifyInitConfig(String name, String initType) {
        InitConfigInfo logicPortsInitConfig = queryInitConfigByTypeAndEsn(initType, getDeviceId());
        List<String> logicPortNameList = JSONArray.fromObject(logicPortsInitConfig.getInitValue()).toBean(String.class);
        logicPortNameList.remove(name);
        updateInitConfigByTypeAndEsn(initType, JSONObject.writeValueAsString(logicPortNameList), getDeviceId());
    }

    private void deletePortOfDm(String name, ServicePortPo existServicePort) {
        deleteLogicPortByName(name);
        try {
            // 删除逻辑端口之后再删除vlan,然后再删除绑定端口
            deletePhysicPort(existServicePort);
        } catch (LegoCheckedException | DecodeException | DeviceManagerException e) {
            log.error("Delete vlan port failed.", ExceptionUtil.getErrorMessage(e));
        } catch (Exception e) {
            log.error("Delete bond port failed.", ExceptionUtil.getErrorMessage(e));
        }
    }
    @Override
    public void deleteSingleLogicPortOfDm(String name, ServicePortPo existServicePort) {
        deleteLogicPortByName(name);
    }

    @Override
    public void addSingleLogicPortOfDm(LogicPortDto logicPortDto) {
        BondPort bondPort = createLogicPortByRole(logicPortDto);
        log.info("Create logic port success, bondPortIdList:{}",
            JSONObject.writeValueAsString(bondPort.getPortIdList()));
    }

    private void deleteLogicPortByName(String name) {
        try {
            netWorkPortService.deleteLogicPort(getDeviceId(), getUsername(), name);
        } catch (LegoCheckedException e) {
            if (e.getErrorCode() == RETURN_OBJ_NOT_EXIST) {
                log.error("The logic port: {} does not exist in dm.", name);
                return;
            }
            throw e;
        }
    }

    private void deleteReuseLogicPortFromDb(String name) {
        deleteObjectFromSpecifyInitConfig(name, Constants.REUSE_LOGIC_PORTS);
        deletePmCreateLogicPortFromDb(name);
    }

    private boolean isOnlyDeleteLogicPort(String name) {
        InitConfigInfo initConfigInfo = initNetworkConfigMapper.queryInitConfigByEsnAndType(Constants.REUSE_LOGIC_PORTS,
                        getDeviceId()).stream()
                .findFirst()
                .orElse(new InitConfigInfo());
        List<String> reuseLogicPortNameList = JSONArray.fromObject(initConfigInfo.getInitValue()).toBean(String.class);
        return reuseLogicPortNameList.contains(name);
    }

    private void deletePhysicPort(ServicePortPo existServicePort) {
        // 删vlan
        if (existServicePort.getHomePortType().equalsHomePortType(HomePortType.VLAN.getHomePortType())) {
            log.info("Delete vlan, id:{}", existServicePort.getVlan().getId());
            netWorkPortService.deleteVlan(getDeviceId(), getUsername(), existServicePort.getVlan().getId());
        }
        // 删vlan创建的绑定端口
        if (!VerifyUtil.isEmpty(existServicePort.getVlan())
            && existServicePort.getVlan().getPortType().equalsVlanPortType(VlanPortType.BOND.getVlanPortType())) {
            log.info("Delete bond port of vlan, bond port id:{}", existServicePort.getVlan().getBondPortId());
            netWorkPortService.deleteBondPort(getDeviceId(), getUsername(), existServicePort.getVlan().getBondPortId());
        }
        // 删绑定端口
        if (existServicePort.getHomePortType().equalsHomePortType(HomePortType.BINDING.getHomePortType())) {
            log.info("Delete bond port, id:{}", existServicePort.getBondPort().getId());
            netWorkPortService.deleteBondPort(getDeviceId(), getUsername(), existServicePort.getBondPort().getId());
        }
    }

    /**
     * 根据过滤条件获取端口列表
     *
     * @param condition 过滤条件
     * @return 所有的端口信息
     */
    @Override
    public AllPortListResponseDto getPorts(LogicPortFilterParam condition) {
        log.info("Query port info filter condition portName: {}, portId: {}, ethLogicTypeValue: {}",
                condition.getPortName(), condition.getPortId(), condition.getEthLogicTypeValue());

        List<LogicPortAddRequest> dmLogicPortList = getLogicPort();
        List<LogicPortDto> logicPortList = getLogicPortsCreatedByUser(dmLogicPortList);
        if (StringUtils.isNotEmpty(condition.getPortName()) || StringUtils.isNotEmpty(condition.getPortId())) {
            // 根据过滤条件查询逻辑端口
            logicPortList = logicPortList.stream()
                .filter(port -> StringUtils.equals(port.getName(), condition.getPortName())
                    || StringUtils.equals(port.getId(), condition.getPortId()))
                .collect(Collectors.toList());
        }
        boolean isAllLogicPortSaved = isAllLogicPortSaved(logicPortList);
        String ethFilter = Optional.ofNullable(condition.getEthLogicTypeValue())
            .orElse(LogicType.SERVICE_PORT.getRole());
        List<EthPortDto> ethPortsList = getEthPort(ethFilter);
        List<BondPortDto> bondPortList = getBondPort();
        List<VlanPo> vlanPoList = getVlan();

        AllPortListResponseDto allPortListResponseDto = new AllPortListResponseDto();
        allPortListResponseDto.setBondPortList(bondPortList);
        allPortListResponseDto.setLogicPortDtoList(logicPortList);
        allPortListResponseDto.setDmLogicPortList(dmLogicPortList);
        allPortListResponseDto.setEthPortDtoList(ethPortsList);
        allPortListResponseDto.setVlanList(vlanPoList);
        InitConfigInfo initConfigInfo = initNetworkConfigMapper
                .queryInitConfigByEsnAndType(Constants.REUSE_LOGIC_PORTS, getDeviceId())
                        .stream().findFirst()
                        .orElse(new InitConfigInfo());
        allPortListResponseDto.setReuseLogicPortNameList(JSONArray.fromObject(initConfigInfo.getInitValue())
                .toBean(String.class));
        allPortListResponseDto.setAllLogicPortsValid(isAllLogicPortSaved);
        return allPortListResponseDto;
    }

    private boolean isAllLogicPortSaved(List<LogicPortDto> logicPortList) {
        for (LogicPortDto logicPort : logicPortList) {
            if (!logicPort.isValid()) {
                return false;
            }
        }
        Set<String> logicIplist = logicPortList.stream().map(LogicPortDto::getIp).collect(Collectors.toSet());
        List<String> allNetPlaneIps = getAllNetIps();
        for (String netPlane : allNetPlaneIps) {
            if (!logicIplist.contains(netPlane)) {
                return false;
            }
        }
        return true;
    }

    private List<VlanPo> getVlan() {
        return netWorkPortService.queryVlan(getDeviceId(), getUsername()).getData().stream().map(vlanInfo -> {
            VlanPo vlanPo = new VlanPo();
            BeanUtils.copyProperties(vlanInfo, vlanPo);
            vlanPo.setTags(Collections.singletonList(vlanInfo.getTag()));
            String bondPortId = vlanInfo.getPortType().isBondPort() ? vlanInfo.getPortId() : StringUtils.EMPTY;
            if (VerifyUtil.isEmpty(bondPortId)) {
                vlanPo.setPortNameList(Collections.singletonList(portFactory.createPort(HomePortType.ETHERNETPORT)
                        .queryHomePortName(vlanInfo.getPortId())));
            } else {
                String portNameStr = netWorkPortService.getBondPort(getDeviceId(), getUsername()).getData().stream()
                        .filter(bondPortRes -> bondPortRes.getId().equals(bondPortId))
                        .findFirst()
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                                "Bond port not exist"))
                        .getBondInfo();
                List<String> portName = Arrays.stream(portNameStr.split(",")).collect(Collectors.toList());
                vlanPo.setPortNameList(portName);
            }
            vlanPo.setBondPortId(bondPortId);
            return vlanPo;
        }).collect(Collectors.toList());
    }

    private void fillRoleFromDb(List<LogicPortDto> logicPortList, List<ServicePortPo> dbPortList) {
        Map<String, PortRole> dbPortMapFitered = dbPortList.stream().filter(port -> !VerifyUtil.isEmpty(port.getRole()))
                .collect(Collectors.toMap(ServicePortPo::getId, ServicePortPo::getRole));
        logicPortList.forEach(port -> {
            if (dbPortMapFitered.containsKey(port.getId())) {
                port.setRole(dbPortMapFitered.get(port.getId()));
            }
        });
    }

    private void fillRoleFromConfig(List<LogicPortDto> logicPortList) {
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        List<String> backupIps = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
        List<String> replicationIps = new ArrayList<>(networkService.getNetPlaneIp(
            deviceNetworkInfo.getReplicationConfig()));
        List<String> archiveIps = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getArchiveConfig()));
        logicPortList.stream().filter(port -> backupIps.contains(port.getIp()))
            .forEach(port -> port.setRole(PortRole.SERVICE));
        logicPortList.stream().filter(port -> replicationIps.contains(port.getIp()))
            .forEach(port -> port.setRole(PortRole.TRANSLATE));
        logicPortList.stream().filter(port -> archiveIps.contains(port.getIp()))
            .forEach(port -> port.setRole(PortRole.ARCHIVE));
    }

    private List<BondPortDto> getBondPort() {
        return netWorkPortService.getBondPort(getDeviceId(), getUsername()).getData().stream().map(bondPortRes -> {
            BondPortDto bondPortDto = new BondPortDto();
            BeanUtils.copyProperties(bondPortRes, bondPortDto);
            List<String> strings = JSONArray.fromObject(bondPortRes.getPortIdList()).toBean(String.class);
            bondPortDto.setPortIdList(strings);
            return bondPortDto;
        }).collect(Collectors.toList());
    }

    @Override
    public PortRouteInfo addPortRoute(ModifyLogicPortRouteRequest request) {
        request.getRoute().setPortId(getPortIdByName(request.getPortName()));
        DeviceManagerResponse<PortRouteInfo> response = netWorkPortService.addRoute(getDeviceId(), getUsername(),
            request.getRoute());
        dealDmResponse(response);
        return response.getData();
    }

    @Override
    public void deletePortRoute(ModifyLogicPortRouteRequest request) {
        request.getRoute().setPortId(getPortIdByName(request.getPortName()));
        DeviceManagerResponse<PortRouteInfo> response = netWorkPortService.deleteRoute(getDeviceId(), getUsername(),
            request.getRoute());
        dealDmResponse(response);
    }

    @Override
    public List<PortRouteInfo> getPortRouteInfo(String portName) {
        DeviceManagerResponse<List<PortRouteInfo>> portRoutes = netWorkPortService.getRoute(getDeviceId(),
            getUsername(), getPortIdByName(portName));
        dealDmResponse(portRoutes);
        return portRoutes.getData();
    }

    /**
     * 获取用户创建的逻辑端口
     *
     * @return 用户创建的逻辑端口
     */
    @Override
    public List<LogicPortDto> getLogicPortsCreatedByUser() {
        return this.getLogicPortsCreatedByUser(getLogicPort());
    }

    /**
     * 获取用户创建的逻辑端口
     *
     * @param dmLogicPortList 底座逻辑端口列表
     * @return 用户创建的逻辑端口
     */
    @Override
    public List<LogicPortDto> getLogicPortsCreatedByUser(List<LogicPortAddRequest> dmLogicPortList) {
        writePortsToDbFromConfigWhenUpdateBeforeSix();
        List<ServicePortPo> dbServicePortList = getServicePortsFromDb();
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        return getLogicPortList(dbServicePortList, dmLogicPortList, deviceNetworkInfo);
    }

    private List<ServicePortPo> getServicePortsFromDb() {
        String logicPortNameStrFromDb = initNetworkConfigMapper
            .queryInitConfigByEsnAndType(Constants.LOGIC_PORTS_CREATED_BY_USER, getDeviceId()).stream().findFirst()
            .orElse(new InitConfigInfo()).getInitValue();
        if (VerifyUtil.isEmpty(logicPortNameStrFromDb)) {
            return Collections.emptyList();
        }
        List<String> logicPortNameList = JSONArray.fromObject(logicPortNameStrFromDb).toBean(String.class);
        return logicPortNameList.stream().map(name -> {
            String portStr = queryInitConfigByTypeAndEsn(name, getDeviceId()).getInitValue();
            return JSONObject.fromObject(portStr).toBean(ServicePortPo.class);
        }).collect(Collectors.toList());
    }

    private List<LogicPortDto> getLogicPortList(List<ServicePortPo> dbServicePortList,
        List<LogicPortAddRequest> dmLogicPortList, DeviceNetworkInfo deviceNetworkInfo) {
        Map<String, LogicPortAddRequest> dmLogicPortMap = dmLogicPortList.stream()
            .collect(Collectors.toMap(LogicPortAddRequest::getName, logicPortAddRequest -> logicPortAddRequest));
        Map<String, LogicPortAddRequest> dmIdLogicPortMap = dmLogicPortList.stream()
            .collect(Collectors.toMap(LogicPortAddRequest::getId, logicPortAddRequest -> logicPortAddRequest));
        Map<String, PortRole> dbPortMapFitered = dbServicePortList.stream()
            .filter(port -> !VerifyUtil.isEmpty(port.getRole()))
            .collect(Collectors.toMap(ServicePortPo::getId, ServicePortPo::getRole));
        Map<String, NetWorkIpRoute> netWorkIpRouteMap = getAllNetWorkIpRouteMap(deviceNetworkInfo);

        return dbServicePortList.stream().map(servicePort -> {
            LogicPortDto logicPort = new LogicPortDto();
            logicPort.setName(servicePort.getName());
            fillPropertiesFromDm(dmLogicPortMap, logicPort);
            fillPropertiesFromCm(deviceNetworkInfo, logicPort);
            fillPropertiesFromDb(dbPortMapFitered, logicPort);
            fillIsWorkedFromAll(servicePort, dmIdLogicPortMap, netWorkIpRouteMap, logicPort);
            mappingProperties(logicPort);
            return logicPort;
        }).collect(Collectors.toList());
    }

    /**
     * 获取网络配置中 ip 和 route的映射关系
     *
     * @param deviceNetworkInfo deviceNetworkInfo
     * @return 网络配置中 ip 和 route的映射关系
     */
    public Map<String, NetWorkIpRoute> getAllNetWorkIpRouteMap(DeviceNetworkInfo deviceNetworkInfo) {
        List<NetWorkLogicIp> netPlaneLogicIps = getAllNetPlaneLogicIps(deviceNetworkInfo);
        Map<String, NetWorkIpRoute> netWorkIpRouteMap = new HashMap<>();
        Map<String, List<NetWorkRouteInfo>> allNetPlaneIpRouteMap = getAllNetPlaneIpRouteMap(deviceNetworkInfo);
        netPlaneLogicIps.forEach(netPlaneLogicIp -> {
            NetWorkIpRoute ipRoute = new NetWorkIpRoute();
            ipRoute.setNetWorkLogicIp(netPlaneLogicIp);
            if (allNetPlaneIpRouteMap.containsKey(netPlaneLogicIp.getIp())) {
                ipRoute.setRoutes(allNetPlaneIpRouteMap.get(netPlaneLogicIp.getIp()));
            } else {
                ipRoute.setRoutes(new ArrayList<>());
            }
            netWorkIpRouteMap.put(netPlaneLogicIp.getIp(), ipRoute);
        });
        return netWorkIpRouteMap;
    }

    private List<NetWorkLogicIp> getAllNetPlaneLogicIps(DeviceNetworkInfo deviceNetworkInfo) {
        List<NetWorkLogicIp> netWorkLogicIps = new ArrayList<>(
            networkService.getNetPlaneIpList(deviceNetworkInfo.getBackupConfig()));
        List<NetWorkLogicIp> relicationLogicIpList = networkService.getNetPlaneIpList(
            deviceNetworkInfo.getReplicationConfig());
        List<NetWorkLogicIp> archiveLogicIpList = networkService.getNetPlaneIpList(
            deviceNetworkInfo.getArchiveConfig());
        netWorkLogicIps.addAll(relicationLogicIpList);
        netWorkLogicIps.addAll(archiveLogicIpList);
        return netWorkLogicIps;
    }

    private void fillIsWorkedFromAll(ServicePortPo dbServicePort, Map<String, LogicPortAddRequest> dmIdLogicPortMap,
        Map<String, NetWorkIpRoute> netWorkIpRouteMap, LogicPortDto logicPort) {
        List<NetWorkRouteInfo> dmRouteList = getDmRouteInfos(logicPort);
        String dmIp;
        String dmMask;
        String dmGateway;
        if (dmIdLogicPortMap.containsKey(dbServicePort.getId())) {
            LogicPortAddRequest logicPortAddRequest = dmIdLogicPortMap.get(dbServicePort.getId());
            log.info("DeviceManager logic port name is : {}, id : {}", logicPortAddRequest.getName(),
                logicPortAddRequest.getId());
            if (VerifyUtil.isEmpty(logicPortAddRequest.getIpv4Addr())) {
                dmIp = logicPortAddRequest.getIpv6Addr();
                dmMask = logicPortAddRequest.getIpv6Mask();
                dmGateway = logicPortAddRequest.getIpv6Gateway();
            } else {
                dmIp = logicPortAddRequest.getIpv4Addr();
                dmMask = logicPortAddRequest.getIpv4Mask();
                dmGateway = logicPortAddRequest.getIpv4Gateway();
            }

            if (!netWorkIpRouteMap.containsKey(dmIp)) {
                log.info("The current logic port is not contains in network config, ip is : {}", dmIp);
                logicPort.setValid(false);
                return;
            }
            NetWorkIpRoute ipRoute = netWorkIpRouteMap.get(dmIp);
            if (!isSameIpAndMask(ipRoute.getNetWorkLogicIp(), dmIp, dmMask)) {
                log.info(
                    "The current logic port is not valid, because ip or mask not contains in network plane, ip is : {}",
                    dmIp);
                logicPort.setValid(false);
                return;
            }
            // 逻辑端口配置网关表示有默认路由
            if (StringUtils.isNotEmpty(dmGateway)) {
                NetWorkRouteInfo netWorkRouteInfo = new NetWorkRouteInfo();
                netWorkRouteInfo.setType(RouteType.DEFAULT.getRouteType());
                netWorkRouteInfo.setGateway(dmGateway);
                netWorkRouteInfo.setDestination("0.0.0.0");
                netWorkRouteInfo.setMask("0.0.0.0");
                dmRouteList.add(netWorkRouteInfo);
            }
            dmRouteList.sort(Comparator.comparing(NetWorkRouteInfo::hashCode));
            ipRoute.getRoutes().sort(Comparator.comparing(NetWorkRouteInfo::hashCode));
            if (dmRouteList.toString().equals(ipRoute.getRoutes().toString())) {
                logicPort.setValid(true);
                log.info("The current logic port is valid, ip is : {}", dmIp);
                return;
            }
            log.info("The current logic port is not valid, ip is : {}", dmIp);
            logicPort.setValid(false);
        }
        log.info("DeviceManager is not contains logic port id : {}", dbServicePort.getId());
    }

    private static boolean isSameIpAndMask(NetWorkLogicIp netPlane, String dmIp, String dmMask) {
        return netPlane.getIp().equals(dmIp) && netPlane.getMask().equals(dmMask);
    }

    private Map<String, List<NetWorkRouteInfo>> getAllNetPlaneIpRouteMap(DeviceNetworkInfo deviceNetworkInfo) {
        Map<String, List<NetWorkRouteInfo>> allNetPlaneIpRouteMap = new HashMap<>(networkService.getNetPlaneIpRouteList(
            deviceNetworkInfo.getBackupConfig()));
        if (!CollectionUtils.isEmpty(deviceNetworkInfo.getReplicationConfig())) {
            allNetPlaneIpRouteMap.putAll(networkService.getNetPlaneIpRouteList(
                deviceNetworkInfo.getReplicationConfig()));
        }
        if (!CollectionUtils.isEmpty(deviceNetworkInfo.getArchiveConfig())) {
            allNetPlaneIpRouteMap.putAll(networkService.getNetPlaneIpRouteList(
                deviceNetworkInfo.getArchiveConfig()));
        }
        return allNetPlaneIpRouteMap;
    }

    private List<NetWorkRouteInfo> getDmRouteInfos(LogicPortDto logicPort) {
        List<PortRouteInfo> dmRouteInfo = getPortRouteInfo(logicPort.getName());
        return dmRouteInfo.stream()
            .map(PortRouteInfo::convertToNetWorkRouteInfo)
            .collect(Collectors.toList());
    }

    private void mappingProperties(LogicPortDto logicPort) {
        // 底座数据类型（2）、管理+数据类型（3）逻辑端口，OP统一展示为备份类型逻辑端口
        // 底座归档类型（11）并且是服务化场景，OP显示为备份类型逻辑端口
        // 复制（4）类型逻辑端口保持不变保持不变
        if (PortRole.MANAGEMENT_SERVICE.equals(logicPort.getDmRole())
                || (PortRole.ARCHIVE.equals(logicPort.getDmRole()) && OpServiceUtil.isHcsService())) {
            logicPort.setRole(PortRole.SERVICE);
        }
    }

    private void fillPropertiesFromDm(Map<String, LogicPortAddRequest> dmLogicPortMap, LogicPortDto logicPort) {
        LogicPortAddRequest dmLogicPort = dmLogicPortMap.get(logicPort.getName());
        if (dmLogicPort == null) {
            log.error("Logic port: {} created by user is lost.", logicPort.getName());
            logicPort.setDmExists(false);
            return;
        }
        logicPort.setDmExists(true);
        if (StringUtils.isEmpty(dmLogicPort.getIpv4Addr())) {
            logicPort.setIp(dmLogicPort.getIpv6Addr());
            logicPort.setMask(dmLogicPort.getIpv6Mask());
            logicPort.setGateWay(dmLogicPort.getIpv6Gateway());
            logicPort.setIpType(IpType.IPV6.getValue());
        } else {
            logicPort.setIp(dmLogicPort.getIpv4Addr());
            logicPort.setMask(dmLogicPort.getIpv4Mask());
            logicPort.setGateWay(dmLogicPort.getIpv4Gateway());
            logicPort.setIpType(IpType.IPV4.getValue());
        }
        BeanUtils.copyProperties(dmLogicPort, logicPort);
    }

    private void fillPropertiesFromDb(Map<String, PortRole> dbPortMapFitered, LogicPortDto logicPort) {
        String portStr = queryInitConfigByTypeAndEsn(logicPort.getName(), getDeviceId()).getInitValue();
        ServicePortPo dbLogicPort = JSONObject.fromObject(portStr).toBean(ServicePortPo.class);
        BeanUtils.copyProperties(dbLogicPort, logicPort);
        if (dbPortMapFitered.containsKey(logicPort.getId())) {
            logicPort.setRole(dbPortMapFitered.get(logicPort.getId()));
        }
    }

    private void fillPropertiesFromCm(DeviceNetworkInfo deviceNetworkInfo, LogicPortDto logicPortDto) {
        List<String> backupIps = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
        if (backupIps.contains(logicPortDto.getIp())) {
            logicPortDto.setRole(PortRole.SERVICE);
            return;
        }
        List<String> replicationIps = new ArrayList<>(networkService.getNetPlaneIp(
            deviceNetworkInfo.getReplicationConfig()));
        if (replicationIps.contains(logicPortDto.getIp())) {
            logicPortDto.setRole(PortRole.TRANSLATE);
            return;
        }
        List<String> archiveIps = new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getArchiveConfig()));
        if (archiveIps.contains(logicPortDto.getIp())) {
            logicPortDto.setRole(PortRole.ARCHIVE);
        }
    }


    @Override
    public void writePortsToDbFromConfigWhenUpdateBeforeSix() {
        if (networkService.isDbExistInitConfigInfo()) {
            log.info("No need to write port into db.");
            return;
        }
        log.info("Start write port info into db.");
        writePortInfoIntoDb();
        writePortNameIntoDb();
        log.info("End write port info into db.");
    }

    private void writePortNameIntoDb() {
        List<String> portNames = getServicePortsFromConfig().stream()
                .map(ServicePortPo::getName)
                .collect(Collectors.toList());
        log.info("Port name list: {}", portNames);
        if (VerifyUtil.isEmpty(portNames)) {
            return;
        }
        insertInitConfig(Constants.LOGIC_PORTS_CREATED_BY_USER, JSONObject.writeValueAsString(portNames),
                getDeviceId());
    }

    private void writePortInfoIntoDb() {
        getServicePortsFromConfig().forEach(port -> insertInitConfig(port.getName(),
                JSONObject.writeValueAsString(port), getDeviceId()));
    }

    private List<ServicePortPo> getServicePortsFromConfig() {
        List<String> netConfigIps = getAllNetIps();
        return getLogicPort().stream().filter(logicPort -> isIpOfConfig(netConfigIps, logicPort))
                .map(this::logicPortConvertToServicePort).collect(Collectors.toList());
    }

    private ServicePortPo logicPortConvertToServicePort(LogicPortAddRequest logicPort) {
        ServicePortPo result = new ServicePortPo();
        fillIdAndName(logicPort, result);
        fillPortInfo(logicPort, result);
        fillRoleAndCurrentControllerId(logicPort, result);
        return result;
    }

    private void fillRoleAndCurrentControllerId(LogicPortAddRequest logicPort, ServicePortPo result) {
        if (VerifyUtil.isEmpty(logicPort.getHomeControllerId())) {
            log.error("Logic home port : {} type is error.", logicPort.getHomeControllerId());
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Logic home port type is error.");
        }
        result.setCurrentControllerId(logicPort.getHomeControllerId());
        fillDmRoleFromCm(logicPort, result);
    }

    private void fillDmRoleFromCm(LogicPortAddRequest logicPort, ServicePortPo result) {
        List<String> backupIps = getBackupIps();
        List<String> replicationIps = getReplicationIps();
        List<String> archiveIps = getArchiveIps();
        String ip = VerifyUtil.isEmpty(logicPort.getIpv4Addr()) ? logicPort.getIpv6Addr() : logicPort.getIpv4Addr();
        if (backupIps.contains(ip)) {
            result.setRole(PortRole.SERVICE);
            result.setDmRole(logicPort.getRole());
        } else if (replicationIps.contains(ip)) {
            result.setRole(PortRole.TRANSLATE);
            result.setDmRole(logicPort.getRole());
        } else if (archiveIps.contains(ip)) {
            result.setRole(PortRole.ARCHIVE);
            result.setDmRole(logicPort.getRole());
        } else {
            log.error("Logic port role: {} type is error.", logicPort.getRole());
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Logic role type is error.");
        }
    }

    private void fillPortInfo(LogicPortAddRequest logicPort, ServicePortPo result) {
        if (VerifyUtil.isEmpty(logicPort.getHomePortType())) {
            log.error("Logic port: {} type is error.", logicPort.getName());
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Logic port type is error.");
        }
        result.setHomePortType(logicPort.getHomePortType());

        if (logicPort.getHomePortType().isBondPort()) {
            fillBondInfo(logicPort, result);
            return;
        }
        if (logicPort.getHomePortType().isVlanPort()) {
            fillVlanInfo(logicPort, result);
        }
    }

    private void fillBondInfo(LogicPortAddRequest logicPort, ServicePortPo result) {
        BondPortDto bondPort = getBondPort().stream()
                .filter(port -> StringUtils.equals(port.getId(), logicPort.getHomePortId())).findFirst()
                .orElseThrow(() -> {
                    log.error("Bound ports associated with logical port: {} are missing.", logicPort.getName());
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                            "Bound ports associated with logical port are missing.");
                });
        BondPortPo bondPortPo = new BondPortPo();
        bondPortPo.setMtu(bondPort.getMtu());
        bondPortPo.setId(bondPort.getId());
        bondPortPo.setPortNameList(Arrays.asList(bondPort.getBondInfo().split(COMMA)));
        result.setBondPort(bondPortPo);
    }

    private void fillVlanInfo(LogicPortAddRequest logicPort, ServicePortPo result) {
        VlanInfo configVlan = getVlanList().stream()
                .filter(vlanInfo -> StringUtils.equals(vlanInfo.getId(), logicPort.getHomePortId())).findFirst()
                .orElseThrow(() -> {
                    log.error("Vlan id: {} not exist.", logicPort.getHomePortId());
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Vlan id not exist.");
                });
        VlanPo vlanPo = new VlanPo();
        fillVlanBaseInfo(configVlan, vlanPo);
        if (configVlan.getPortType().isEthPort()) {
            fillEthVlan(configVlan, vlanPo);
            result.setVlan(vlanPo);
            return;
        }
        if (configVlan.getPortType().isBondPort()) {
            fillBondVlan(configVlan, vlanPo);
            result.setVlan(vlanPo);
            BondPortPo bondPortPo = new BondPortPo();
            fillVlanBondInfo(vlanPo, bondPortPo);
            result.setBondPort(bondPortPo);
        }
    }

    private void fillVlanBondInfo(VlanPo configVlan, BondPortPo bondPortPo) {
        bondPortPo.setPortNameList(configVlan.getPortNameList());
        bondPortPo.setMtu(configVlan.getMtu());
    }

    private List<VlanInfo> getVlanList() {
        return netWorkPortService.queryVlan(getDeviceId(), getUsername()).getData();
    }

    private void fillBondVlan(VlanInfo configVlan, VlanPo vlanPo) {
        BondPortDto bondPort = getBondPort().stream()
                .filter(port -> StringUtils.equals(port.getId(), configVlan.getPortId()))
                .findFirst()
                .orElseThrow(() -> {
                    log.error("Bond id: {} not exist.", configVlan.getPortId());
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Bond id not exist.");
                });
        vlanPo.setBondPortId(bondPort.getId());
        vlanPo.setPortNameList(Arrays.asList(bondPort.getBondInfo().split(COMMA)));
    }

    private static void fillVlanBaseInfo(VlanInfo configVlan, VlanPo vlanPo) {
        vlanPo.setPortType(configVlan.getPortType());
        vlanPo.setMtu(configVlan.getMtu());
        vlanPo.setId(configVlan.getId());
        vlanPo.setTags(Collections.singletonList(configVlan.getTag()));
    }

    private void fillEthVlan(VlanInfo configVlan, VlanPo vlanPo) {
        String condition = LogicType.SERVICE_PORT.getRole() + SEMICOLONS + LogicType.FRONT_END_CONTAINER_PORT.getRole();
        EthPortDto ethPort = getEthPort(condition)
                .stream()
                .filter(port -> StringUtils.equals(port.getId(), configVlan.getPortId()))
                .findFirst()
                .orElseThrow(() -> {
                    log.error("Eth id: {} not exist.", configVlan.getPortId());
                    return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Eth id not exist.");
                });
        vlanPo.setPortNameList(Collections.singletonList(ethPort.getLocation()));
    }

    private void fillIdAndName(LogicPortAddRequest logicPort, ServicePortPo result) {
        result.setId(logicPort.getId());
        result.setName(logicPort.getName());
    }

    private boolean isIpOfConfig(List<String> netConfigIps, LogicPortAddRequest logicPort) {
        return netConfigIps.contains(logicPort.getIpv4Addr()) || netConfigIps.contains(logicPort.getIpv6Addr());
    }

    private List<String> getAllNetIps() {
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        List<String> result = new ArrayList<>();
        result.addAll(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
        result.addAll(networkService.getNetPlaneIp(deviceNetworkInfo.getArchiveConfig()));
        result.addAll(networkService.getNetPlaneIp(deviceNetworkInfo.getReplicationConfig()));
        return result;
    }

    private List<String> getBackupIps() {
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        return new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getBackupConfig()));
    }

    private List<String> getReplicationIps() {
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        return new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getReplicationConfig()));
    }

    private List<String> getArchiveIps() {
        DeviceNetworkInfo deviceNetworkInfo = networkService.getDeviceNetworkInfo();
        return new ArrayList<>(networkService.getNetPlaneIp(deviceNetworkInfo.getArchiveConfig()));
    }


    private String getPortIdByName(String portName) {
        List<InitConfigInfo> initConfigInfos = initNetworkConfigMapper.queryInitConfigByEsnAndType(portName,
            getDeviceId());

        return initConfigInfos.stream()
            .map(initConfigInfo -> JSONObject.fromObject(initConfigInfo.getInitValue())
                .toBean(ServicePortPo.class)
                .getId())
            .findAny()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "portName not exist"));
    }

    private void dealDmResponse(DeviceManagerResponse<?> deviceManagerResponse) {
        if (!deviceManagerResponse.getError().isSuccess()) {
            log.error("request dm failed,cause: {}", deviceManagerResponse.getError().getDescription());
            throw new LegoCheckedException(deviceManagerResponse.getError().getCode(),
                deviceManagerResponse.getError().getErrorParam().split("//,"),
                deviceManagerResponse.getError().getDescription());
        }
    }

    private List<EthPortDto> getEthPort(String typeListString) {
        List<String> typeList = Arrays.stream(typeListString.split(SEMICOLONS)).collect(Collectors.toList());
        return netWorkPortService.queryEthPorts(getDeviceId(), getUsername())
            .getData()
            .stream()
            .filter(ethPort -> isSpecifiedLogicType(typeList, ethPort))
            .map(ethPort -> {
                EthPortDto ethPortDto = new EthPortDto();
                BeanUtils.copyProperties(ethPort, ethPortDto);
                return ethPortDto;
            }).collect(Collectors.toList());
    }

    private boolean isSpecifiedLogicType(List<String> typeList, EthPort ethPort) {
        return !VerifyUtil.isEmpty(ethPort.getLogicType()) && typeList.contains(ethPort.getLogicType().getRole());
    }

    /**
     * 校验绑定端口的参数是否有效
     *
     * @param bondPortPo 绑定端口
     * @return true:无效 false:有效
     */
    private boolean isBondPortInValid(BondPortPo bondPortPo) {
        if (VerifyUtil.isEmpty(bondPortPo.getPortNameList()) || bondPortPo.getPortNameList().size() < 2) {
            return true;
        }
        if (VerifyUtil.isEmpty(bondPortPo.getMtu())) {
            return true;
        }
        return Integer.parseInt(bondPortPo.getMtu()) < 1280 || Integer.parseInt(bondPortPo.getMtu()) > 9000;
    }

    /**
     * 检查vlan的字段是否有效
     *
     * @param vlanPo vlan
     * @return 是否有效
     */
    public boolean isVlanInfoInValid(VlanPo vlanPo) {
        if (VerifyUtil.isEmpty(vlanPo.getTags()) || VerifyUtil.isEmpty(vlanPo.getTags().get(0))) {
            return true;
        }
        if (VerifyUtil.isEmpty(vlanPo.getMtu())) {
            return true;
        }
        if (Integer.parseInt(vlanPo.getMtu()) < 1280 || Integer.parseInt(vlanPo.getMtu()) > 9000) {
            return true;
        }
        if (VerifyUtil.isEmpty(vlanPo.getPortNameList()) && VerifyUtil.isEmpty(vlanPo.getBondPortId())) {
            return true;
        }
        if (!InitNetworkConfigConstants.VLAN_PORT_TYPES.contains(vlanPo.getPortType())) {
            return true;
        }
        return vlanPo.getPortType().equalsVlanPortType(VlanPortType.BOND.getVlanPortType())
                && vlanPo.getPortNameList().size() < 2;
    }

    /**
     * 检查网络平面配置的网络和相同物理端口已经存在的网络是否属于网段
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param portIdList 物理端口id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    @Override
    public void checkNetworkSegment(NetPlaneInfoReq netPlaneInfoReq, List<String> portIdList,
                                    boolean isInternalNetworkExisted) {
        if (netPlaneInfoReq.isReuse()) {
            // 不是复用场景，则portIdList为以太网端口集合
            this.checkNetworkSegmentWhenReuse(netPlaneInfoReq, portIdList, isInternalNetworkExisted);
            return;
        }
        this.checkNetworkSegmentDependEth(netPlaneInfoReq, portIdList, isInternalNetworkExisted);
    }

    /**
     * 检查内部网络平面配置的网络和相同端口已经存在的网络是否属于同网段。不同ip类型，或者相同ip类型但是网段不同，都可能会导致网络问题。
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param ethPortIdList 内部网络的以太端口的id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    @Override
    public void checkNetworkSegmentDependEth(NetPlaneInfoReq netPlaneInfoReq, List<String> ethPortIdList,
                                             boolean isInternalNetworkExisted) {
        // 内部网络只能使用前端端口，所以这里只需要获取dm上的类型未前端端口的以太端口
        Map<String, EthPortDto> idEthPortMap = getEthPort(LogicType.SERVICE_PORT.getRole()
                + SEMICOLONS + LogicType.FRONT_END_CONTAINER_PORT.getRole()).stream()
                .collect(Collectors.toMap(EthPortDto::getId, ethPort -> ethPort));
        Map<String, BondPortDto> idBondPortDtoMap = getBondPort().stream()
                .collect(Collectors.toMap(BondPortDto::getId, bondPortDto -> bondPortDto));
        Map<String, VlanPo> idVlanMap = getVlan().stream().collect(Collectors.toMap(VlanPo::getId, vlanPo -> vlanPo));
        List<LogicPortAddRequest> logicPortDtoList = getLogicPort()
                .stream()
                .filter(logicPortDto -> checkLogicPortDependEthPorts(logicPortDto, ethPortIdList,
                        idBondPortDtoMap, idVlanMap, idEthPortMap))
                .collect(Collectors.toList());
        // 更新内部网络时，如果只有多集群内部通信网络的两个逻辑端口，则无需后续检查，防止这种情况下不能修改为其他网段ip
        if (isMultiClusterNetPlaneExisted(netPlaneInfoReq, isInternalNetworkExisted) && logicPortDtoList.size() == 2) {
            log.info("Update internal network and only 2 logic port exist.");
            return;
        }
        // 更新sftp网络时，且只有sftp一个逻辑端口，则无需后续同网段检查，防止不能修改为其他网段ip
        if (isSftpNetPlaneExisted(netPlaneInfoReq, isInternalNetworkExisted) && logicPortDtoList.size() == 1) {
            log.info("Update sftp net plane and only 1 logic port exist.");
            return;
        }
        checkNetworkSegment(netPlaneInfoReq, logicPortDtoList);
    }

    /**
     * 检查内部网络平面配置的网络和相同端口已经存在的网络是否属于同网段
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param reusePortIdList 复用端口id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    @Override
    public void checkNetworkSegmentWhenReuse(NetPlaneInfoReq netPlaneInfoReq, List<String> reusePortIdList,
                                             boolean isInternalNetworkExisted) {
        List<LogicPortAddRequest> logicPortDtoList = getLogicPort()
            .stream()
            .filter(logicPort -> reusePortIdList.contains(logicPort.getHomePortId()))
            .collect(Collectors.toList());
        checkNetworkSegment(netPlaneInfoReq, logicPortDtoList);
    }

    private boolean isMultiClusterNetPlaneExisted(NetPlaneInfoReq netPlaneInfoReq, boolean isInternalNetworkExisted) {
        return StringUtils.isNotEmpty(netPlaneInfoReq.getGaussIp())
            && StringUtils.isNotEmpty(netPlaneInfoReq.getInfraIp())
            && isInternalNetworkExisted;
    }

    private boolean isSftpNetPlaneExisted(NetPlaneInfoReq netPlaneInfoReq, boolean isInternalNetworkExisted) {
        return StringUtils.isNotEmpty(netPlaneInfoReq.getSftpIp()) && isInternalNetworkExisted;
    }

    private void checkNetworkSegment(NetPlaneInfoReq netPlaneInfoReq, List<LogicPortAddRequest> logicPortDtoList) {
        // 校验需要修改网络协议类型与已存在逻辑端口网络协议类型是否相同
        for (LogicPortAddRequest dmLogicPortAddRequest : logicPortDtoList) {
            // 根据IP网络协议类型进行对应的校验
            LogicPortDto logicPortDto = new LogicPortDto();
            if (AddressFamily.IPV4.getAddressFamily() == Integer.parseInt(netPlaneInfoReq.getIpType())) {
                logicPortDto.setMask(netPlaneInfoReq.getMask());
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getGaussIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getGaussIp());
                    checkIpv4NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getInfraIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getInfraIp());
                    checkIpv4NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getSftpIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getSftpIp());
                    checkIpv4NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
            } else {
                logicPortDto.setMask(netPlaneInfoReq.getMask());
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getGaussIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getGaussIp());
                    checkIpv6NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getInfraIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getInfraIp());
                    checkIpv6NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
                if (StringUtils.isNotEmpty(netPlaneInfoReq.getSftpIp())) {
                    logicPortDto.setIp(netPlaneInfoReq.getSftpIp());
                    checkIpv6NetworkSegment(logicPortDto, dmLogicPortAddRequest);
                }
            }
        }
    }

    // 检查逻辑端口依赖的以太端口是否在ethPortNameSet里
    private boolean checkLogicPortDependEthPorts(LogicPortAddRequest logicPort, List<String> ethPortIdList,
                                                 Map<String, BondPortDto> idBondPortMap,
                                                 Map<String, VlanPo> idVlanPoMap,
                                                 Map<String, EthPortDto> idEthPortMap) {
        switch (logicPort.getHomePortType()) {
            case ETHERNETPORT:
                // 以太端口的逻辑端口
                return ethPortIdList.contains(logicPort.getHomePortId());
            case BINDING:
                // 绑定端口的逻辑端口
                return checkLogicBondPortDependEthPorts(logicPort, idBondPortMap, ethPortIdList);
            case VLAN:
                // vlan端口的逻辑端口
                return checkLogicVlanPortDependEthPorts(logicPort, ethPortIdList, idVlanPoMap, idEthPortMap);
            default:
                log.error("Check logic port: {} depend eth port failed, home port type:{} error",
                        logicPort.getName(), logicPort.getHomePortType());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                        "Check logic port depend eth port failed, home port type error");
        }
    }

    // 检查依赖bond端口的逻辑端口，是否依赖指定的以太端口
    private boolean checkLogicBondPortDependEthPorts(LogicPortAddRequest logicPort,
                                                     Map<String, BondPortDto> idBondPortMap,
                                                     List<String> ethPortIdList) {
        String homePortId = logicPort.getHomePortId();
        BondPortDto bondPortDto = idBondPortMap.get(homePortId);
        return bondPortDto.getPortIdList().stream().anyMatch(ethPortIdList::contains);
    }

    // 检查依赖vlan端口的逻辑端口，是否依赖指定的以太端口
    private boolean checkLogicVlanPortDependEthPorts(LogicPortAddRequest logicPort, List<String> ethPortIdList,
                                                     Map<String, VlanPo> idVlanPoMap,
                                                     Map<String, EthPortDto> idEthPortMap) {
        String homePortId = logicPort.getHomePortId();
        VlanPo vlanPo = idVlanPoMap.get(homePortId);
        List<String> portNameList = vlanPo.getPortNameList();
        List<String> requestEthPortNameList = ethPortIdList.stream().map(id -> idEthPortMap.get(id).getLocation())
                .collect(Collectors.toList());
        return portNameList.stream().anyMatch(requestEthPortNameList::contains);
    }
}
