/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.pacific;

import com.huawei.oceanprotect.base.cluster.sdk.service.PacificClusterService;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeUserServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.api.StorageArraySessionResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.enums.DeviceTypeEnum;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.DeviceUserService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.vo.InitNetWorkParam;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.InitConstants;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.RandomPwdUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.entity.NodeInfoDto;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * pacific 特有的初始化的逻辑
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Service
@Slf4j
public class PacificInitNetworkServiceImpl implements InitNetworkService {
    private static final Integer MACHINE_MACHINE_ACCOUNT_ID = 4;

    // 超级管理员角色id
    private static final Integer SUPER_ADMINISTRATOR_ROLE_TYPE = 1;

    // 密码处于初始化状态，需要修改
    private static final int PASSWORD_INIT_NEED_CHANGE = 4;

    // dm上超级管理员的最大数量
    private static final int SUPER_USER_SIZE_MAX = 2;

    /**
     * 适用的部署类型
     */
    private static final ImmutableList<String> APPLICABLE_DEPLOY_TYPE_LIST =
        ImmutableList.of(DeployTypeEnum.E6000.getValue());

    @Autowired
    private InitConfigService initConfigService;

    @Autowired
    private InitializeUserServiceAbility initializeUserServiceAbility;

    @Autowired
    private DeviceUserService deviceUserService;

    @Autowired
    private PacificClusterService pacificClusterService;

    @Autowired
    private OpenStorageService openStorageService;

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
        List<NodeNetworkInfoRequest> backupInitNetworkInfoList = initNetworkBody.getBackupNetworkConfig()
            .getPacificInitNetWorkInfoList();

        Map<String, NodeInfoDto> manageIpAndNodeMap = openStorageService.getNetworkInfo(deviceId, username);
        // 备份
        List<NetWorkConfigInfo> backupNetWorkConfigInfoList = new ArrayList<>();
        pacificClusterService.buildNetworkConfigInfo(backupInitNetworkInfoList,
            backupNetWorkConfigInfoList, manageIpAndNodeMap);
        // 归档网络可以不配，所有节点都配，或者都不配
        ArchiveNetworkConfig archiveNetworkConfig = Optional.ofNullable(initNetworkBody.getArchiveNetworkConfig())
            .orElseGet(ArchiveNetworkConfig::new);
        List<NodeNetworkInfoRequest> archiveInitNetworkInfoList = archiveNetworkConfig.getPacificInitNetWorkInfoList();
        List<NetWorkConfigInfo> archiveNetWorkConfigInfoList = new ArrayList<>();
        if (!VerifyUtil.isEmpty(archiveInitNetworkInfoList)) {
            pacificClusterService.buildNetworkConfigInfo(archiveInitNetworkInfoList,
                archiveNetWorkConfigInfoList, manageIpAndNodeMap);
        }
        // 复制网络可以不配，所有节点都配，或者都不配
        CopyNetworkConfig copyNetworkConfig = Optional.ofNullable(initNetworkBody.getCopyNetworkConfig())
            .orElseGet(CopyNetworkConfig::new);
        List<NodeNetworkInfoRequest> copyInitNetworkInfoList = copyNetworkConfig.getPacificInitNetWorkInfoList();
        List<NetWorkConfigInfo> copyNetWorkConfigInfoList = new ArrayList<>();
        if (!VerifyUtil.isEmpty(copyInitNetworkInfoList)) {
            pacificClusterService.buildNetworkConfigInfo(copyInitNetworkInfoList,
                copyNetWorkConfigInfoList, manageIpAndNodeMap);
        }

