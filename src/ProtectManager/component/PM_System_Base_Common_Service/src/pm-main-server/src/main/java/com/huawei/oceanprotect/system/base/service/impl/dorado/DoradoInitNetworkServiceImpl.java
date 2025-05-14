/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.dorado;

import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.ARCHIVE_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.BACKUP_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.REPLICATION_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM;
import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.BINDING;
import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.ETHERNETPORT;
import static com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType.VLAN;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.constant.InitConfigErrorCode;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.ModifyLogicPortRouteRequest;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.api.StorageArraySessionResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.enums.DeviceTypeEnum;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.DeviceSystemService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.vo.InitNetWorkParam;

import com.google.common.collect.ImmutableList;

import feign.FeignException;
import jodd.util.StringUtil;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkIpRoutesInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.bean.NetWorkRouteInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.enums.IpType;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * dorado 特有的初始化的逻辑
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Slf4j
@Component
public class DoradoInitNetworkServiceImpl implements InitNetworkService {
    // 超级管理员角色id
    private static final Integer SUPER_ADMINISTRATOR_ROLE_TYPE = 1;

    // open storage api 访问设备的ip
    private static final String DEVICE_IP = "127.0.0.1";

    /**
     * 适用的部署类型
     */
    private static final ImmutableList<String> APPLICABLE_DEPLOY_TYPE_LIST = ImmutableList.of(
            DeployTypeEnum.X9000.getValue(), DeployTypeEnum.X8000.getValue(), DeployTypeEnum.X6000.getValue(),
            DeployTypeEnum.X3000.getValue(), DeployTypeEnum.OPEN_SOURCE.getValue());

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private InitializePortService initializePortService;

    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private NetWorkPortService netWorkPortService;

    @Autowired
    private DeviceSystemService deviceSystemService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    @Override
    public boolean applicable(String deployType) {
        return APPLICABLE_DEPLOY_TYPE_LIST.contains(deployType);
    }

    /**
     * 获取初始化的网络参数
     *
     * @param deviceId deviceId
     * @param username username
     * @param initNetworkBody 请求参数
     * @return InitNetWorkParam
     */
    @Override
    public InitNetWorkParam getInitNetWorkParam(InitNetworkBody initNetworkBody, String deviceId, String username) {
        List<String> backLogicPortNames = getLogicPortNames(getBackupLogicPorts(initNetworkBody));
        List<String> archiveLogicPortNames = getLogicPortNames(getArchiveLogicPorts(initNetworkBody));
        List<String> copyLogicPortNames = getLogicPortNames(getCopyLogicPorts(initNetworkBody));
        List<LogicPortDto> logicPortsCreatedByUser = initializePortService.getLogicPortsCreatedByUser();
        List<LogicPortDto> backLogicPorts = getUserCreatedBusinessPorts(backLogicPortNames, logicPortsCreatedByUser);
        List<LogicPortDto> archiveLogicPorts = getUserCreatedBusinessPorts(archiveLogicPortNames,
                logicPortsCreatedByUser);
        List<LogicPortDto> replicationLogicPorts =
            getUserCreatedBusinessPorts(copyLogicPortNames, logicPortsCreatedByUser);
        List<NetWorkConfigInfo> backupNetWorkConfigInfoList = new ArrayList<>();
        List<NetWorkConfigInfo> archiveNetWorkConfigInfoList = new ArrayList<>();
        List<NetWorkConfigInfo> replicationNetWorkConfigInfoList = new ArrayList<>();
        Map<String, String> map = queryControllerMapsToTheK8sNode();
        for (Map.Entry<String, String> next : map.entrySet()) {
            String controllerId = next.getKey();
            List<NetWorkLogicIp> backupIps = getIps(backLogicPorts, controllerId);
            List<NetWorkLogicIp> archiveIps = getIps(archiveLogicPorts, controllerId);
            List<NetWorkLogicIp> replicationIps = getIps(replicationLogicPorts, controllerId);
            List<NetWorkIpRoutesInfo> backupIpRoutes = getIpRoutes(backLogicPorts, controllerId);
            List<NetWorkIpRoutesInfo> archiveIpRoutes = getIpRoutes(archiveLogicPorts, controllerId);
            List<NetWorkIpRoutesInfo> replicationRoutes = getIpRoutes(replicationLogicPorts, controllerId);
            String nodeId = next.getValue();
            backupNetWorkConfigInfoList.add(new NetWorkConfigInfo(nodeId, backupIps, backupIpRoutes));
            archiveNetWorkConfigInfoList.add(new NetWorkConfigInfo(nodeId, archiveIps, archiveIpRoutes));
            replicationNetWorkConfigInfoList.add(new NetWorkConfigInfo(nodeId, replicationIps, replicationRoutes));
        }
        InitNetWorkParam initNetWorkParam = new InitNetWorkParam();
        initNetWorkParam.setBackupNetPlane(JSONObject.writeValueAsString(backupNetWorkConfigInfoList));
        initNetWorkParam.setArchiveNetPlane(JSONObject.writeValueAsString(archiveNetWorkConfigInfoList));
        initNetWorkParam.setReplicationNetPlane(JSONObject.writeValueAsString(replicationNetWorkConfigInfoList));
        return initNetWorkParam;
    }

