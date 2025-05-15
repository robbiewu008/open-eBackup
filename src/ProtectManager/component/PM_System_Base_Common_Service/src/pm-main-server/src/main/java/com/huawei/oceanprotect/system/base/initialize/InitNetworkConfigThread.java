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
package com.huawei.oceanprotect.system.base.initialize;

import com.huawei.oceanprotect.base.cluster.remote.dorado.service.ClusterStorageService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.repository.bo.UpdateStorageRequest;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.repository.task.LocalStorageScheduler;
import com.huawei.oceanprotect.system.base.constant.InitConfigErrorCode;
import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeBackStorage;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetPlane;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.initialize.network.beans.PortFactory;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.util.SystemTime;
import com.huawei.oceanprotect.system.base.initialize.status.InitStatusService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.bondport.FailOverGroupBondPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.failovergroup.FailovergroupMember;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.vlan.VlanInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.request.ModifyNfsServiceRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;
import com.huawei.oceanprotect.system.base.vo.InitNetWorkParam;

import feign.FeignException;
import feign.codec.DecodeException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.ConfigMapOperationParam;
import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.bean.NetworkBaseInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.InitConstants;
import openbackup.system.base.common.constants.StorageCommonErrorCode;
import openbackup.system.base.common.enums.AddressFamily;
import openbackup.system.base.common.enums.AuthTypeEnum;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.agent.UpdateAgentBusinessIps;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;
import openbackup.system.base.task.InitializedEvent;
import openbackup.system.base.task.UpsertClusterESNEvent;

import org.apache.commons.lang3.StringUtils;
import org.redisson.RedissonShutdownException;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisResponseTimeoutException;
import org.springframework.context.ApplicationContext;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 初始化创建线程
 *
 */
@Slf4j
public class InitNetworkConfigThread extends Thread {
    private static final List<Long> SUPPORT_SINGLE_ERROR_PARAMS = Arrays.asList(StorageCommonErrorCode.DORADO_USER_PSW,
        StorageCommonErrorCode.ACCOUNT_LOCKED, CommonErrorCode.THE_SAME_SUBNETWORK_ERROR,
        CommonErrorCode.SYSTEM_PASSWORD_EXPIRE, CommonErrorCode.USER_IS_NOT_AUTHENTICATED,
        CommonErrorCode.USER_PASSWORD_IS_NOT_CHANGED, CommonErrorCode.RETURN_INVALID_IP,
        InitConfigErrorCode.INITALIZATION_NOT_CONFIGURE_LOGIC_PORT_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_LOGIC_PORT_STATUS_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_NOT_CONFIGURE_BACKUP_NETWORK_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_LOGIC_PORT_SELECT_DATA_PROTOCOL_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_LOGIC_PORT_ROLE_ERROR_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_COPY_REPLICATION_NETWORK_ERROR_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_COPY_BACKUP_NETWORK_ERROR_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_COPY_ARCHIVE_NETWORK_ERROR_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_IP_NAME_OF_LOGIC_PORT_REPEATABLE_EXCEPTION,
        InitConfigErrorCode.INITALIZATION_LOGIC_PORT_NOT_EXIST_EXCEPTION);

    private static final int USER_SIZE_MAX = 128;

    private static final int SUPER_USER_SIZE_MAX = 2;

    /**
     * 超级管理员角色值
     */
    private static final String SUPER_ADMINISTRATOR_ROLE_TYPE = "1";

    private final InitNetworkConfigMapper initNetworkConfigMapper;

    private final DeviceManagerHandler deviceManagerHandler;

    private final InitNetworkBody initNetworkBody;

    private final InitializeBackStorage initializeBackStorage;

    private final LocalStorageService localStorageService;

    private final InfrastructureRestApi infrastructureRestApi;

    private final InitStatusService initStatusService;

    private final SystemTime systemTime;

    private final RedissonClient redissonClient;

    private final LocalStorageScheduler localStorageScheduler;

    private final ClusterStorageService clusterStorageService;

    private String randomPwd = "";

    private final InitializeNetPlane initializeNetPlane;

    private final InitNetworkService initNetworkService;

    private final InitConfigService initConfigService;

    private final SystemService systemService;

    private final boolean isFirstInit;

    private final DeployTypeService deployTypeService;

    private final InfrastructureService infrastructureService;

    private final UpdateAgentBusinessIps updateAllAgentIp;

    private final NetWorkPortService netWorkPortService;

    private final NetworkService networkService;