        InitNetWorkParam initNetWorkParam = new InitNetWorkParam();
        initNetWorkParam.setBackupNetPlane(JSONObject.writeValueAsString(backupNetWorkConfigInfoList));
        initNetWorkParam.setArchiveNetPlane(JSONObject.writeValueAsString(archiveNetWorkConfigInfoList));
        initNetWorkParam.setReplicationNetPlane(JSONObject.writeValueAsString(copyNetWorkConfigInfoList));

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
        unifiedCheck(deviceId, username, initNetworkBody, false);
    }

    /**
     * 统一参数校验
     *
     * @param deviceId deviceId
     * @param username username
     * @param initNetworkBody 初始化参数
     * @param isLld 是否是解析lld时检查参数
     */
    @Override
    public void unifiedCheck(String deviceId, String username, InitNetworkBody initNetworkBody, boolean isLld) {
        log.info("Unified check start, deviceId: {}, username: {}, isLld: {}.", deviceId, username, isLld);
        if (!isLld) {
            checkStorageUserStatus(deviceId, username);
        }

        // 所有节点的管理ip组成的list
        Map<String, NodeInfoDto> manageIpAndNodeMap = openStorageService.getNetworkInfo(deviceId, username);

        checkBackupBusinessIp(deviceId, initNetworkBody, isLld, manageIpAndNodeMap);
        checkArchiveBusinessIp(deviceId, initNetworkBody, isLld, manageIpAndNodeMap);
        checkCopyBusinessIp(deviceId, initNetworkBody, isLld, manageIpAndNodeMap);
    }

    private void checkBackupBusinessIp(String deviceId, InitNetworkBody initNetworkBody, boolean isLld,
        Map<String, NodeInfoDto> manageIpAndNodeMap) {
        // 检查管理ip：不能少，不能重复，必须包含所有节点，不能包含dm上不存在的管理ip
        List<NodeNetworkInfoRequest> backupInitNetworkInfoList = initNetworkBody.getBackupNetworkConfig()
            .getPacificInitNetWorkInfoList();
        List<String> allNodeManageIpList = new ArrayList<>(manageIpAndNodeMap.keySet());
        pacificClusterService.checkManageIp(allNodeManageIpList, backupInitNetworkInfoList, isLld);
        // 检查业务网络配置：每个节点至少一个， 在dm上能找到，
        pacificClusterService.checkBusinessIp(backupInitNetworkInfoList, manageIpAndNodeMap, isLld);
        log.info("Unified check backup network info success, deviceId: {}.", deviceId);
    }

    private void checkArchiveBusinessIp(String deviceId, InitNetworkBody initNetworkBody, boolean isLld,
        Map<String, NodeInfoDto> manageIpAndNodeMap) {
        ArchiveNetworkConfig archiveNetworkConfig = Optional.ofNullable(initNetworkBody.getArchiveNetworkConfig())
            .orElseGet(ArchiveNetworkConfig::new);
        List<NodeNetworkInfoRequest> archiveInitNetworkInfoList = archiveNetworkConfig.getPacificInitNetWorkInfoList();
        // 归档网络可以不配，所有节点都配，或者都不配
        if (VerifyUtil.isEmpty(archiveInitNetworkInfoList)) {
            log.info("Archive init network info list is empty, deviceId: {}.", deviceId);
            return;
        }
        // 检查管理ip：不能少，不能重复，必须包含所有节点，不能包含dm上不存在的管理ip
        List<String> allNodeManageIpList = new ArrayList<>(manageIpAndNodeMap.keySet());
        pacificClusterService.checkManageIp(allNodeManageIpList, archiveInitNetworkInfoList, isLld);
        // 检查业务网络配置：每个节点至少一个， 在dm上能找到，
        pacificClusterService.checkBusinessIp(archiveInitNetworkInfoList, manageIpAndNodeMap, isLld);
        log.info("Unified check archive network info success, deviceId: {}.", deviceId);
    }

    private void checkCopyBusinessIp(String deviceId, InitNetworkBody initNetworkBody, boolean isLld,
        Map<String, NodeInfoDto> manageIpAndNodeMap) {
        CopyNetworkConfig copyNetworkConfig = Optional.ofNullable(initNetworkBody.getCopyNetworkConfig())
            .orElseGet(CopyNetworkConfig::new);
        List<NodeNetworkInfoRequest> copyInitNetworkInfoList = copyNetworkConfig.getPacificInitNetWorkInfoList();
        // 复制网络可以不配，所有节点都配，或者都不配
        if (VerifyUtil.isEmpty(copyInitNetworkInfoList)) {
            log.info("Copy init network info list is empty, deviceId: {}.", deviceId);
            return;
        }
        // 检查管理ip：不能少，不能重复，必须包含所有节点，不能包含dm上不存在的管理ip
        List<String> allNodeManageIpList = new ArrayList<>(manageIpAndNodeMap.keySet());
        pacificClusterService.checkManageIp(allNodeManageIpList, copyInitNetworkInfoList, isLld);
        // 检查业务网络配置：每个节点至少一个， 在dm上能找到，
        pacificClusterService.checkBusinessIp(copyInitNetworkInfoList, manageIpAndNodeMap, isLld);
        log.info("Unified check copy network info success, deviceId: {}.", deviceId);
    }

    /**
     * 检查存储用户状态
     *
     * @param deviceId 设备id
     * @param username 用户名
     */
    private void checkStorageUserStatus(String deviceId, String username) {
        StorageResponse<List<UserObjectResponse>> response = deviceUserService.getUser(deviceId, username);
        List<UserObjectResponse> userObjectResponseList = response.getData();
        if (userObjectResponseList.stream()
            .anyMatch(
                userObjectResponse -> InitConstants.DATAPROTECT_ADMIN.equals(userObjectResponse.getName()))) {
            throw new LegoCheckedException(CommonErrorCode.USER_ALREADY_EXIST, "dataprotect_admin already exist.");
        }
        if (userObjectResponseList.stream()
            .filter(userObjectResponse -> SUPER_ADMINISTRATOR_ROLE_TYPE.equals(userObjectResponse.getRoleId()))
            .distinct()
            .count() >= SUPER_USER_SIZE_MAX) {
            throw new LegoCheckedException(CommonErrorCode.SUPER_USER_UPPER_LIMIT, "super administrator >= 2");
        }
    }

    /**
     * 获取设备ip用于openstorageapi访问设备
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceIp() {
        String localStorageDeviceIp = initConfigService.getLocalStorageDeviceIp();
        if (StringUtils.isEmpty(localStorageDeviceIp)) {
            localStorageDeviceIp = openStorageService.getLoginDmIp().get();
            initConfigService.updateLocalStorageDeviceIp(localStorageDeviceIp);
        }
        return localStorageDeviceIp;
    }

    /**
     * 获取设备类型
     *
     * @return deviceIp
     */
    @Override
    public String getDeviceType() {
        return DeviceTypeEnum.OCEAN_PACIFIC.getType();
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
    public void checkLoginResponse(StorageArraySessionResponse arraySessionResponse,
        String username, boolean isAllowUnInitUser) {
        if (!isAllowUnInitUser && arraySessionResponse.getAccountState() == PASSWORD_INIT_NEED_CHANGE) {
            log.error("Check user: {} login response failed.", username);
            throw new LegoCheckedException(CommonErrorCode.USER_PASSWORD_IS_NOT_CHANGED, new String[]{username});
        }
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
        try {
            return initializeUserServiceAbility.createUser(authUsername, addUsername, addUserRoleId);
        } catch (DeviceManagerException e) {
            log.error("Create user: {} failed.", addUsername, ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * 初始化存储用户
     *
     * @param addUsername 待创建的用户名
     */
    @Override
    public void initStorageUser(String addUsername) {
        try {
            initializeUserServiceAbility.setNeverExpire(addUsername);
            initializeUserServiceAbility.modifyUserLoginMethod(addUsername);
        } catch (DeviceManagerException e) {
            log.error("Init storage user: {} failed.", addUsername, ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
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
        try {
            String newPwd = RandomPwdUtil.generate(IsmNumberConstant.TWELVE);
            initializeUserServiceAbility.modifyUser(addUsername, password, newPwd);
            return newPwd;
        } catch (DeviceManagerException e) {
            log.error("Change user: {} failed.", addUsername, ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
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
     */
    @Override
    public void initOperationLogStrategy() {
    }
}