    private List<LogicPortDto> getUserCreatedBusinessPorts(List<String> backLogicPortNames,
            List<LogicPortDto> logicPortsCreatedByUser) {
        return logicPortsCreatedByUser.stream().filter(port -> backLogicPortNames.contains(port.getName()))
                .collect(Collectors.toList());
    }

    private List<String> getLogicPortNames(List<LogicPortDto> logicPorts) {
        return logicPorts.stream().map(LogicPortDto::getName).collect(Collectors.toList());
    }

    @Override
    public void unifiedCheck(String deviceId, String username, InitNetworkBody initNetworkBody) {
        // 校验逻辑端口是否存在
        checkLogicPortIsExist(initNetworkBody);

        // 校验必须配置备份网络
        checkBackupNetworkIsExist(initNetworkBody);

        // 逻辑端口基本信息校验,逻辑端口名称重复
        checkLogicPortDuplicateName(initNetworkBody);

        // 备份、复制、归档业务配置后，每一控至少配置一个逻辑端口;
        // 备份逻辑端口角色必须为数据，复制逻辑端口角色必须为复制，归档逻辑端口角色必须为Databackup
        checkLogicPortNumsOfController(initNetworkBody);

        // 校验网络配置合法性,需要吗 TODO
        checkNetworkConfigPara(initNetworkBody);
    }

    private void checkLogicPortNumsOfController(InitNetworkBody initNetworkBody) {
        // 检查备份业务
        checkNetworkConfig(getBackupLogicPorts(initNetworkBody), PortRole.SERVICE);
        // 检查复制业务
        checkNetworkConfig(getCopyLogicPorts(initNetworkBody), PortRole.TRANSLATE);
        // 检查归档业务
        checkNetworkConfig(getArchiveLogicPorts(initNetworkBody), PortRole.ARCHIVE);
    }

    private void checkNetworkConfigPara(InitNetworkBody initNetworkBody) {
    }

    private void checkLogicPortDuplicateName(InitNetworkBody initNetworkBody) {
        List<LogicPortDto> allLogicPorts = new ArrayList<>();
        allLogicPorts.addAll(getBackupLogicPorts(initNetworkBody));
        allLogicPorts.addAll(getArchiveLogicPorts(initNetworkBody));
        allLogicPorts.addAll(getCopyLogicPorts(initNetworkBody));
        logicPortIsDuplicateName(allLogicPorts);
    }

    private void logicPortIsDuplicateName(List<LogicPortDto> initConfigLogicPorts) {
        List<String> checkNameDuplicateList = new ArrayList<>();
        for (LogicPortDto port : initConfigLogicPorts) {
            if (checkNameDuplicateList.contains(port.getName())) {
                log.error("Logic port: {} name duplicate.", port.getName());
                throw new LegoCheckedException(
                        InitConfigErrorCode.INITALIZATION_IP_NAME_OF_LOGIC_PORT_REPEATABLE_EXCEPTION,
                        "Logic port name duplicate.");
            }
            checkNameDuplicateList.add(port.getName());
        }
    }