    private final PortFactory portFactory;

    private final ClusterBasicService clusterBasicService;

    /**
     * 临时变量，和上下文无关
     */
    private DeviceManagerService deviceManagerService;

    private ApplicationContext applicationContext;

    private NfsServiceApi nfsServiceApi;

    /**
     * 线程传参
     *
     * @param initNetworkConfigParams 初始化参数
     */
    public InitNetworkConfigThread(InitNetworkConfigParams initNetworkConfigParams) {
        super.setName("InitNetworkConfigThread");
        initNetworkBody = initNetworkConfigParams.getInitNetworkBody();
        initNetworkConfigMapper = initNetworkConfigParams.getInitNetworkConfigMapper();
        deviceManagerHandler = initNetworkConfigParams.getDeviceManagerHandler();
        initializeBackStorage = initNetworkConfigParams.getInitializeBackStorage();
        localStorageService = initNetworkConfigParams.getLocalStorageService();
        initStatusService = initNetworkConfigParams.getInitStatusService();
        redissonClient = initNetworkConfigParams.getRedissonClient();
        infrastructureRestApi = initNetworkConfigParams.getInfrastructureRestApi();
        localStorageScheduler = initNetworkConfigParams.getLocalStorageScheduler();
        clusterStorageService = initNetworkConfigParams.getClusterStorageService();
        initializeNetPlane = initNetworkConfigParams.getInitializeNetPlane();
        systemTime = new SystemTime();
        applicationContext = initNetworkConfigParams.getApplicationContext();
        initNetworkService = initNetworkConfigParams.getInitNetworkService();
        deviceManagerService = initNetworkConfigParams.getDeviceManagerService();
        initConfigService = initNetworkConfigParams.getInitConfigService();
        systemService = initNetworkConfigParams.getSystemService();
        isFirstInit = initNetworkConfigParams.isFirstInit();
        deployTypeService = initNetworkConfigParams.getDeployTypeService();
        infrastructureService = initNetworkConfigParams.getInfrastructureService();
        updateAllAgentIp = initNetworkConfigParams.getUpdateAgentBusinessIps();
        netWorkPortService = initNetworkConfigParams.getNetWorkPortService();
        networkService = initNetworkConfigParams.getNetworkService();
        portFactory = initNetworkConfigParams.getPortFactory();
        nfsServiceApi = initNetworkConfigParams.getNfsServiceApi();
        clusterBasicService = initNetworkConfigParams.getClusterBasicService();
    }

    /**
     * 初始化执行
     */
    @Override
    @ExterAttack
    public void run() {
        CheckStatusThread checkThread = null;
        log.info("Handling the initialize operation");
        String deviceId = clusterBasicService.getCurrentClusterEsn();
        // 初始化状态
        initStatusService.clrInitConfigStatus(deviceId);

        try {
            checkThread = new CheckStatusThread(redissonClient, InitConfigConstant.INIT_STAUS_FLAG);
            checkThread.start();

            String username = "";
            if (deployTypeService.isPacific()) {
                deviceId = initConfigService.getLocalStorageDeviceId();
                username = initNetworkBody.getStorageAuth().getUsername();
            }
            handleInitialization(deviceId, username);
            return;
        } catch (DeviceManagerException exception) {
            log.error("InitNetworkConfigThread.run(),DeviceManagerException: ", exception);
            // 记录错误码
            initStatusService.setInitProgressCode(String.valueOf(exception.getCode()), deviceId);

            // 错误码参数
            if (SUPPORT_SINGLE_ERROR_PARAMS.contains(exception.getCode())) {
                log.error("InitNetworkConfigThread.run(),DeviceManagerException error params");
                List<String> errorTimes = new ArrayList<>();
                errorTimes.add(exception.getErrorParam());
                initStatusService.setInitProgressParams(errorTimes, deviceId);
            }
        } catch (LegoCheckedException exception) {
            log.error("InitNetworkConfigThread.run(),LegoCheckedException: ", exception);

            // 如下错误码：
            if (SUPPORT_SINGLE_ERROR_PARAMS.contains(exception.getErrorCode())) {
                log.error("InitNetworkConfigThread.run(),DeviceManagerException error params");
                initStatusService.setInitProgressParams(getParameter(exception.getParameters()), deviceId);
            }

            // 记录错误码：
            initStatusService.setInitProgressCode(String.valueOf(exception.getErrorCode()), deviceId);
        } catch (Exception exception) {
            // 其他异常，仅打印
            log.error("InitNetworkConfigThread.run(),Exception: ", exception);

            // 记录错误码
            initStatusService.setInitProgressCode(String.valueOf(CommonErrorCode.OPERATION_FAILED), deviceId);
        } finally {
            updateLocalStorage();
            // 清理密码
            StringUtil.clean(randomPwd);
            if (!VerifyUtil.isEmpty(initNetworkBody.getStorageAuth())) {
                StringUtil.clean(initNetworkBody.getStorageAuth().getPassword());
            }
            if (checkThread != null) {
                try {
                    log.info("stopping monitor thread");
                    checkThread.setExitFlag(true);
                    checkThread.join();
                } catch (InterruptedException exception) {
                    log.error("stop thread failed, exception: ", exception);
                }
            }
        }

        // 如果没有经历上面的return，那么就是流程是错误的。
        updateInitConfigInfoByEsn(InitConfigConstant.MODIFY_NETWORK_ERROR, InitConfigConstant.ERROR_CODE_FAILED,
            deviceId);
    }

