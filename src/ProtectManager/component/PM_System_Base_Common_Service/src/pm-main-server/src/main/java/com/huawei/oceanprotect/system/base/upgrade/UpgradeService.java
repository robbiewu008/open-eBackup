/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.upgrade;

import static com.huawei.oceanprotect.system.base.constant.UpgradeConstants.SFTP_DEPLOY_TYPE_LIST;
import static com.huawei.oceanprotect.system.base.constant.UpgradeConstants.SFTP_USER_DTREE_NFS_CLIENT_NAME;
import static com.huawei.oceanprotect.system.base.constant.UpgradeConstants.SFTP_USER_DTREE_NFS_CLIENT_VSTORE_ID;
import static com.huawei.oceanprotect.system.base.constant.UpgradeConstants.SFTP_USER_DTREE_QUERY_RANGE;

import com.huawei.oceanprotect.base.cluster.repository.ClusterRepository;
import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.bo.UpdateStorageRequest;
import com.huawei.oceanprotect.repository.dao.ProductStorageDao;
import com.huawei.oceanprotect.repository.entity.ProductStorage;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.sftp.entity.SftpUser;
import com.huawei.oceanprotect.sftp.service.SftpUserService;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.model.ServicePortPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.nfs.NfsClient;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.AccessVal;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.MsgReturnType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RootSquash;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.SquashType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.LoginAuthRestApi;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsShareServiceApi;
import com.huawei.oceanprotect.system.base.service.InitNetworkService;
import com.huawei.oceanprotect.system.base.service.impl.dorado.DoradoInitNetworkServiceImpl;

import feign.FeignException;
import jodd.util.StringUtil;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.enums.AuthTypeEnum;
import openbackup.system.base.common.enums.IpType;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.system.SystemConfigService;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.secret.DeviceSecretService;
import openbackup.system.base.util.ConfigMapUtil;
import openbackup.system.base.util.OpServiceUtil;
import openbackup.system.base.util.ProviderRegistry;
import openbackup.system.base.util.SQLDistributeLock;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 升级服务
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-15
 */
@Component
@Slf4j
public class UpgradeService implements CommandLineRunner {
    // 升级场景，更新device-secret 锁
    private static final String UPGRADE_DEVICE_SECRET_LOCK = "UPGRADE_DEVICE_SECRET_LOCK";

    /**
     * 归档网络是否更新的标志
     */
    private static final String ARCHIVE_IS_UPDATE_INIT_CONFIG = "IS_UPDATE_INIT_CONFIG";

    /**
     * 备份网络是否更新的标志
     */
    private static final String BACKUP_IS_UPDATE_INIT_CONFIG = "BACKUP_IS_UPDATE_INIT_CONFIG";

    @Autowired
    private ProductStorageDao productStorageDao;

    @Autowired
    private ClusterBasicService clusterBasicService;

    @Autowired
    private DeviceSecretService deviceSecretService;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DoradoInitNetworkServiceImpl doradoInitNetworkService;

    @Autowired
    private LoginAuthRestApi authRestApi;

    @Autowired
    private ClusterRepository clusterRepository;

    @Autowired
    private DeviceManagerHandler deviceManagerHandler;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private LocalStorageService service;

    @Autowired
    private SftpUserService sftpUserService;

    @Autowired
    private NfsShareServiceApi nfsShareServiceApi;

    @Autowired
    private SystemConfigService systemConfigService;

    @Autowired
    private InitializePortService initializePortService;

    @Override
    @SQLDistributeLock(lockName = UPGRADE_DEVICE_SECRET_LOCK, masterOnly = false)
    @Async
    public void run(String... args) {
        log.info("---------------------Start to run---------------------");
        updateLocalStorageSecret();
        updateExternalStorageSecret();
        updateBusinessAuthUser();
        updateSftpUserShareAddress();
        // 更新初始化的底座配置信息
        updateBackupAndArchiveInitConfig();
    }

    private void updateBackupAndArchiveInitConfig() {
        // 如果是1.3/1.5升级上来的没有逻辑端口，需要先写入数据库，才能执行删除创建动作
        initializePortService.writePortsToDbFromConfigWhenUpdateBeforeSix();
        updateInitConfig(Constants.ARCHIVE_NET_PLANE, ARCHIVE_IS_UPDATE_INIT_CONFIG);
        updateInitConfig(Constants.BACKUP_NET_PLANE, BACKUP_IS_UPDATE_INIT_CONFIG);
    }