    private void checkNetworkConfig(List<LogicPortDto> logicPorts, PortRole role) {
        if (VerifyUtil.isEmpty(logicPorts)) {
            return;
        }
        List<LogicPortDto> logicPortsCreatedByUser = initializePortService.getLogicPortsCreatedByUser();
        Map<String, String> map = queryControllerMapsToTheK8sNode();
        for (Map.Entry<String, String> next : map.entrySet()) {
            String controllerId = next.getKey();
            // 1.校验每一控是否配置了至少一个逻辑端口
            checkControllerNums(logicPorts, logicPortsCreatedByUser, controllerId, role);
            // 2.从现有的逻辑端口中查询配置的逻辑端口信息，根据查询出的逻辑端口信息检查是否存在指定角色的逻辑端口
            checkRole(role, logicPortsCreatedByUser, logicPorts);
        }
    }

    private void checkRole(PortRole role, List<LogicPortDto> logicPortsCreatedByUser, List<LogicPortDto> logicPorts) {
        List<String> portNames = logicPorts.stream().map(LogicPortDto::getName).collect(Collectors.toList());
        logicPortsCreatedByUser.stream().filter(port -> portNames.contains(port.getName())).forEach(configPort -> {
            if (!role.equals(configPort.getRole())) {
                log.error("Logical port: {} role misconfigured.", configPort.getName());
                throw new LegoCheckedException(InitConfigErrorCode.INITALIZATION_LOGIC_PORT_ROLE_ERROR_EXCEPTION,
                        new String[]{configPort.getName()}, "Logical port role misconfigured.");
            }
        });
    }

    private void checkControllerNums(List<LogicPortDto> logicPorts, List<LogicPortDto> logicPortsCreatedByUser,
            String controllerId, PortRole role) {
        List<String> portNames = logicPorts.stream().map(LogicPortDto::getName).collect(Collectors.toList());
        List<LogicPortDto> portsOfController = logicPortsCreatedByUser.stream()
                .filter(port -> portNames.contains(port.getName()))
                .filter(port -> controllerId.equals(port.getCurrentControllerId())).collect(Collectors.toList());
        int logicPortsNum = portsOfController.size();
        throwNetPlaneConfigError(controllerId, role, logicPortsNum);
        String portTypeLabel = null;
        int logicPortLimit;
        switch (role) {
            case SERVICE:
                portTypeLabel = BACKUP_SERVICE_PORT_LABEL;
                logicPortLimit = SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
                break;
            case TRANSLATE:
                portTypeLabel = REPLICATION_SERVICE_PORT_LABEL;
                logicPortLimit = SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM;
                break;
            case ARCHIVE:
                portTypeLabel = ARCHIVE_SERVICE_PORT_LABEL;
                logicPortLimit = SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
                break;
            default:
                log.error("The logic port role: {} is illegal.", role);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal port role.");
        }
        if (logicPortsNum > logicPortLimit) {
            log.error("The service: {} port num: {} exceed limit: {}.", role, logicPortsNum, logicPortLimit);
            throw new LegoCheckedException(CommonErrorCode.SERVICE_PORT_NUM_EXCEED_LIMIT,
                new String[] {portTypeLabel, String.valueOf(logicPortLimit)},
                ("logical port role" + role.getRole() + "is illegal."));
        }
    }

    private static void throwNetPlaneConfigError(String controllerId, PortRole role, int logicPortsNum) {
        if (logicPortsNum < 1) {
            log.error("Controller: {} config business failed.", controllerId);
            switch (role) {
                case SERVICE:
                    throw new LegoCheckedException(
                        InitConfigErrorCode.INITALIZATION_COPY_BACKUP_NETWORK_ERROR_EXCEPTION,
                        new String[] {controllerId}, "Controller config backup netplane failed.");
                case TRANSLATE:
                    throw new LegoCheckedException(
                        InitConfigErrorCode.INITALIZATION_COPY_REPLICATION_NETWORK_ERROR_EXCEPTION,
                        new String[] {controllerId}, "Controller config replication netplane failed.");
                case ARCHIVE:
                    throw new LegoCheckedException(
                        InitConfigErrorCode.INITALIZATION_COPY_ARCHIVE_NETWORK_ERROR_EXCEPTION,
                        new String[] {controllerId}, "Controller config archive netplane failed.");
                default:
                    log.error("The logic port role: {} is illegal.", role);
                    throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal port role.");
            }
        }
    }