    private void updateLocalStorage() {
        if (StringUtils.isNotEmpty(randomPwd)) {
            // 更新认证鉴权信息
            StorageAuth storageAuth = new StorageAuth();
            storageAuth.setUsername(UserUtils.getBusinessUsername());
            storageAuth.setPassword(randomPwd);
            // 更新认证鉴权信息
            updateLocalStorageAuth(storageAuth, AuthTypeEnum.SERVICE_AUTH.getLoginAuthType());
            StringUtil.clean(storageAuth.getPassword());
            updateRedis();
        }
        updateLocalStorageAuth(initNetworkBody.getStorageAuth(), AuthTypeEnum.MANAGER_AUTH.getLoginAuthType());
        // 异步发送事件,初始化完毕
        ThreadPoolTool.getPool()
            .execute(() -> applicationContext.publishEvent(new InitializedEvent(applicationContext)));
    }

    private void updateRedis() {
        try {
            log.info("Update system super already create start");
            redissonClient.getBucket(Constants.SYSTEM_SUPER_ALREADY_CREATE).set(Boolean.TRUE);
            log.info("Update system super already create success");
        } catch (RedissonShutdownException | RedisResponseTimeoutException e) {
            log.error("Update system super already create fail");
        }
    }

    private void handleInitialization(String deviceId, String username) {
        log.info("InitNetworkConfigThread.process:start time: {}", systemTime.getStartDateTime(SystemTime.ALL_TIME));

        if (!deployTypeService.isE1000() && !deployTypeService.isOpenSource() && isFirstInit) {
            // 鉴权/登陆
            authentication();
        }
        initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_05, deviceId);

        // 参数/条件检查
        initNetworkService.unifiedCheck(deviceId, username, initNetworkBody);