    /**
     * 方法作用：环境启动时会根据t_config表的配置中的值(IS_UPDATE_INIT_CONFIG)来判断，若为true表示已做更新操作
     *  2025.2.26需要对备份网络同样操作,根据t_config表中的值(BACKUP_NETWORK_IS_UPDATE_INIT_CONFIG)来判断，若为true表示已操作
     *
     * @param netPlane 需要修改的网络平面名称
     * @param isUpdateInitConfigKey 数据库中是否需要更新的key标识
     */
    public void updateInitConfig(String netPlane, String isUpdateInitConfigKey) {
        try {
            log.info("Start to update init config.");
            String isUpdateInitConfig = systemConfigService.getConfigValue(isUpdateInitConfigKey);
            if (!VerifyUtil.isEmpty(isUpdateInitConfig) || !deployTypeService.isXSeries()) {
                return;
            }
            List<LogicPortAddRequest> logicPortList = initializePortService.getLogicPort();
            List<NetWorkConfigInfo> netPlaneList = JSONArray.toCollection(
                (JSONArray.fromObject(ConfigMapUtil.getValueInConfigMap(ConfigMapUtil.NETWORK_CONF, netPlane))),
                NetWorkConfigInfo.class);
            // 处理备份和归档网络平面，复制不涉及
            log.info("Start to handle {} .", netPlane);
            handleNetPlane(netPlaneList, logicPortList);

            systemConfigService.addConfig(isUpdateInitConfigKey, "true");
            log.info("End to update init config.");
        } catch (Exception e) {
            log.error("Fail to update init config.", ExceptionUtil.getErrorMessage(e));
        }
    }

    private void handleNetPlane(List<NetWorkConfigInfo> netWorkConfigInfoList,
        List<LogicPortAddRequest> logicPortList) {
        if (VerifyUtil.isEmpty(netWorkConfigInfoList.isEmpty())) {
            return;
        }
        String deviceId = initializePortService.getDeviceId();

        netWorkConfigInfoList.forEach(logicPort -> {
            for (NetWorkLogicIp netWorkLogicIp : logicPort.getLogicIpList()) {
                try {
                    handleSingleNetPlane(deviceId, netWorkLogicIp, logicPortList);
                } catch (Exception e) {
                    log.error("The ip is refused to delete at DeviceManager.", ExceptionUtil.getErrorMessage(e));
                }
            }
        });
    }

    private void handleSingleNetPlane(String deviceId, NetWorkLogicIp netWorkLogicIp,
        List<LogicPortAddRequest> logicPortList) {
        // 底座查到的所有属性
        LogicPortAddRequest logicPortAddRequest = getLogicPort(netWorkLogicIp, logicPortList);
        // 把 底座查到的所有属性 写入 t_config表
        systemConfigService.addConfig(logicPortAddRequest.getName(),
            JSONObject.writeValueAsString(logicPortAddRequest));
        log.info("End to write logic port information to config table.");
        if (VerifyUtil.isEmpty(logicPortAddRequest.getName())) {
            log.info("The name of logic port request is empty.");
            return;
        }

        InitConfigInfo initConfigInfo = initializePortService.queryInitConfigByTypeAndEsn(logicPortAddRequest.getName(),
            deviceId);
        // pm数据库中查到的数据
        ServicePortPo existServicePort = JSONObject.fromObject(initConfigInfo.getInitValue())
            .toBean(ServicePortPo.class);
        log.info("Handle logic port, name:{}, id:{}, homePortType:{}, role : {}", existServicePort.getName(),
            existServicePort.getId(), existServicePort.getHomePortType(), existServicePort.getRole());
        if (!VerifyUtil.isEmpty(existServicePort.getId())) {
            log.info("Start to delete port from DeviceManager. The name is : {}, id: {}, role: {}, dmRole: {}",
                logicPortAddRequest.getName(), existServicePort.getId(), existServicePort.getRole(),
                existServicePort.getDmRole());
            initializePortService.deleteSingleLogicPortOfDm(logicPortAddRequest.getName(), existServicePort);

            LogicPortDto logicPortDto = getLogicPortByDmAndRequest(existServicePort, logicPortAddRequest);

            log.info(
                "Start to add logic port from DeviceManager. The add logic port name is {}, ip: {}, homePortType: {}, "
                    + "role : {}, dmRole : {}", logicPortDto.getName(), logicPortDto.getIp(),
                logicPortDto.getHomePortType(), logicPortDto.getRole(), logicPortDto.getDmRole());
            initializePortService.addSingleLogicPortOfDm(logicPortDto);
            log.info("End to add logic port from DeviceManager.");
        }
    }