    private void checkBackupNetworkIsExist(InitNetworkBody initNetworkBody) {
        BackupNetworkConfig backupNetworkConfig = initNetworkBody.getBackupNetworkConfig();
        if (VerifyUtil.isEmpty(backupNetworkConfig) || VerifyUtil.isEmpty(backupNetworkConfig.getLogicPorts())) {
            log.error("No backup network, init network config failed.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "No backup network, init network config failed.");
        }
    }

    private void checkLogicPortIsExist(InitNetworkBody initNetworkBody) {
        List<LogicPortDto> backLogicPorts = getBackupLogicPorts(initNetworkBody);
        List<LogicPortDto> archiveLogicPorts = getArchiveLogicPorts(initNetworkBody);
        List<LogicPortDto> copyLogicPorts = getCopyLogicPorts(initNetworkBody);
        List<LogicPortDto> logicPortsCreatedByUser = initializePortService.getLogicPortsCreatedByUser();
        List<String> logicPortNamesCreatedByUser = logicPortsCreatedByUser.stream().map(LogicPortDto::getName)
                .collect(Collectors.toList());
        String headLogNameBackup = "Backup";
        String headLogNameArchive = "Archive";
        String headLogNameCopy = "Copy";
        checkLogicPortIsExistByName(backLogicPorts, logicPortNamesCreatedByUser, headLogNameBackup);
        checkLogicPortIsExistByName(archiveLogicPorts, logicPortNamesCreatedByUser, headLogNameArchive);
        checkLogicPortIsExistByName(copyLogicPorts, logicPortNamesCreatedByUser, headLogNameCopy);
    }

    private static List<LogicPortDto> getCopyLogicPorts(InitNetworkBody initNetworkBody) {
        List<LogicPortDto> logicPorts = Optional.ofNullable(initNetworkBody.getCopyNetworkConfig())
                .orElse(new CopyNetworkConfig()).getLogicPorts();
        return Optional.ofNullable(logicPorts).orElse(new ArrayList<>());
    }

    private static List<LogicPortDto> getArchiveLogicPorts(InitNetworkBody initNetworkBody) {
        List<LogicPortDto> logicPorts = Optional.ofNullable(initNetworkBody.getArchiveNetworkConfig())
                .orElse(new ArchiveNetworkConfig()).getLogicPorts();
        return Optional.ofNullable(logicPorts).orElse(new ArrayList<>());
    }

    private static List<LogicPortDto> getBackupLogicPorts(InitNetworkBody initNetworkBody) {
        List<LogicPortDto> logicPorts = Optional.ofNullable(initNetworkBody.getBackupNetworkConfig())
                .orElse(new BackupNetworkConfig()).getLogicPorts();
        return Optional.ofNullable(logicPorts).orElse(new ArrayList<>());
    }

    private static void checkLogicPortIsExistByName(List<LogicPortDto> initConfigLogicPorts,
            List<String> logicPortNamesCreatedByUser, String headLogNameBackup) {
        initConfigLogicPorts.forEach(port -> {
            if (!logicPortNamesCreatedByUser.contains(port.getName())) {
                log.error(headLogNameBackup + "logic port: {} not exist", port.getName());
                throw new LegoCheckedException(InitConfigErrorCode.INITALIZATION_LOGIC_PORT_NOT_EXIST_EXCEPTION,
                        new String[]{port.getName()}, headLogNameBackup + "logic port not exist");
            }
        });
    }

    private List<NetWorkLogicIp> getIps(List<LogicPortDto> backLogicPorts, String controllerId) {
        return backLogicPorts.stream()
            .filter(port -> port.getCurrentControllerId().equals(controllerId))
            .map(port -> new NetWorkLogicIp(port.getIp(), port.getMask()))
            .collect(Collectors.toList());
    }

    private List<NetWorkIpRoutesInfo> getIpRoutes(List<LogicPortDto> logicPorts, String controllerId) {
        return logicPorts.stream()
            .filter(port -> port.getCurrentControllerId().equals(controllerId))
            .map(this::convertPortToIpRoutesInfo)
            .collect(Collectors.toList());
    }

    private NetWorkIpRoutesInfo convertPortToIpRoutesInfo(LogicPortDto portDto) {
        List<PortRouteInfo> portRouteInfos = netWorkPortService.getRoute(initializePortService.getDeviceId(),
            initializePortService.getUsername(), portDto.getId()).getData();
        List<NetWorkRouteInfo> netWorkRouteInfos = portRouteInfos.stream()
            .map(portRouteInfo -> new NetWorkRouteInfo(portRouteInfo.getRouteType().getRouteType(),
                portRouteInfo.getDestination(), portRouteInfo.getMask(), portRouteInfo.getGateway()))
            .collect(Collectors.toList());
        Optional<NetWorkRouteInfo> defaultRoute = netWorkRouteInfos.stream()
            .filter(route -> StringUtil.equals(RouteType.DEFAULT.getRouteType(), route.getType()))
            .findAny();
        if (!defaultRoute.isPresent() && !StringUtil.isEmpty(portDto.getGateWay())) {
            if (IpType.IPV4.getValue().equals(portDto.getIpType())) {
                netWorkRouteInfos.add(new NetWorkRouteInfo(RouteType.DEFAULT.getRouteType(),
                    "0.0.0.0", "0.0.0.0", portDto.getGateWay()));
            } else {
                netWorkRouteInfos.add(new NetWorkRouteInfo(RouteType.DEFAULT.getRouteType(), "::", "0",
                    portDto.getGateWay()));
            }
        }
        return new NetWorkIpRoutesInfo(portDto.getIp(), netWorkRouteInfos);
    }

    private Map<String, String> queryControllerMapsToTheK8sNode() {
        InfraResponseWithError<List<NodeDetail>> infraNodeInfo = Optional
                .ofNullable(infrastructureRestApi.getInfraNodeInfo())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.STATUS_ERROR,
                        "Invoke infrastructure interface to query relation between controller and node failed"));
        if (infraNodeInfo.getError() != null) {
            log.error("get node response from infra fail, errId: {}, errMsg: {}", infraNodeInfo.getError().getErrId(),
                    infraNodeInfo.getError().getErrMsg());
            throw new LegoCheckedException(CommonErrorCode.STATUS_ERROR,
                    "Invoke infrastructure interface to query " + "relation between controller and node failed");
        }
        if (infraNodeInfo.getData() == null) {
            log.error("get node response data from infra is null");
            throw new LegoCheckedException(CommonErrorCode.STATUS_ERROR,
                    "Invoke infrastructure interface to query " + "relation between controller and node failed");
        }
        List<NodeDetail> nodeInfos = infraNodeInfo.getData();
        return nodeInfos.stream().collect(Collectors.toMap(NodeDetail::getNodeName, NodeDetail::getHostName));
    }

    /**
     * 获取设备ip用于openstorageapi访问设备
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceIp() {
        return DEVICE_IP;
    }

    /**
     * 获取设备类型
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceType() {
        return DeviceTypeEnum.OCEAN_PROTECT_X.getType();
    }

    /**
     * 新建机机账号时，获取用户角色id
     *
     * @return 用户角色id
     */
    @Override
    public Integer getStorageUserRoleId() {
        return SUPER_ADMINISTRATOR_ROLE_TYPE;
    }

    /**
     * 获取超级管理员角色id
     *
     * @return 超级管理员角色id
     */
    @Override
    public Integer getSuperAdminRoleId() {
        return SUPER_ADMINISTRATOR_ROLE_TYPE;
    }

    /**
     * 获取设备session时检查返回结果
     *
     * @param arraySessionResponse arraySessionResponse
     * @param username username
     * @param isAllowUnInitUser isAllowUnInitUser
     */
    @Override
    public void checkLoginResponse(StorageArraySessionResponse arraySessionResponse, String username,
            boolean isAllowUnInitUser) {
        return;
    }

    /**
     * 创建存储用户
     *
     * @param authUsername 已经认证过的用户名
     * @param addUsername 待创建的用户名
     * @param addUserRoleId 角色id
     * @return 获取创建之后密码
     */
    @Override
    public String createStorageUser(String authUsername, String addUsername, Integer addUserRoleId) {
        return "";
    }

    /**
     * 初始化存储用户
     *
     * @param addUsername 待创建的用户名
     */
    @Override
    public void initStorageUser(String addUsername) {
    }

    /**
     * 修改密码
     *
     * @param addUsername 待修改的用户名
     * @param password 当前的password
     * @return 修改之后的密码
     */
    @Override
    public String changePass(String addUsername, String password) {
        return password;
    }

    /**
     * 创建逻辑端口
     *
     * @param initNetworkBody 请求参数
     */
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

        List<LogicPortDto> alreadyCreateLogicPortDtos = new ArrayList<>();
        try {
            logicPortDtos.forEach(port -> {
                LogicPortDto logicPortDto = new LogicPortDto();
                BeanUtils.copyProperties(port, logicPortDto);
                // 复用已有端口信息
                reuseExistPortInfo(logicPortDto);
                // 创建逻辑端口
                initializePortService.handleLogicPort(logicPortDto);
                // 该端口添加路由
                addPortRoute(logicPortDto);
                alreadyCreateLogicPortDtos.add(logicPortDto);
            });
        } catch (LegoCheckedException e) {
            log.error("Add logic port error. ", ExceptionUtil.getErrorMessage(e));
            if (!VerifyUtil.isEmpty(alreadyCreateLogicPortDtos)) {
                List<String> alreadyCreateNames = getAlreadyCreateLogicPortDtosName(alreadyCreateLogicPortDtos);
                log.error("LLD initialize failed, delete already create logic ports name : {}.",
                    JSONObject.writeValueAsString(alreadyCreateNames));
                // LLD初始化时遇到报错，应把已创建好的端口都删掉
                alreadyCreateNames.forEach(name -> initializePortService.deleteLogicPort(name));
            }
            throw e;
        }
    }

    private List<String> getAlreadyCreateLogicPortDtosName(List<LogicPortDto> alreadyCreateLogicPortDtos) {
        return alreadyCreateLogicPortDtos.stream().map(LogicPortDto::getName).collect(Collectors.toList());
    }

    /**
     * 添加端口路由
     *
     * @param logicPortDto 逻辑端口
     */
    @Override
    public void addPortRoute(LogicPortDto logicPortDto) {
        if (CollectionUtils.isEmpty(logicPortDto.getRoute())) {
            log.info("No route for this logic port: {}", logicPortDto.getName());
            return;
        }
        for (PortRouteInfo route : logicPortDto.getRoute()) {
            if (RouteType.DEFAULT.equals(route.getRouteType()) && !StringUtil.isEmpty(logicPortDto.getGateWay())) {
                log.info("The logic port configures gateway, no need to configure default route.");
                continue;
            }
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
            ServicePortPo existLogicPort = Optional.ofNullable(JSONObject.fromObject(
                initNetworkConfigMapper.queryInitConfig(object.toString()).get(0).getInitValue())
                    .toBean(ServicePortPo.class))
                .orElse(new ServicePortPo());
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

    private boolean reuseBondPortInfo(LogicPortDto logicPortDto, ServicePortPo existLogicPort) {
        String homePortType = existLogicPort.getHomePortType().getHomePortType();
        if (ETHERNETPORT.equalsHomePortType(homePortType)) {
            log.warn("No available bond port in PM database.");
            return false;
        }
        List<String> newPortNameList = logicPortDto.getBondPort().getPortNameList();
        BondPortPo existBondPort = existLogicPort.getBondPort();
        List<String> existPortNameList = existBondPort.getPortNameList();

        // 校验绑定端口名称和端口列表是否都相同，不相同会报错
        initializePortService.checkIsValidPortNameAndPortNameList(logicPortDto, existLogicPort);

        // 新创的绑定口和已有绑定口名称列表相同，并且绑定端口名称也相同，复用
        if (isSamePortList(existPortNameList, newPortNameList) && isSameBondPortName(existBondPort,
            logicPortDto.getBondPort())) {
            String bondPortId = existBondPort.getId();
            logicPortDto.setHomePortId(bondPortId);
            logicPortDto.getBondPort().setId(bondPortId);
            return true;
        }
        return false;
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

    private boolean reuseVlanEthPortInfo(LogicPortDto newLogicPortDto, ServicePortPo existLogicPort) {
        // 只有vlan的以太端口可以复用
        HomePortType existHomePortType = existLogicPort.getHomePortType();
        if (VLAN != existHomePortType) {
            return false;
        }
        VlanPo newVlan = newLogicPortDto.getVlan();
        VlanPo existVlan = existLogicPort.getVlan();
        if (existVlan.getPortType() == VlanPortType.BOND) {
            return false;
        }
        List<String> newPortNameList = newVlan.getPortNameList();
        List<String> existVlanPortNameList = existVlan.getPortNameList();
        String newVlanTag = newVlan.getTags().get(0);
        String existVlanTag = existVlan.getTags().get(0);
        // vlan的tag和名称列表都一样
        if (isSamePortList(newPortNameList, existVlanPortNameList) && newVlanTag.equals(existVlanTag)) {
            String id = existVlan.getId();
            newLogicPortDto.setHomePortId(id);
            newLogicPortDto.getVlan().setId(id);
            return true;
        }
        return false;
    }

    // 只考虑新建的是vlan绑定的情况
    private boolean reuseVlanBondPortInfo(LogicPortDto newLogicPortDto, ServicePortPo existLogicPort) {
        // 新创的一定是vlan绑定
        if (newLogicPortDto.getHomePortType() != VLAN || newLogicPortDto.getVlan().getPortType() != VlanPortType.BOND) {
            log.warn("The new add logic port is not vlan bond.");
            return false;
        }
        HomePortType existHomePortType = existLogicPort.getHomePortType();
        if (existHomePortType == ETHERNETPORT || (existHomePortType == VLAN
            && existLogicPort.getVlan().getPortType() == VlanPortType.ETH)) {
            return false;
        }

        BondPortPo existBondPort = existLogicPort.getBondPort();
        List<String> existPortNameList = existBondPort.getPortNameList();
        BondPortPo newBondPort = newLogicPortDto.getBondPort();
        List<String> newPortNameList = newBondPort.getPortNameList();
        // 先考虑只复用绑定端口的情况
        if (existHomePortType == BINDING) {
            initializePortService.checkIsValidPortNameAndPortNameList(newLogicPortDto, existLogicPort);
            if (isSamePortList(existPortNameList, newPortNameList) && isSameBondPortName(existBondPort, newBondPort)) {
                String bondPortId = existBondPort.getId();
                newBondPort.setId(bondPortId);
                return true;
            }
        }
        return handleExistVlanBond(newLogicPortDto, existLogicPort, newPortNameList);
    }

    // 只处理已有的逻辑端口是vlan绑定的情况，包含2.1和2.2
    private boolean handleExistVlanBond(LogicPortDto newLogicPortDto, ServicePortPo existLogicPort,
        List<String> newPortNameList) {
        if (existLogicPort.getHomePortType() != VLAN || existLogicPort.getVlan().getPortType() != VlanPortType.BOND) {
            log.warn("The exist logic port is not vlan bond.");
            return false;
        }
        VlanPo newVlan = newLogicPortDto.getVlan();
        VlanPo existVlan = existLogicPort.getVlan();
        List<String> existVlanPortName = existVlan.getPortNameList();
        initializePortService.checkIsValidPortNameAndPortNameList(newLogicPortDto, existLogicPort);
        if (isSamePortList(existVlanPortName, newPortNameList)) {
            String newVlanTag = newVlan.getTags().get(0);
            String existVlanTag = existVlan.getTags().get(0);

            // tag一样，复用整个vlan绑定
            if (StringUtils.equals(existVlanTag, newVlanTag)) {
                String id = existVlan.getId();
                newLogicPortDto.setHomePortId(id);
                newLogicPortDto.getVlan().setId(id);
                return true;
            }

            // tag不相同，只需创建vlan口
            String bondPortId = existVlan.getBondPortId();
            newLogicPortDto.getVlan().setBondPortId(bondPortId);
            return true;
        }
        return false;
    }

    private boolean reuseVlanPortInfo(LogicPortDto logicPortDto, ServicePortPo existLogicPort) {
        if (ETHERNETPORT.equalsHomePortType(existLogicPort.getHomePortType().getHomePortType())) {
            log.warn("No available vlan port in PM database.");
            return false;
        }

        boolean isReuse = false;
        VlanPortType newVlanPortType = logicPortDto.getVlan().getPortType();
        switch (newVlanPortType) {
            case ETH:
                isReuse = reuseVlanEthPortInfo(logicPortDto, existLogicPort);
                break;
            case BOND:
                isReuse = reuseVlanBondPortInfo(logicPortDto, existLogicPort);
                break;
        }
        return isReuse;
    }

    /**
     * 初始化操作日志存储策略
     *
     */
    @Override
    public void initOperationLogStrategy() {
        try {
            String esn = clusterBasicService.getCurrentClusterEsn();
            deviceSystemService.initOperationLogStrategy(esn);
        } catch (FeignException e) {
            log.error("Fail to init operation log strategy, error is ", ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
    }
}