        // 更新初始化进度
        initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_10, deviceId);
        log.info("Check start... Time: {}", systemTime.getStartDateTime(SystemTime.PART_TIME).getTime());
        if (!deployTypeService.isE1000() && isFirstInit && !deployTypeService.isPacific()
            && !deployTypeService.isOpenSource()) {
            checkStorageUserStatus(deviceManagerService);
        }
        log.info("Check OK... Time: {}", systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
        log.warn("Init time report, check Param info : {} seconds", systemTime.updateDateTime());

        if (!handleInitializeMainSteps(deviceId, username)) {
            return;
        }
        updateNfsService();
        // 7.3 保存状态
        updateInitConfigInfoByEsn(InitConfigConstant.MODIFY_NETWORK_FINISH, Constants.ERROR_CODE_OK, deviceId);
        // 100%完成
        initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_OK, deviceId);
        redissonClient.getBucket(Constants.SYSTEM_INITIALIZED).set(Boolean.TRUE);
        log.info("InitNetworkConfigThread.process:end time: {}", systemTime.getEndDateTime(SystemTime.ALL_TIME));
        log.warn("InitNetworkConfigThread.process: update time: {} second", systemTime.allUpdateDateTime());
    }

    private void updateNfsService() {
        if (!deployTypeService.isXSeries()) {
            return;
        }
        try {
            log.info("Start to update nfs service.");
            ModifyNfsServiceRequest request = new ModifyNfsServiceRequest();
            request.setSupportNfsV3(Boolean.TRUE);
            request.setSupportNfsV41(Boolean.TRUE);
            request.setSupportNfsV42(Boolean.TRUE);
            request.setVstoreId("0");
            nfsServiceApi.modifyNfsServiceConfig(initConfigService.getLocalStorageDeviceId(),
                UserUtils.getBusinessUsername(), request);
        } catch (LegoCheckedException | FeignException e) {
            log.error("update nfs service failed.", ExceptionUtil.getErrorMessage(e));
        }
    }

    private boolean handleInitializeMainSteps(String deviceId, String username) {
        String initPassword = null;
        try {
            // 查询控制器与k8s节点映射关系
            log.info("Query controller and k8s node relation start ... Time: {}",
                systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
            InitNetWorkParam netWorkParam = initNetworkService.getInitNetWorkParam(initNetworkBody, deviceId,
                username);
            log.info("Query controller and k8s node relation success ... Time: {}",
                systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
            // 更新初始化进度
            initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_20, deviceId);

            // 调用基础设施接口将初始化信息写入到configmap中；network-conf
            log.info("Set controller and k8s node relation start ... Time: {}",
                systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
            writeInitNetworkInfoIntoConfigmap(netWorkParam);
            log.info("Set controller and k8s node relation success ... Time: {}",
                systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
            // 更新初始化进度
            initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_30, deviceId);
            // 创建业务存储账户,软硬解耦不用创
            if (isFirstInit && deployTypeService.isXSeries()) {
                randomPwd = deviceManagerHandler.createStorageUser();
                String localStorageDeviceId = initConfigService.getLocalStorageDeviceId();
                DeviceUser deviceUser = new DeviceUser();
                deviceUser.setUsername(UserUtils.getBusinessUsername());
                deviceUser.setPassword(randomPwd);
                deviceUser.setId(localStorageDeviceId);
                deviceUser.setAuthType(AuthTypeEnum.SERVICE_AUTH.getEntryAuthType());
                String initDeviceId = systemService.configStorageAuth(deviceUser);
                initConfigService.updateLocalStorageDeviceId(initDeviceId);
            }
            if (deployTypeService.isXSeries() && !isFirstInit) {
                infrastructureService.notifyDmeBusinessNetworkChanged();
                updateAllAgentIp.updateAllAgentIp();
            }
            if (deployTypeService.isPacific()) {
                // 创建业务存储账户
                log.info("Create storage user start ... Time: {}",
                    systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
                Integer storageUserRoleId = initNetworkService.getStorageUserRoleId();
                initPassword = initNetworkService.createStorageUser(username, UserUtils.getBusinessUsername(),
                    storageUserRoleId);
                updateServiceUserDeviceSecret(UserUtils.getBusinessUsername(), initPassword);
                randomPwd = initNetworkService.changePass(UserUtils.getBusinessUsername(), initPassword);
                // 更新secret
                updateServiceUserDeviceSecret(UserUtils.getBusinessUsername(), randomPwd);
                initNetworkService.initStorageUser(UserUtils.getBusinessUsername());
                log.info("Create storage user success ... Time: {}",
                    systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
                // 更新初始化进度
                initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_40, deviceId);
            }

            // 更新初始化进度
            initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_50, deviceId);

            // 保存备份网络，归档网络，复制网络信息
            saveInitConfigInfo(netWorkParam, deviceId);
            // 更新初始化进度
            initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_60, deviceId);
            // 初始化PM日志存储策略保留时间
            initNetworkService.initOperationLogStrategy();
        } catch (Exception exception) {
            log.error("initialize main steps failed, exception: ", exception);

            // 记录错误码
            initStatusService.setInitProgressCode(
                String.valueOf(CommonErrorCode.INITALIZATION_UNRECOVERABLE_EXCEPTION), deviceId);
            updateInitConfigInfoByEsn(InitConfigConstant.MODIFY_NETWORK_ERROR, InitConfigConstant.ERROR_CODE_FAILED,
                deviceId);
            return false;
        } finally {
            StringUtil.clean(initPassword);
        }
        return true;
    }

    private void addMemberToFailOverGroup(String homePortId, String failOverGroupId, HomePortType portType) {
        FailovergroupMember failovergroupMember = new FailovergroupMember();
        failovergroupMember.setId(failOverGroupId);
        failovergroupMember.setAssociateObjId(homePortId);
        failovergroupMember.setAssociateObjType(portFactory.createPort(portType).convertToFailOverGroupMemberType());
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        netWorkPortService.addMemberOfFailovergroup(deviceInfo.getEsn(), deviceInfo.getUsername(), failovergroupMember);
    }

    private Map<String, HomePortType> getNeedToAddFgPhyicalPortIds(List<LogicPortAddRequest> logicPortList,
        LogicPortAddRequest port) {
        NetworkBaseInfo source = toNetworkBaseInfo(port);
        return logicPortList.stream()
            .filter(portTmp -> networkService.isSameSubNetwork(source, toNetworkBaseInfo(portTmp)))
            .collect(Collectors.toMap(LogicPortAddRequest::getHomePortId, LogicPortAddRequest::getHomePortType,
                (existingValue, newValue) -> newValue // 覆盖已有值
            ));
    }

    private List<LogicPortAddRequest> convertLogicPortNameToLogicPortOfDm(List<String> logicPortNames) {
        List<LogicPortAddRequest> logicPortAddRequestList = netWorkPortService
            .queryLogicPorts(systemService.getDeviceInfo().getEsn(), systemService.getDeviceInfo().getUsername())
            .getData()
            .stream()
            .filter(portTmp -> PortRole.SERVICE.equals(portTmp.getRole()))
            .collect(Collectors.toList());
        return logicPortNames.stream().map(name -> logicPortAddRequestList.stream()
                .filter(portTmp -> name.equals(portTmp.getName())).findFirst().orElseGet(LogicPortAddRequest::new))
            .collect(Collectors.toList());
    }

    private void updateFailOverGroup(BackupNetworkConfig backupNetworkConfig) {
        List<String> logicPortNameList = backupNetworkConfig.getLogicPorts().stream().map(LogicPortDto::getName)
            .collect(Collectors.toList());
        log.info("Start to update fail over group, logic port name list: {}.", logicPortNameList);
        List<LogicPortAddRequest> logicPortList = convertLogicPortNameToLogicPortOfDm(logicPortNameList);
        if (VerifyUtil.isEmpty(logicPortList)) {
            log.info("No logic port need to update fail over group.");
            return;
        }
        List<LogicPortAddRequest> vlanLogicPortList = logicPortList.stream()
            .filter(port -> !VerifyUtil.isEmpty(port.getHomePortType()) && port.getHomePortType().isVlanPort())
            .collect(Collectors.toList());
        List<LogicPortAddRequest> ethAndBondLogicPortList = logicPortList.stream()
            .filter(port -> !VerifyUtil.isEmpty(port.getHomePortType()) && !port.getHomePortType().isVlanPort())
            .collect(Collectors.toList());
        updateMemberNeedToAddToFailOverGroup(vlanLogicPortList);
        updateMemberNeedToAddToFailOverGroup(ethAndBondLogicPortList);
        log.info("End to update fail over group.");
    }

    private void updateMemberNeedToAddToFailOverGroup(List<LogicPortAddRequest> logicPortList) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        List<VlanInfo> vlanInfoList = netWorkPortService.queryVlan(deviceInfo.getEsn(), deviceInfo.getUsername())
            .getData();

        logicPortList.forEach(port -> updateFailOverGroup(logicPortList, deviceInfo, vlanInfoList, port));
    }

    private void updateFailOverGroup(List<LogicPortAddRequest> logicPortList, DeviceInfo deviceInfo,
        List<VlanInfo> vlanInfoList, LogicPortAddRequest port) {
        Map<String, HomePortType> needToAddFgPhyicalPortIdMap = getNeedToAddFgPhyicalPortIds(logicPortList, port);
        InitConfigInfo initConfigInfo = queryInitConfigByTypeAndEsn(
            InitNetworkConfigConstants.FAIL_OVER_GROUP + port.getName(), deviceInfo.getEsn());
        if (VerifyUtil.isEmpty(initConfigInfo.getInitValue()) || VerifyUtil.isEmpty(needToAddFgPhyicalPortIdMap)) {
            log.info("Skip update fail over group.");
            return;
        }
        Map<String, HomePortType> memberPortIdMap = getFailOverGroupAllMemberPort(initConfigInfo.getInitValue());
        needToAddFgPhyicalPortIdMap.keySet().forEach(id -> {
            if (!memberPortIdMap.containsKey(id)) {
                if (isVlanSameSubNetworkAndTagOrEthOrBond(vlanInfoList, id, port.getHomePortId())) {
                    log.info("Add member: {} to fail over group: {}", id, initConfigInfo.getInitValue());
                    addMemberToFailOverGroup(id, initConfigInfo.getInitValue(),
                        needToAddFgPhyicalPortIdMap.get(id));
                }
            }
        });
        memberPortIdMap.keySet().forEach(id -> {
            if (!needToAddFgPhyicalPortIdMap.containsKey(id)) {
                log.info("Remove member: {} to fail over group: {}", id, initConfigInfo.getInitValue());
                removeMemberOfFailOverGroup(id, initConfigInfo.getInitValue(), memberPortIdMap.get(id));
            }
        });
    }

    private boolean isVlanSameSubNetworkAndTagOrEthOrBond(List<VlanInfo> vlanInfoList, String memberId,
        String sourceId) {
        if (isEthOrBondPort(vlanInfoList, memberId, sourceId)) {
            log.info("Eth and bond port just return, memberId: {}, sourceId: {}.", memberId, sourceId);
            return true;
        }
        VlanInfo member = vlanInfoList.stream().filter(vlanInfo -> memberId.equals(vlanInfo.getId())).findFirst()
            .orElseGet(VlanInfo::new);
        VlanInfo source = vlanInfoList.stream().filter(vlanInfo -> sourceId.equals(vlanInfo.getId())).findFirst()
            .orElseGet(VlanInfo::new);
        log.info("Vlan port need to check tag, member tag: {}, source tag: {}.", member.getTag(), source.getTag());
        return member.getTag().equals(source.getTag());
    }

    private boolean isEthOrBondPort(List<VlanInfo> vlanInfoList, String memberVlanId, String sourceVlanId) {
        List<String> vlanIdList = vlanInfoList.stream().map(VlanInfo::getId).collect(Collectors.toList());
        return !vlanIdList.contains(sourceVlanId) || !vlanIdList.contains(memberVlanId);
    }

    private Map<String, HomePortType> getFailOverGroupAllMemberPort(String failOverGroupId) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        Map<String, HomePortType> result = new HashMap<>();
        result.putAll(getEthPortMap(deviceInfo.getEsn(), deviceInfo.getUsername(), failOverGroupId));
        result.putAll(getBondPortMap(deviceInfo.getEsn(), deviceInfo.getUsername(), failOverGroupId));
        result.putAll(getVlanMap(deviceInfo.getEsn(), deviceInfo.getUsername(), failOverGroupId));
        return result;
    }

    private Map<String, HomePortType> getEthPortMap(String esn, String username, String failOverGroupId) {
        List<EthPort> ethPorts = Optional.ofNullable(netWorkPortService.queryEthPortFailovergroupMember(esn, username,
                failOverGroupId, InitNetworkConfigConstants.FAIL_OVER_GROUP_TYPE).getData())
            .orElse(Collections.emptyList());
        return ethPorts.stream().collect(Collectors.toMap(EthPort::getId, ethPort -> HomePortType.ETHERNETPORT));
    }

    private Map<String, HomePortType> getBondPortMap(String esn, String username, String failOverGroupId) {
        List<FailOverGroupBondPort> bondPorts = Optional
            .ofNullable(netWorkPortService.queryBondPortFailovergroupMember(esn, username, failOverGroupId,
                InitNetworkConfigConstants.FAIL_OVER_GROUP_TYPE).getData())
            .orElse(Collections.emptyList());
        return bondPorts.stream()
            .collect(Collectors.toMap(FailOverGroupBondPort::getId, bondPort -> HomePortType.BINDING));
    }

    private Map<String, HomePortType> getVlanMap(String esn, String username, String failOverGroupId) {
        List<VlanInfo> vlanInfos = Optional.ofNullable(netWorkPortService.queryVlanFailovergroupMember(esn, username,
                failOverGroupId, InitNetworkConfigConstants.FAIL_OVER_GROUP_TYPE).getData())
            .orElse(Collections.emptyList());
        return vlanInfos.stream().collect(Collectors.toMap(VlanInfo::getId, vlanInfo -> HomePortType.VLAN));
    }

    private void removeMemberOfFailOverGroup(String homePortId, String failOverGroupId, HomePortType portType) {
        DeviceInfo deviceInfo = systemService.getDeviceInfo();
        netWorkPortService.movefailovergroupMember(deviceInfo.getEsn(), deviceInfo.getUsername(), failOverGroupId,
            homePortId, portFactory.createPort(portType).convertToFailOverGroupMemberType());
    }

    private NetworkBaseInfo toNetworkBaseInfo(LogicPortAddRequest port) {
        NetworkBaseInfo result = new NetworkBaseInfo();
        if (AddressFamily.IPV4.equals(port.getAddressFamily())) {
            result.setIp(port.getIpv4Addr());
            result.setMask(port.getIpv4Mask());
            result.setIpType(AddressFamily.IPV4);
        } else {
            result.setIp(port.getIpv6Addr());
            result.setMask(port.getIpv6Mask());
            result.setIpType(AddressFamily.IPV6);
        }
        return result;
    }

    private void updateInitConfigInfoByEsn(int modifyNetworkError, int errorCodeFailed, String deviceId) {
        if (!isFirstInit) {
            updateInitConfigInfoByTypeAndEsn(Constants.MODIFY_STATUS_FLAG,
                String.valueOf(modifyNetworkError), deviceId);
        } else {
            updateInitConfigInfoByTypeAndEsn(Constants.INIT_ERROR_FLAG,
                String.valueOf(errorCodeFailed), deviceId);
        }
    }

    private void updateServiceUserDeviceSecret(String username, String password) {
        String deviceId = initConfigService.getLocalStorageDeviceId();
        DeviceUser deviceUser = new DeviceUser();
        deviceUser.setId(deviceId);
        deviceUser.setUsername(username);
        deviceUser.setPassword(password);
        deviceUser.setAuthType(AuthTypeEnum.SERVICE_AUTH.getEntryAuthType());

        log.info("Update service user info, device id: {}.", deviceId);
        systemService.updateServiceUserDeviceSecret(deviceUser);
    }

    private void writeInitNetworkInfoIntoConfigmap(InitNetWorkParam netWorkParam) {
        callInfrastructureInterface(Constants.BACKUP_NET_PLANE, netWorkParam.getBackupNetPlane());
        callInfrastructureInterface(Constants.ARCHIVE_NET_PLANE, netWorkParam.getArchiveNetPlane());
        callInfrastructureInterface(Constants.REPLICATION_NET_PLANE,
            netWorkParam.getReplicationNetPlane());
    }

    private void callInfrastructureInterface(String netPlaneName, String netPlaneInfo) {
        // 第一次初始化的时候，配置文件没有data字段。直接查询会报500错误,有data字段后再查询会报json解析错误
        InfraResponseWithError<String> response = null;
        InfraResponseWithError<List<JSONObject>> commonConfValue = null;
        String defaultNetPlaneInfo = JSONObject.writeValueAsString(new ArrayList<>());
        try {
            commonConfValue = infrastructureRestApi.getCommonConfValue(Constants.NAME_SPACE,
                Constants.CONFIG_MAP, netPlaneName);
        } catch (DecodeException | LegoUncheckedException e) {
            log.info("Starting to add fields: {} to configmap", netPlaneName);
            response = infrastructureRestApi.createConfigMapInfo(new ConfigMapOperationParam(Constants.NAME_SPACE,
                Constants.CONFIG_MAP, netPlaneName, Optional.ofNullable(netPlaneInfo).orElse(defaultNetPlaneInfo)));
        }
        if (!VerifyUtil.isEmpty(commonConfValue) && !VerifyUtil.isEmpty(commonConfValue.getData())) {
            response = infrastructureRestApi.setConfigMapInfo(new ConfigMapOperationParam(Constants.NAME_SPACE,
                Constants.CONFIG_MAP, netPlaneName, Optional.ofNullable(netPlaneInfo).orElse(defaultNetPlaneInfo)));
        }
        if (VerifyUtil.isEmpty(response) || !"success".equals(response.getData())) {
            log.error("Failed to call the infrastructure interface: create or set configmap.");
            throw new LegoCheckedException("Failed to call the infrastructure interface.");
        }
    }

    private void saveInitConfigInfo(InitNetWorkParam initNetWorkParam, String deviceId) {
        // 7.1 保存备份网络信息
        insertInitConfigInfo(Constants.BACKUP_NETWORK_FLAG, initNetWorkParam.getBackupNetPlane(), deviceId);

        // 7.2 保存归档网络信息
        insertInitConfigInfo(Constants.ARCHIVE_NETWORK_FLAG, initNetWorkParam.getArchiveNetPlane(), deviceId);

        // 7.3 保存复制网络信息
        insertInitConfigInfo(Constants.REPLICATION_NETWORK_FLAG, initNetWorkParam.getReplicationNetPlane(), deviceId);
    }

    private void authentication() {
        log.info("Create device manager service start ... Time: {}",
            systemTime.getStartDateTime(SystemTime.PART_TIME).getTime());
        StorageAuth storageAuth = initNetworkBody.getStorageAuth();
        String localStorageDeviceId = initConfigService.getLocalStorageDeviceId();
        DeviceUser deviceUser = new DeviceUser();
        deviceUser.setUsername(storageAuth.getUsername());
        deviceUser.setPassword(storageAuth.getPassword());
        deviceUser.setId(localStorageDeviceId);
        deviceUser.setAuthType(AuthTypeEnum.MANAGER_AUTH.getEntryAuthType());
        String deviceId = systemService.configStorageAuth(deviceUser);
        log.info("authentication success, ESN:{}", deviceId);
        if (StringUtils.isNotBlank(deviceId)) {
            log.info("set initial_esn redis bucket");
            redissonClient.getBucket(InitConstants.INITIAL_ESN).set(deviceId);
            applicationContext.publishEvent(new UpsertClusterESNEvent(applicationContext, deviceId));
        }
        initStatusService.setInitProgressRate(InitConfigConstant.PROGRESS_RATE_05, deviceId);
        log.info("Create device manager service OK!...Time: {}",
            systemTime.getEndDateTime(SystemTime.PART_TIME).getTime());
        log.warn("init time report, Auth User info : {} seconds", systemTime.updateDateTime());
    }

    private void insertInitConfigInfo(String initType, String initValue, String deviceId) {
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitType(initType);
        initConfigInfo.setInitValue(initValue);
        initConfigInfo.setEsn(deviceId);
        initNetworkConfigMapper.insertInitConfig(initConfigInfo);
    }

    private void updateInitConfigInfo(String initType, String initValue, String deviceId) {
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitType(initType);
        initConfigInfo.setInitValue(initValue);
        initConfigInfo.setEsn(deviceId);
        initNetworkConfigMapper.updateInitConfig(initConfigInfo);
    }

    private void updateInitConfigInfoByTypeAndEsn(String initType, String initValue, String esn) {
        InitConfigInfo initConfigInfo = new InitConfigInfo();
        initConfigInfo.setInitType(initType);
        initConfigInfo.setInitValue(initValue);
        initConfigInfo.setEsn(esn);
        initNetworkConfigMapper.updateInitConfigByEsnAndType(initConfigInfo);
    }

    private InitConfigInfo queryInitConfigByTypeAndEsn(String initType, String esn) {
        return initNetworkConfigMapper.queryInitConfigByEsnAndType(initType, esn).stream().findFirst()
            .orElse(new InitConfigInfo());
    }


    private void updateLocalStorageAuth(StorageAuth storageAuth, String authType) {
        log.info("Begin to update the storage info");
        try {
            UpdateStorageRequest request = new UpdateStorageRequest();
            request.setUserName(storageAuth.getUsername());
            request.setPassword(storageAuth.getPassword());
            request.setAuthType(authType);
            localStorageService.updateLocalStorage(request);
            localStorageScheduler.monitorLocalRepository();
        } catch (Exception exception) {
            log.error("Failed to update the storage info");
        }
        log.info("Finished to update the storage info");
    }

    private List<String> getParameter(String[] parameters) {
        List<String> parameter = new ArrayList<>();
        for (int index = 0; index < parameters.length; index++) {
            parameter.add(parameters[index]);
        }
        return parameter;
    }

    private void checkStorageUserStatus(DeviceManagerService service) {
        List<UserObjectResponse> userObjectResponses = service.getApiRest(UserRest.class)
            .getUser(service.getDeviceId());
        if (userObjectResponses.stream().distinct().count() >= USER_SIZE_MAX) {
            throw new LegoCheckedException(CommonErrorCode.USER_UPPER_LIMIT, "user size > 128");
        }

        if (userObjectResponses.stream()
            .filter(userObjectResponse -> SUPER_ADMINISTRATOR_ROLE_TYPE.equals(userObjectResponse.getRoleId()))
            .distinct()
            .count() >= SUPER_USER_SIZE_MAX) {
            throw new LegoCheckedException(CommonErrorCode.SUPER_USER_UPPER_LIMIT, "super administrator > 2");
        }

        if (userObjectResponses.stream()
            .anyMatch(
                userObjectResponse -> UserUtils.getBusinessUsername().equals(userObjectResponse.getName()))) {
            throw new LegoCheckedException(CommonErrorCode.USER_ALREADY_EXIST, "dataprotect admin already exist");
        }
    }
}