    private LogicPortDto getLogicPortByDmAndRequest(ServicePortPo existServicePort,
        LogicPortAddRequest logicPortAddRequest) {
        switch (logicPortAddRequest.getHomePortType()) {
            case ETHERNETPORT:
                return convertEthLogicPortDto(existServicePort, logicPortAddRequest);
            case BINDING:
                return convertBondLogicPortDto(existServicePort, logicPortAddRequest);
            case VLAN:
                return convertVlanLogicPortDto(existServicePort, logicPortAddRequest);
            default:
                log.error("Create logic port: {} failed, home port type error.", logicPortAddRequest.getName());
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "Create logic port failed, home port type error.");
        }
    }

    private LogicPortDto convertBondLogicPortDto(ServicePortPo servicePortPo, LogicPortAddRequest dmLogicPort) {
        LogicPortDto logicPortDto = new LogicPortDto();
        logicPortDto.setBondPort(servicePortPo.getBondPort());
        convertBasicLogicPortDto(servicePortPo, dmLogicPort, logicPortDto);
        return logicPortDto;
    }

    private LogicPortDto convertEthLogicPortDto(ServicePortPo servicePortPo, LogicPortAddRequest dmLogicPort) {
        LogicPortDto logicPortDto = new LogicPortDto();
        logicPortDto.setHomePortName(dmLogicPort.getHomePortName());
        convertBasicLogicPortDto(servicePortPo, dmLogicPort, logicPortDto);
        return logicPortDto;
    }

    private LogicPortDto convertVlanLogicPortDto(ServicePortPo servicePortPo, LogicPortAddRequest dmLogicPort) {
        LogicPortDto logicPortDto = new LogicPortDto();
        logicPortDto.setVlan(servicePortPo.getVlan());
        if (VlanPortType.ETH.equalsVlanPortType(servicePortPo.getVlan().getPortType().getVlanPortType())) {
            logicPortDto.setHomePortName(dmLogicPort.getHomePortName());
        }
        if (VlanPortType.BOND.equalsVlanPortType(servicePortPo.getVlan().getPortType().getVlanPortType())) {
            logicPortDto.setBondPort(servicePortPo.getBondPort());
        }
        convertBasicLogicPortDto(servicePortPo, dmLogicPort, logicPortDto);
        return logicPortDto;
    }

    private void convertBasicLogicPortDto(ServicePortPo servicePortPo, LogicPortAddRequest dmLogicPort,
        LogicPortDto logicPortDto) {
        logicPortDto.setName(servicePortPo.getName());
        logicPortDto.setHomePortType(servicePortPo.getHomePortType());
        logicPortDto.setHomePortId(dmLogicPort.getHomePortId());

        String ip = VerifyUtil.isEmpty(dmLogicPort.getIpv4Addr())
            ? dmLogicPort.getIpv6Addr()
            : dmLogicPort.getIpv4Addr();
        logicPortDto.setIp(ip);
        String mask = VerifyUtil.isEmpty(dmLogicPort.getIpv4Mask())
            ? dmLogicPort.getIpv6Mask()
            : dmLogicPort.getIpv4Mask();
        logicPortDto.setMask(mask);
        String gateWay = VerifyUtil.isEmpty(dmLogicPort.getIpv4Gateway())
            ? dmLogicPort.getIpv6Gateway()
            : dmLogicPort.getIpv4Gateway();
        logicPortDto.setGateWay(gateWay);
        logicPortDto.setRole(servicePortPo.getRole());
        logicPortDto.setDmRole(servicePortPo.getDmRole());
        String ipType = VerifyUtil.isEmpty(dmLogicPort.getIpv4Addr()) ? IpType.IPV6.getValue() : IpType.IPV4.getValue();
        logicPortDto.setIpType(ipType);
        logicPortDto.setSupportProtocol(dmLogicPort.getSupportProtocol());
        logicPortDto.setHomeControllerId(dmLogicPort.getHomeControllerId());
        logicPortDto.setCurrentControllerId(dmLogicPort.getCurrentControllerId());
        logicPortDto.setFailoverGroupId(dmLogicPort.getFailoverGroupId());
        logicPortDto.setIsFailOver(dmLogicPort.getIsFailOver());
    }

    private LogicPortAddRequest getLogicPort(NetWorkLogicIp netWorkLogicIp, List<LogicPortAddRequest> logicPortList) {
        return logicPortList.stream()
            .filter(logicPort -> logicPort.getIpv4Addr().equals(netWorkLogicIp.getIp()) || logicPort.getIpv6Addr()
                .equals(netWorkLogicIp.getIp()))
            .findFirst()
            .orElseGet(LogicPortAddRequest::new);
    }

    private void updateBusinessAuthUser() {
        StorageAuth storageAuth = new StorageAuth();
        Optional<ProductStorage> productStorage = Optional.empty();
        try {
            log.info("Start update business user");
            productStorage = productStorageDao.findProductStoragesById(clusterBasicService.getCurrentClusterEsn())
                .stream()
                .filter(productStorageItem -> AuthTypeEnum.SERVICE_AUTH.getEntryAuthType()
                    == productStorageItem.getAuthType())
                .findFirst();
            if (!productStorage.isPresent()) {
                log.info("There are no business user in the current environment,skip");
                return;
            }
            storageAuth.setUsername(productStorage.get().getUserName());
            storageAuth.setPassword(encryptorService.decrypt(productStorage.get().getPassword()));
            DeviceManagerService deviceManagerService = deviceManagerHandler.achiveDeviceManagerService(storageAuth);
            List<UserObjectResponse> userObjectResponses = deviceManagerService.getApiRest(UserRest.class)
                .getUser(deviceManagerService.getDeviceId());
            if (userObjectResponses.stream()
                .anyMatch(userObjectResponse -> UserUtils.getBusinessUsername().equals(userObjectResponse.getName()))) {
                log.info("There are business user in the current environment,skip");
                return;
            }
            if (!deployTypeService.isXSeries() || OpServiceUtil.isHcsService()) {
                log.info("Only XSeries need update business user,skip");
                return;
            }
            String randomPwd = deviceManagerHandler.createStorageUser();
            UpdateStorageRequest updateStorageRequest = new UpdateStorageRequest();
            updateStorageRequest.setUserName(UserUtils.getBusinessUsername());
            updateStorageRequest.setPassword(randomPwd);
            // 删除之前的用户
            productStorageDao.deleteProductStorage(clusterBasicService.getCurrentClusterEsn());
            // 更新业务用户
            service.updateLocalStorage(updateStorageRequest);
            // 删除之前的用户的secret
            deviceSecretService.deleteSecret(clusterBasicService.getCurrentClusterEsn(),
                productStorage.get().getUserName());
            log.info("End update business user");
        } catch (Exception e) {
            log.error("Update business user failed.", ExceptionUtil.getErrorMessage(e));
        } finally {
            productStorage.ifPresent(productStorageItem -> {
                openbackup.system.base.common.utils.StringUtil.clean(productStorageItem.getPassword());
            });
            openbackup.system.base.common.utils.StringUtil.clean(storageAuth.getPassword());
        }
    }

    private void updateDeviceUserAuth(ProductStorage productStorage, String currentClusterEsn) {
        try {
            DeviceUser user = new DeviceUser();
            InitNetworkService initNetworkService = registry.findProviderOrDefault(InitNetworkService.class,
                deployTypeService.getDeployType().getValue(), doradoInitNetworkService);
            user.setUsername(productStorage.getUserName());
            user.setPassword(productStorage.getPassword());
            user.setId(currentClusterEsn);
            user.setIp(initNetworkService.getDeviceIp());
            user.setPort(ProtocolPortConstant.DM_MANAGE_PORT);
            user.setDeviceType(initNetworkService.getDeviceType());
            user.setAuthType(productStorage.getAuthType());
            if (!deviceSecretService.createSecret(user)) {
                log.error("Create device info failed, device id: {}, float ip: {}.", user.getId(), user.getIp());
                return;
            }
            log.info("Update success, username: {}.", user.getUsername());
            authRestApi.login(user.getId(), user.getUsername());
        } catch (LegoCheckedException | FeignException exception) {
            log.error("Login osa failed.", ExceptionUtil.getErrorMessage(exception));
        }
    }

    private void updateLocalStorageSecret() {
        try {
            String currentClusterEsn = clusterBasicService.getCurrentClusterEsn();
            if (StringUtil.isEmpty(currentClusterEsn)) {
                log.info("Current cluster esn is empty, can not or no need to upgrade.");
                return;
            }
            List<ProductStorage> productStorageList = productStorageDao.findAll(currentClusterEsn);
            if (CollectionUtils.isEmpty(productStorageList)) {
                log.info("Product storage list is empty, no need to upgrade.");
                return;
            }
            List<DeviceUser> deviceUsers = deviceSecretService.querySecret(currentClusterEsn);
            Map<String, DeviceUser> userNameDeviceUserMap = deviceUsers.stream()
                .collect(Collectors.toMap(DeviceUser::getUsername, Function.identity()));
            Set<String> userNameSet = userNameDeviceUserMap.keySet();
            productStorageList.stream()
                .filter(productStorage -> !userNameSet.contains(productStorage.getUserName()))
                .forEach(productStorage -> updateDeviceUserAuth(productStorage, currentClusterEsn));
        } catch (Exception e) {
            log.error("Update storage failed.", ExceptionUtil.getErrorMessage(e));
        }
    }

    private void updateExternalStorageSecret() {
        try {
            clusterRepository.getAllTargetClusters()
                .stream()
                .filter(this::isNeedUpdateSecret)
                .forEach(this::updateStorageSecret);
        } catch (Exception e) {
            log.error("Update device secret failed.", ExceptionUtil.getErrorMessage(e));
        }
    }

    private boolean isNeedUpdateSecret(TargetCluster cluster) {
        return cluster.getStatus() == ClusterEnum.StatusEnum.ONLINE.getStatus()
            && cluster.getRole() == ClusterEnum.RoleType.BACKUP.getRoleType()
            && ClusterEnum.GeneratedTypeEnum.MANUAL.getType().equals(cluster.getGeneratedType());
    }

    private void updateStorageSecret(TargetCluster cluster) {
        try {
            DeviceUser deviceUser = new DeviceUser();
            deviceUser.setId(cluster.getRemoteEsn());
            deviceUser.setIp(cluster.getIp());
            deviceUser.setUsername(cluster.getUsername());
            deviceUser.setPassword(cluster.getPassword());
            deviceUser.setPort(cluster.getClusterPort());
            deviceUser.setDeviceType(cluster.getDeviceType());
            boolean isSuccess = deviceSecretService.createSecret(deviceUser);
            log.info("Update storage finish, deviceId: {}, result: {}.", cluster.getRemoteEsn(), isSuccess);
        } catch (Exception e) {
            log.error("Update storage failed, deviceId: {}.", cluster.getRemoteEsn(), ExceptionUtil.getErrorMessage(e));
        }
    }

    private void updateSftpUserShareAddress() {
        if (!SFTP_DEPLOY_TYPE_LIST.contains(deployTypeService.getDeployType().getValue())) {
            return;
        }
        try {
            log.info("Update sftp user's nas client scene.");
            List<SftpUser> sftpUsers = sftpUserService.queryAllSftpUser();
            if (CollectionUtils.isEmpty(sftpUsers)) {
                log.info("No sftp users exist.");
                return;
            }
            String esn = clusterBasicService.getCurrentClusterEsn();
            String name = UserUtils.getBusinessUsername();
            sftpUsers.forEach(user -> {
                List<NfsClient> nfsShareClients = Optional.ofNullable(
                    nfsShareServiceApi.getNfsShareClients(esn, name, SFTP_USER_DTREE_QUERY_RANGE,
                        user.getDtreeShareId(), buildQueryNfsClientRequest(user)).getData()).orElse(new ArrayList<>());
                Optional<String> nfsClientName = nfsShareClients.stream()
                    .map(NfsClient::getName)
                    .filter(SFTP_USER_DTREE_NFS_CLIENT_NAME::equals)
                    .findAny();
                if (nfsClientName.isPresent()) {
                    log.info("current sftp user: {} no need to update nas client.", user.getUsername());
                    return;
                }
                NfsClient nfsClientAddRequest = buildAddNfsClientRequest(user);
                nfsShareServiceApi.addNfsShareClient(esn, name, nfsClientAddRequest);
                log.info("Update sftp user: {} dtree nfs client success.", user.getUsername());
            });
        } catch (Exception e) {
            log.error("Update sftp user dtree nfs client failed.", ExceptionUtil.getErrorMessage(e));
        }
    }

    private NfsClient buildQueryNfsClientRequest(SftpUser user) {
        NfsClient nfsClient = new NfsClient();
        nfsClient.setVstoreId(SFTP_USER_DTREE_NFS_CLIENT_VSTORE_ID);
        nfsClient.setParentId(user.getDtreeShareId());
        return nfsClient;
    }

    private NfsClient buildAddNfsClientRequest(SftpUser user) {
        NfsClient nfsClient = new NfsClient();
        nfsClient.setName(SFTP_USER_DTREE_NFS_CLIENT_NAME);
        nfsClient.setAllSquash(SquashType.NO_SQUASH);
        nfsClient.setRootSquash(RootSquash.NO_SQUASH);
        nfsClient.setParentId(user.getDtreeShareId());
        nfsClient.setAccessVal(AccessVal.ALL);
        nfsClient.setSync(MsgReturnType.ASYNC);
        return nfsClient;
    }
}
