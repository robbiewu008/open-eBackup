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
package com.huawei.oceanprotect.system.base.service.impl.dependent;

import com.huawei.oceanprotect.base.cluster.sdk.service.DependentClusterService;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.api.StorageArraySessionResponse;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.vo.InitNetWorkParam;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetworkPortInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.entity.IpPoolDto;
import openbackup.system.base.sdk.devicemanager.request.IpInfo;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;
import openbackup.system.base.util.DependentDeployTypeIpUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Service for initialization in case of {@link DeployTypeEnum#E1000}
 *
 * @since 2024-02-10
 */
@Service
@Slf4j
public class DependentInitNetworkServiceImpl implements InitNetworkService {
    // 超级管理员角色id
    private static final int SUPER_ADMINISTRATOR_ROLE_TYPE = 1;

    // ip regex pattern
    private static final Pattern IP_PATTERN = Pattern.compile("[0-9]*\\.[0-9]*\\.[0-9]*\\.[0-9]*/[0-9]*");

    /**
     * 适用的部署类型
     */
    private static final ImmutableList<String> APPLICABLE_DEPLOY_TYPE_LIST = ImmutableList.of(
        DeployTypeEnum.E1000.getValue(), DeployTypeEnum.OPEN_SERVER.getValue());

    @Autowired
    private InfrastructureService infrastructureService;

    @Autowired
    private DependentClusterService dependentClusterService;

    /**
     * detect object applicable
     *
     * @param deployType 部署类型
     * @return detect result
     */
    @Override
    public boolean applicable(String deployType) {
        return APPLICABLE_DEPLOY_TYPE_LIST.contains(deployType);
    }

    /**
     * 获取初始化的网络参数
     *
     * @param initNetworkBody 请求参数
     * @param deviceId deviceId
     * @param username username
     * @return InitNetWorkParam
     */
    @Override
    public InitNetWorkParam getInitNetWorkParam(InitNetworkBody initNetworkBody, String deviceId, String username) {
        log.info("Get init network param start.");
        List<NodeNetworkInfoRequest> backup = initNetworkBody.getBackupNetworkConfig().getPacificInitNetWorkInfoList();
        List<NodeNetworkInfoRequest> archive = Optional.ofNullable(initNetworkBody.getArchiveNetworkConfig())
            .orElse(new ArchiveNetworkConfig())
            .getPacificInitNetWorkInfoList();
        if (CollectionUtils.isEmpty(archive)) {
            archive = backup;
        }
        List<NodeNetworkInfoRequest> replication = Optional.ofNullable(initNetworkBody.getCopyNetworkConfig())
            .orElse(new CopyNetworkConfig())
            .getPacificInitNetWorkInfoList();
        if (CollectionUtils.isEmpty(replication)) {
            replication = backup;
        }
        List<NodeDetail> nodeDetails = infrastructureService.queryHostNodeInfo();
        Map<String, String> ipNodeMap = nodeDetails.stream()
            .collect(Collectors.toMap(NodeDetail::getManagementAddress, NodeDetail::getHostName));

        InitNetWorkParam initNetWorkParam = new InitNetWorkParam();

        List<NetWorkConfigInfo> backupParamList = backup.stream()
            .map(info -> DependentDeployTypeIpUtil.generateConfigMapParam(ipNodeMap.get(info.getManageIp()),
                info.getIpInfoList().stream().map(IpInfo::getIpAddress).collect(Collectors.toList())))
            .collect(Collectors.toList());
        initNetWorkParam.setBackupNetPlane(JSONObject.writeValueAsString(backupParamList));

        List<NetWorkConfigInfo> archiveParamList = archive.stream()
            .map(info -> DependentDeployTypeIpUtil.generateConfigMapParam(ipNodeMap.get(info.getManageIp()),
                info.getIpInfoList().stream().map(IpInfo::getIpAddress).collect(Collectors.toList())))
            .collect(Collectors.toList());
        initNetWorkParam.setArchiveNetPlane(JSONObject.writeValueAsString(archiveParamList));

        List<NetWorkConfigInfo> replicationParamList = replication.stream()
            .map(info -> DependentDeployTypeIpUtil.generateConfigMapParam(ipNodeMap.get(info.getManageIp()),
                info.getIpInfoList().stream().map(IpInfo::getIpAddress).collect(Collectors.toList())))
            .collect(Collectors.toList());
        initNetWorkParam.setReplicationNetPlane(JSONObject.writeValueAsString(replicationParamList));
        log.info("Get init network param success.");

        return initNetWorkParam;
    }

    /**
     * 统一参数校验
     *
     * @param deviceId deviceId
     * @param username username
     * @param initNetworkBody 初始化参数
     */
    @Override
    public void unifiedCheck(String deviceId, String username, InitNetworkBody initNetworkBody) {
        List<NodeDetail> nodeDetails = infrastructureService.queryHostNodeInfo();
        Map<String, List<NetworkPortInfo>> nodeIpMap = nodeDetails.stream()
            .collect(Collectors.toMap(NodeDetail::getManagementAddress,
                nodeDetail -> DependentDeployTypeIpUtil.convertIpStringToNetworkPortPo(
                    dependentClusterService.getNodeIpInfo(nodeDetail.getHostName()))));
        // check whether the ip is still up
        checkNetworkParam(nodeIpMap, initNetworkBody.getBackupNetworkConfig().getPacificInitNetWorkInfoList());
        if (!VerifyUtil.isEmpty(initNetworkBody.getCopyNetworkConfig().getPacificInitNetWorkInfoList())) {
            checkNetworkParam(nodeIpMap, initNetworkBody.getCopyNetworkConfig().getPacificInitNetWorkInfoList());
        }
        if (!VerifyUtil.isEmpty(initNetworkBody.getArchiveNetworkConfig().getPacificInitNetWorkInfoList())) {
            checkNetworkParam(nodeIpMap, initNetworkBody.getArchiveNetworkConfig().getPacificInitNetWorkInfoList());
        }
    }

    private void checkNetworkParam(Map<String, List<NetworkPortInfo>> nodeIpMap,
        List<NodeNetworkInfoRequest> infoList) {
        for (NodeNetworkInfoRequest ipInfo : infoList) {
            String manageIp = ipInfo.getManageIp();
            if (StringUtils.isBlank(manageIp)) {
                log.warn("NodeNetworkInfoRequest has an empty or blank manageIp.");
                continue;
            }
            if (!nodeIpMap.containsKey(manageIp)) {
                log.error("Manage IP '{}' is not found in nodeIpMap.", manageIp);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Manage IP not found: " + manageIp);
            }
            List<NetworkPortInfo> networkPortInfoList = nodeIpMap.get(manageIp);
            List<IpInfo> ipInfoList = ipInfo.getIpInfoList();
            if (CollectionUtils.isEmpty(ipInfoList)) {
                log.warn("NodeNetworkInfoRequest with manageIp '{}' has an empty ipInfoList.", manageIp);
                continue;
            }
            checkNodeIpValidity(ipInfoList, manageIp, networkPortInfoList);
        }
    }

    private void checkNodeIpValidity(List<IpInfo> ipInfoList, String manageIp,
        List<NetworkPortInfo> networkPortInfoList) {
        for (IpInfo info : ipInfoList) {
            if (info == null) {
                log.warn("Encountered a null IpInfo in the list for manageIp '{}'.", manageIp);
                continue;
            }
            String ifaceName = info.getIfaceName();
            if (StringUtils.isBlank(ifaceName)) {
                log.warn("IpInfo has an empty or blank ifaceName for manageIp '{}'.", manageIp);
                continue;
            }
            NetworkPortInfo portInfo = networkPortInfoList.stream()
                .filter(networkPortInfo -> networkPortInfo.getIfaceName().equals(ifaceName))
                .findFirst()
                .orElse(new NetworkPortInfo());
            if (!ifaceName.equals(portInfo.getIfaceName())) {
                log.error("Interface name '{}' is not found in local network info for manageIp '{}'.", ifaceName,
                    manageIp);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Interface not found: " + ifaceName + " for manageIp: " + manageIp);
            }

            List<String> curIpList = portInfo.getIpList();
            if (CollectionUtils.isEmpty(curIpList)) {
                log.warn("Interface '{}' for manageIp '{}' has an empty IP list.", ifaceName, manageIp);
                continue;
            }

            String ipAddress = info.getIpAddress();
            if (StringUtils.isBlank(ipAddress)) {
                log.warn("IpInfo has an empty or blank ipAddress for manageIp '{}' and iface '{}'.", manageIp,
                    ifaceName);
                continue;
            }

            if (!curIpList.contains(ipAddress)) {
                log.error("IP '{}' is not found under interface '{}' for manageIp '{}'.", ipAddress, ifaceName,
                    manageIp);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "IP not found: " + ipAddress + " under interface: " + ifaceName + " for manageIp: " + manageIp);
            }
        }
    }

    /**
     * 获取业务网络配置信息NetworkInfo
     *
     * @param manageIp management IP
     * @return network info
     */
    public NetworkInfoDto getNetworkInfoForDecoupled(String manageIp) {
        List<NodeDetail> nodeDetails = infrastructureService.queryHostNodeInfo();
        Map<String, String> hostNameManagerIpMap = nodeDetails.stream()
            .collect(Collectors.toMap(NodeDetail::getHostName, NodeDetail::getManagementAddress));
        List<String> managerIpList = new ArrayList<>(hostNameManagerIpMap.values());
        if (!StringUtils.isEmpty(manageIp) && !managerIpList.contains(manageIp)) {
            // 模糊查询不匹配
            return new NetworkInfoDto(new ArrayList<>());
        }
        List<NodeNetworkInfoDto> nodesNetWorkInfo = nodeDetails.stream().map(nodeDetail -> {
            List<String> ipList = dependentClusterService.getNodeIpInfo(nodeDetail.getHostName());
            return new NodeNetworkInfoDto(hostNameManagerIpMap.get(nodeDetail.getHostName()),
                buildNodeIp(ipList, nodeDetail.getManagementAddress(), nodeDetails));
        }).collect(Collectors.toList());
        return new NetworkInfoDto(nodesNetWorkInfo);
    }

    private List<IpPoolDto> buildNodeIp(List<String> ipList, String managementIp, List<NodeDetail> nodeDetails) {
        Set<String> doNotShow = getNotShow(managementIp);
        List<IpPoolDto> result = new ArrayList<>();
        for (String ipInfo : ipList) {
            List<String> splitResult = Arrays.stream(ipInfo.split(" "))
                .filter(v -> !" ".equals(v) && !v.isEmpty())
                .collect(Collectors.toList());
            if (splitResult.size() <= IsmNumberConstant.TWO) {
                continue;
            }
            String adapterName = splitResult.get(0);
            for (int i = IsmNumberConstant.TWO; i < splitResult.size(); i++) {
                String ipAddress = splitResult.get(i);
                String[] splitIp = ipAddress.split("/");
                log.debug("The " + i + " adapterName is : {}, ipAddress is : {}", adapterName, ipAddress);
                Matcher matcher = IP_PATTERN.matcher(ipAddress);
                // 过滤掉管理ip和k8sip ,k8s节点集群浮动ip, k8s高可用ip, GaussDB高可用ip
                if (!matcher.find() || doNotShow.contains(splitIp[0])) {
                    log.info("The ip {} should not be displayed.", splitIp[0]);
                    continue;
                }
                IpPoolDto ipPoolDto = new IpPoolDto();
                ipPoolDto.setIfaceName(adapterName);
                ipPoolDto.setIpAddress(ipAddress);
                result.add(ipPoolDto);
            }
        }
        return result;
    }

    private Set<String> getNotShow(String managementIp) {
        List<NodeDetail> details = infrastructureService.queryHostNodeInfo();
        Set<String> doNotShow = new HashSet<>();
        String k8sIp = Strings.EMPTY;
        String controllerPlaneEndpoint = Strings.EMPTY;
        String haEndpoint = Strings.EMPTY;
        String k8sFloatIp = Strings.EMPTY;
        for (NodeDetail nodeDetail : details) {
            if (nodeDetail.getManagementAddress().equals(managementIp)) {
                k8sIp = nodeDetail.getAddress();
                controllerPlaneEndpoint = nodeDetail.getControllerPlaneEndpoint();
                haEndpoint = nodeDetail.getHaEndpoint();
                k8sFloatIp = nodeDetail.getServicePlaneEndpoint();
            }
        }
        doNotShow.add(k8sIp);
        doNotShow.add(controllerPlaneEndpoint);
        doNotShow.add(haEndpoint);
        doNotShow.add(k8sFloatIp);
        doNotShow.add(managementIp);
        log.info("infra do not show contains : {}", JSONObject.writeValueAsString(doNotShow));
        return doNotShow;
    }

    /**
     * 获取设备ip用于openstorageapi访问设备
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceIp() {
        return "";
    }

    /**
     * 获取设备类型
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceType() {
        return "";
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
     * @param response arraySessionResponse
     * @param username username
     * @param isAllowUnInitUser isAllowUnInitUser
     */
    @Override
    public void checkLoginResponse(StorageArraySessionResponse response, String username, boolean isAllowUnInitUser) {
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
        return "";
    }

    /**
     * 添加逻辑端口
     *
     * @param initNetworkBody 请求参数
     */
    @Override
    public void addLogicPort(InitNetworkBody initNetworkBody) {
    }

    /**
     * 添加端口路由
     *
     * @param logicPortDto 逻辑端口
     */
    @Override
    public void addPortRoute(LogicPortDto logicPortDto) {
    }

    /**
     * 初始化操作日志存储策略
     *
     */
    @Override
    public void initOperationLogStrategy() {
    }
}
