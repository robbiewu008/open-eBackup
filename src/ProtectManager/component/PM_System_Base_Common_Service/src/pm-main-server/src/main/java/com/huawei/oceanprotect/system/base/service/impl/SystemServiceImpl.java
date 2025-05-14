/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.repository.dao.ProductStorageDao;
import com.huawei.oceanprotect.repository.entity.ProductStorage;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.dto.dorado.ManualInitPortDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.DependentInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldCopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualCopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificCopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitServiceType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.PacificService;
import com.huawei.oceanprotect.system.base.service.SystemService;
import com.huawei.oceanprotect.system.base.service.impl.dependent.DependentInitNetworkServiceImpl;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.entity.IpPoolDto;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 系统服务实现类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-04
 */
@Service
@Slf4j
public class SystemServiceImpl implements SystemService {
    @Autowired
    private InitConfigService initConfigService;


    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private PacificService pacificService;

    @Autowired
    private DependentInitNetworkServiceImpl dependentInitNetworkService;

    @Autowired
    private OpenStorageService openStorageService;

    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private ClusterBasicService clusterBasicService;

    @Autowired
    private ProductStorageDao productStorageDao;

    @Autowired
    private ProviderRegistry registry;

    /**
     * 获取pacific 多节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @return 业务网络配置信息
     */
    @Override
    public NetworkInfoDto getNetworkInfo(String manageIp) {
        if (deployTypeService.isE1000()) {
            return dependentInitNetworkService.getNetworkInfoForDecoupled(manageIp);
        }

        StorageAuth storageAuth = new StorageAuth();
        try {
            storageAuth = initConfigService.getLocalStorageAuth();
            String deviceId = initConfigService.getLocalStorageDeviceId();
            log.info("Get network info, deviceId: {}.", deviceId);
            return pacificService.getNetworkInfo(deviceId, storageAuth.getUsername(), manageIp);
        } catch (LegoCheckedException e) {
            log.error("Get network info failed.", ExceptionUtil.getErrorMessage(e));
            throw e;
        } finally {
            StringUtil.clean(storageAuth.getPassword());
        }
    }

    /**
     * 获取pacific 指定节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @param ifaceName 端口
     * @param ipAddress ip地址和掩码
     * @return 业务网络配置信息
     */
    @Override
    public NodeNetworkInfoDto getNodeNetworkInfo(String manageIp, String ifaceName, String ipAddress) {
        log.info("Get node network info start, manageIp: {}, ifaceName: {}, ipAddress: {}.",
            manageIp, ifaceName, ipAddress);
        try {
            NetworkInfoDto networkInfo = getNetworkInfo(manageIp);
            List<NodeNetworkInfoDto> nodeNetworkInfoVoList = networkInfo.getNodeNetworkInfoList();
            if (nodeNetworkInfoVoList.isEmpty()) {
                log.info("Get empty node network info, manageIp: {}, ifaceName: {}, ipAddress: {}.",
                    manageIp, ifaceName, ipAddress);
                return new NodeNetworkInfoDto(manageIp, new ArrayList<>());
            }
            return buildNodeNetworkInfo(nodeNetworkInfoVoList, manageIp, ifaceName, ipAddress);
        } catch (LegoCheckedException e) {
            log.error("Get node network info failed, manageIp: {}, ifaceName: {}, ipAddress: {}.",
                manageIp, ifaceName, ipAddress, ExceptionUtil.getErrorMessage(e));
            throw e;
        }
    }

    // 根据nodeNetworkInfoList创建NodeNetworkInfo
    private NodeNetworkInfoDto buildNodeNetworkInfo(List<NodeNetworkInfoDto> nodeNetworkInfoVoList, String manageIp,
        String ifaceName, String ipAddress) {
        NodeNetworkInfoDto nodeNetworkInfo = nodeNetworkInfoVoList.get(LegoNumberConstant.ZERO);
        List<IpPoolDto> ipPoolDtoList = nodeNetworkInfo.getIpPoolDtoList();
        List<IpPoolDto> filteredIpPoolDtoList = ipPoolDtoList.stream()
            .filter(ipPoolDto -> ipPoolDto.getIfaceName().contains(Optional.ofNullable(ifaceName).orElse(
                Strings.EMPTY))
                && ipPoolDto.getIpAddress().contains(Optional.ofNullable(ipAddress).orElse(Strings.EMPTY)))
            .collect(Collectors.toList());
        nodeNetworkInfo.setIpPoolDtoList(filteredIpPoolDtoList);
        log.info("Get node network info success, manageIp: {}, ifaceName: {}, ipAddress: {}.",
            manageIp, ifaceName, ipAddress);
        return nodeNetworkInfo;
    }

    /**
     * 配置存储认证，成功会将存储认证信息写入secret，并返回设备id
     *
     * @param deviceUser 认证信息
     * @return deviceId
     */
    @Override
    public String configStorageAuth(DeviceUser deviceUser) {
        return openStorageService.configStorageAuth(deviceUser, false, true);
    }

    /**
     * 更新指定设备和用户的secret
     *
     * @param deviceUser deviceUser
     */
    @Override
    public void updateServiceUserDeviceSecret(DeviceUser deviceUser) {
        openStorageService.updateServiceUserDeviceSecret(deviceUser);
    }

    @Override
    public DeviceInfo getDeviceInfo() {
        if (isInitialized()) {
            return getDeviceInfoWhenInited();
        }
        return getDeviceInfoWhenNotInited();
    }

    private DeviceInfo getDeviceInfoWhenNotInited() {
        String esn = clusterBasicService.getCurrentClusterEsn();
        String username = initConfigService.getLocalStorageAuth().getUsername();
        return new DeviceInfo(esn, username);
    }

    private DeviceInfo getDeviceInfoWhenInited() {
        String esn = clusterBasicService.getCurrentClusterEsn();
        ProductStorage storage = Optional.ofNullable(productStorageDao.findProductStorageById(esn))
            .orElseThrow(() -> {
                log.error("System error, operating accounts lost.");
                return new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "System error, operating accounts lost.");
            });
        return new DeviceInfo(esn, storage.getUserName());
    }

    @Override
    public boolean isInitialized() {
        List<InitConfigInfo> initConfigInfos =
            initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG);
        if (!VerifyUtil.isEmpty(initConfigInfos)) {
            return StringUtils.equals(initConfigInfos.get(0).getInitValue(),
                String.valueOf(Constants.ERROR_CODE_OK));
        }
        return false;
    }

    @Override
    public ConfigStatus createManualInitConfig(ManualInitNetworkBody manualInitNetworkBody) {
        return createInitConfig(convertToInitParam(manualInitNetworkBody));
    }

    @Override
    public ConfigStatus createLldInitConfig(LldInitNetworkBody lldInitNetworkBody) {
        return createInitConfig(convertToInitParam(lldInitNetworkBody));
    }

    @Override
    public ConfigStatus createPacificInitConfig(PacificInitNetworkBody pacificInitNetworkBody) {
        return createInitConfig(convertToInitParam(pacificInitNetworkBody));
    }

    @Override
    public ConfigStatus createDependentInitConfig(DependentInitNetworkBody dependentInitNetworkBody) {
        return createInitConfig(convertToInitParam(dependentInitNetworkBody));
    }

    private InitNetworkBody convertToInitParam(ManualInitNetworkBody manualInitNetworkBody) {
        InitNetworkBody result = new InitNetworkBody();
        BeanUtils.copyProperties(manualInitNetworkBody, result);

        List<LogicPortDto> backupPortList = new ArrayList<>();
        convertPortInfo(manualInitNetworkBody.getBackupNetworkConfig().getLogicPorts(), backupPortList);
        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        backupNetworkConfig.setLogicPorts(backupPortList);

        List<LogicPortDto> replicationPortList = new ArrayList<>();
        List<ManualInitPortDto> manualCopyPortList = Optional.ofNullable(manualInitNetworkBody.getCopyNetworkConfig())
                .map(ManualCopyNetworkConfig::getLogicPorts).orElse(new ArrayList<>());
        convertPortInfo(manualCopyPortList, replicationPortList);
        CopyNetworkConfig copyNetworkConfig = new CopyNetworkConfig();
        copyNetworkConfig.setLogicPorts(replicationPortList);

        List<LogicPortDto> archivebackupPortList = new ArrayList<>();
        List<ManualInitPortDto> manualArchivePortList = Optional
                .ofNullable(manualInitNetworkBody.getArchiveNetworkConfig())
                .map(ManualArchiveNetworkConfig::getLogicPorts).orElse(new ArrayList<>());
        convertPortInfo(manualArchivePortList, archivebackupPortList);
        ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
        archiveNetworkConfig.setLogicPorts(archivebackupPortList);

        result.setBackupNetworkConfig(backupNetworkConfig);
        result.setCopyNetworkConfig(copyNetworkConfig);
        result.setArchiveNetworkConfig(archiveNetworkConfig);

        return result;
    }

    private InitNetworkBody convertToInitParam(LldInitNetworkBody lldInitNetworkBody) {
        InitNetworkBody result = new InitNetworkBody();
        BeanUtils.copyProperties(lldInitNetworkBody, result);

        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        backupNetworkConfig.setLogicPorts(lldInitNetworkBody.getBackupNetworkConfig().getLogicPorts());
        result.setBackupNetworkConfig(backupNetworkConfig);

        List<LogicPortDto> lldCopyPortList = Optional.ofNullable(lldInitNetworkBody.getCopyNetworkConfig())
                .map(LldCopyNetworkConfig::getLogicPorts).orElse(new ArrayList<>());
        CopyNetworkConfig copyNetworkConfig = new CopyNetworkConfig();
        copyNetworkConfig.setLogicPorts(lldCopyPortList);
        result.setCopyNetworkConfig(copyNetworkConfig);

        List<LogicPortDto> lldArchivePortList = Optional.ofNullable(lldInitNetworkBody.getArchiveNetworkConfig())
                .map(LldArchiveNetworkConfig::getLogicPorts).orElse(new ArrayList<>());
        ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
        archiveNetworkConfig.setLogicPorts(lldArchivePortList);
        result.setArchiveNetworkConfig(archiveNetworkConfig);

        return result;
    }

    private InitNetworkBody convertToInitParam(PacificInitNetworkBody pacificInitNetworkBody) {
        InitNetworkBody result = new InitNetworkBody();
        BeanUtils.copyProperties(pacificInitNetworkBody, result);

        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        backupNetworkConfig.setPacificInitNetWorkInfoList(
                pacificInitNetworkBody.getBackupNetworkConfig().getPacificInitNetWorkInfoList());
        result.setBackupNetworkConfig(backupNetworkConfig);

        List<NodeNetworkInfoRequest> pacificCopyPortList = Optional
                .ofNullable(pacificInitNetworkBody.getCopyNetworkConfig())
                .map(PacificCopyNetworkConfig::getPacificInitNetWorkInfoList).orElse(new ArrayList<>());
        CopyNetworkConfig copyNetworkConfig = new CopyNetworkConfig();
        copyNetworkConfig.setPacificInitNetWorkInfoList(pacificCopyPortList);
        result.setCopyNetworkConfig(copyNetworkConfig);

        List<NodeNetworkInfoRequest> pacificArchivePortList = Optional
                .ofNullable(pacificInitNetworkBody.getArchiveNetworkConfig())
                .map(PacificArchiveNetworkConfig::getPacificInitNetWorkInfoList).orElse(new ArrayList<>());
        ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
        archiveNetworkConfig.setPacificInitNetWorkInfoList(pacificArchivePortList);
        result.setArchiveNetworkConfig(archiveNetworkConfig);

        return result;
    }

    private InitNetworkBody convertToInitParam(DependentInitNetworkBody dependentInitNetworkBody) {
        InitNetworkBody result = new InitNetworkBody();
        BeanUtils.copyProperties(dependentInitNetworkBody, result);

        return result;
    }

    private void convertPortInfo(List<ManualInitPortDto> source, List<LogicPortDto> target) {
        target.addAll(source.stream().map(port -> {
            LogicPortDto logicPortDto = new LogicPortDto();
            logicPortDto.setName(port.getName());
            return logicPortDto;
        }).collect(Collectors.toList()));
    }

    private ConfigStatus createInitConfig(InitNetworkBody initNetworkBody) {
        ConfigStatus status = new ConfigStatus();
        status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);

        InitNetworkServiceImpl provider = registry.findProvider(InitNetworkServiceImpl.class,
                InitServiceType.INIT_NETWORK.getType());
        String initStatus = provider.init(initNetworkBody);
        if (InitConfigConstant.INIT_READY_SUCCESS.equals(initStatus)) {
            status.setStatus(Constants.ERROR_CODE_OK);
            return status;
        }
        return status;
    }
}
