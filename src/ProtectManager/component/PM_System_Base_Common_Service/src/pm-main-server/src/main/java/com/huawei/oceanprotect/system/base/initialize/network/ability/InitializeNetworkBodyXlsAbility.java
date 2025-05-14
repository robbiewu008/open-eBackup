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

import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.ARCHIVE_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.BACKUP_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.CONTRONLLER_0A;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.CONTRONLLER_0B;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.CONTRONLLER_0C;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.CONTRONLLER_0D;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.REPLICATION_SERVICE_PORT_LABEL;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM;
import static com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants.SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.constant.InitConfigErrorCode;
import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;
import com.huawei.oceanprotect.system.base.initialize.network.InitializeNetworkBodyXls;
import com.huawei.oceanprotect.system.base.initialize.network.InitializePortService;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitNetworkCfg;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.CopyNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.ExcelNetworkConfigBean;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.ExcelNetworkConfigTypeEnum;
import com.huawei.oceanprotect.system.base.initialize.network.enums.NetPlaneType;
import com.huawei.oceanprotect.system.base.initialize.network.util.NetworkConfigUtils;
import com.huawei.oceanprotect.system.base.model.BondPortPo;
import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RouteType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.NetWorkPortService;
import com.huawei.oceanprotect.system.base.service.SystemService;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LLDConstants;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.enums.IpType;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.network.Ipv4AddressUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.NetworkService;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * 只在OceanProtectXInitDeployTypeStrategyServiceImpl中使用当前类
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Service
@Slf4j
public class InitializeNetworkBodyXlsAbility implements InitializeNetworkBodyXls {
    private static final int SHEET_INDEX = 11;

    private static final int MANAGER_IP = 3;

    private static final int X_START = 4;

    private static final String MANAGE_PORT_TYPE = "2";

    private static final int MIN_BOND_PORT_NAME_LENGTH = 1;

    private static final int MAX_BOND_PORT_NAME_LENGTH = 31;

    private static final String SPLIT_SEPARATOR = "\\.";

    private static final List<String> PORT_TYPE = ImmutableList.of(HomePortType.ETHERNETPORT.getHomePortType(),
        HomePortType.BINDING.getHomePortType());

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private InitializePortService initializePortService;

    @Autowired
    private NetWorkPortService netWorkPortService;

    @Autowired
    private NetworkService networkService;

    @Autowired
    private SystemService systemService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 根据lld获取初始化网络配置信息
     *
     * @param lld lld
     * @return 网络配置信息
     */
    @Override
    public InitNetworkBody checkAndReturnInitXls(MultipartFile lld) {
        log.info("Start get init xls");
        try (InputStream inputStream = lld.getInputStream()) {
            if (!Objects.requireNonNull(lld.getOriginalFilename()).endsWith(LLDConstants.LLD_SUFFIX)) {
                log.error("Lld file type do not .xls");
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Lld file type do not .xls");
            }
            if (!deployTypeService.isSupportInitByLLD()) {
                log.error("Current device is not support init by lld.");
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "Current device is not support init by lld.");
            }
            return getInitXls(new HSSFWorkbook(inputStream));
        } catch (IOException e) {
            log.error("Get lld file failed.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "get lld file failed");
        }
    }

    private InitNetworkBody getInitXls(HSSFWorkbook workBook) {
        HSSFSheet sheet = workBook.getSheetAt(SHEET_INDEX);
        checkSheetName(sheet.getSheetName(), getSheetValue(sheet, 1, 1));
        // 检测传入IP是否为本地IP
        checkManagerIp(sheet, X_START);
        List<ExcelNetworkConfigBean> excelNetworkConfigBeanList = new ArrayList<>();
        int curRow = X_START;
        while (true) {
            HSSFRow row1 = sheet.getRow(curRow);
            if (ObjectUtils.isEmpty(row1) || ObjectUtils.isEmpty(row1.getCell(4))) {
                break;
            }
            Object cellVal = NetworkConfigUtils.getCellValue(row1.getCell(4));
            if (ObjectUtils.isEmpty(cellVal) || cellVal.toString().isEmpty()) {
                break;
            }
            excelNetworkConfigBeanList.add(ExcelNetworkConfigTypeEnum.updateToNetworkConfigBean(row1));
            curRow += 1;
        }
        InitNetworkBody initNetworkBody = new InitNetworkBody();
        List<EthPort> data = netWorkPortService.queryEthPorts(initializePortService.getDeviceId(),
            initializePortService.getUsername()).getData();
        excelNetworkConfigBeanList.forEach(
            excelNetworkConfigBean -> addInitNetworkBody(data, excelNetworkConfigBean, initNetworkBody));
        checkNetworkConfigParams(initNetworkBody);
        return initNetworkBody;
    }

    private void checkManagerIp(HSSFSheet sheet, int rowIndex) {
        List<LogicPortAddRequest> collect = initializePortService.getLogicPort()
            .stream()
            .filter(logicPortAddRequest -> MANAGE_PORT_TYPE.equals(logicPortAddRequest.getLogicalType()))
            .collect(Collectors.toList());
        String managerIp = getSheetValue(sheet, rowIndex, MANAGER_IP);
        String ipType;
        if (Ipv4AddressUtil.isIPv4Address(managerIp)) {
            ipType = InitConfigConstant.IPV4_TYPE_FLAG;
        } else if (Ipv6AddressUtil.isIpv6Address(managerIp)) {
            ipType = InitConfigConstant.IPV6_TYPE_FLAG;
        } else {
            log.error("The manage ip: {} is illegal.", managerIp);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {"manager_ip_init_system_label", managerIp}, "Ip address format is illegal.");
        }
        if (InitConfigConstant.IPV4_TYPE_FLAG.equals(ipType)) {
            if (!collect.stream()
                .map(LogicPortAddRequest::getIpv4Addr)
                .collect(Collectors.toList())
                .contains(managerIp)) {
                log.error("manager ip: {} not local ip", managerIp);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[] {"manager_ip_init_system_label", managerIp}, "manager ip not local ip");
            }
        } else if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            if (!collect.stream()
                .map(LogicPortAddRequest::getIpv6Addr)
                .collect(Collectors.toList())
                .contains(managerIp)) {
                log.error("manager ip: {} not local ip.", managerIp);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[] {"manager_ip_init_system_label", managerIp}, "manager ip not local ip");
            }
        } else {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {"manager_ip_init_system_label", managerIp}, "manager ip not local ip");
        }
    }

    private void checkNetworkConfigParams(InitNetworkBody initNetworkBody) {
        // 备份网络逻辑端口不能为空
        if (VerifyUtil.isEmpty(initNetworkBody.getBackupNetworkConfig())
            || VerifyUtil.isEmpty(initNetworkBody.getBackupNetworkConfig().getLogicPorts())) {
            log.error("Backup network config is empty.");
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"backup_network_config_label", StringUtils.EMPTY}, "Empty backup network config.");
        }
        checkNetworkConfig(initNetworkBody.getBackupNetworkConfig().getLogicPorts());
        checkLogicPortSettings(initNetworkBody.getBackupNetworkConfig().getLogicPorts());
        if (!VerifyUtil.isEmpty(initNetworkBody.getCopyNetworkConfig())) {
            checkNetworkConfig(initNetworkBody.getCopyNetworkConfig().getLogicPorts());
            checkLogicPortSettings(initNetworkBody.getCopyNetworkConfig().getLogicPorts());
        }
        if (!VerifyUtil.isEmpty(initNetworkBody.getArchiveNetworkConfig())) {
            checkNetworkConfig(initNetworkBody.getArchiveNetworkConfig().getLogicPorts());
            checkLogicPortSettings(initNetworkBody.getArchiveNetworkConfig().getLogicPorts());
        }
    }

    private void checkLogicPortSettings(List<LogicPortDto> logicPortDtos) {
        List<LogicPortDto> vlanLogicPorts = new ArrayList<>();
        List<LogicPortDto> nonVlanLogicPorts = new ArrayList<>();
        List<LogicPortDto> bondLogicPorts = new ArrayList<>();
        logicPortDtos.forEach(logicPortDto -> {
            HomePortType newHomePortType = logicPortDto.getHomePortType();
            if (HomePortType.VLAN == newHomePortType) {
                vlanLogicPorts.add(logicPortDto);
                if (VlanPortType.BOND == logicPortDto.getVlan().getPortType()) {
                    bondLogicPorts.add(logicPortDto);
                }
                return;
            }
            if (HomePortType.BINDING == newHomePortType) {
                bondLogicPorts.add(logicPortDto);
                return;
            }
            nonVlanLogicPorts.add(logicPortDto);
        });

        checkBondLogicPortName(bondLogicPorts);

        for (int i = 0; i < vlanLogicPorts.size(); i++) {
            LogicPortDto vlanLogicPort = vlanLogicPorts.get(i);
            for (int j = i + 1; j < vlanLogicPorts.size(); j++) {
                checkTwoVlanLogicPortTag(vlanLogicPort, vlanLogicPorts.get(j));
            }
            nonVlanLogicPorts.forEach(nonVlanLogicPort -> {
                checkVlanAndNonVlanSameSegment(vlanLogicPort, nonVlanLogicPort);
            });
        }
    }

    /**
     * 校验绑定端口名称是否合法
     * 名称不能为空，只能包含字母、数字、_、-、. 和中文字符
     *
     * @param bondLogicPorts 绑定端口名称列表
     */
    public void checkBondLogicPortName(List<LogicPortDto> bondLogicPorts) {
        log.info("start to check bond logic port name validity.");
        checkNameBasicValidity(bondLogicPorts);
    }

    /**
     * 检查传入的绑定端口名字的基本校验
     *
     * @param bondLogicPorts bondLogicPorts
     */
    public void checkNameBasicValidity(List<LogicPortDto> bondLogicPorts) {
        List<BondPortPo> bondPortPoList = bondLogicPorts.stream()
            .map(LogicPortDto::getBondPort)
            .collect(Collectors.toList());
        if (bondPortPoList.isEmpty()) {
            log.info("The bond port name is empty, no need to check bond port name.");
            return;
        }
        bondPortPoList.forEach(bondPortPo -> checkIsValidityBondPortName(bondPortPo.getName()));
    }

    /**
     * 检查绑定端口的名字的组成：只能由字母、数字、中文字符、-、.、_（下划线）组成，不能为空字符串
     *
     * @param name 绑定端口名字
     */
    public void checkIsValidityBondPortName(String name) {
        int nameLength = getBondPortNameAvailableLength(name);
        if (nameLength > MAX_BOND_PORT_NAME_LENGTH || nameLength < MIN_BOND_PORT_NAME_LENGTH) {
            log.error("The length of bond port name is out of length 1~31.");
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {"bond_port_name_label", name}, "Illegal bond logic port name.");
        }

        if (!ValidateUtil.match(RegexpConstants.BOND_PORT_NAME_REGEXP_PATTERN, name)) {
            log.error("The bond port name is not matches.");
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {"bond_port_name_label", name}, "Illegal bond logic port name.");
        }
    }

    /**
     * 获取绑定端口名称的有效字符位数； 注：UTF-8 编码中，1个中文占3个字符
     *
     * @param name 绑定端口名字
     * @return 绑定端口名称的有效字符位数
     */
    public int getBondPortNameAvailableLength(String name) {
        byte[] nameBytes = name.getBytes(Charset.defaultCharset());
        return nameBytes.length;
    }

    private void checkTwoVlanLogicPortTag(LogicPortDto vlan1, LogicPortDto vlan2) {
        if (!StringUtils.equals(vlan1.getIpType(), vlan2.getIpType())) {
            return;
        }
        if (isSameNetworkSegment(vlan1.getIpType(), vlan1.getIp(), vlan1.getMask(), vlan2.getIp(), vlan2.getMask())
            && !StringUtils.equals(vlan1.getVlan().getTags().get(0), vlan2.getVlan().getTags().get(0))) {
            log.error("The tag of same network segment vlan logic ports must be same.");
            throw new LegoCheckedException(InitConfigErrorCode.VLAN_AND_NON_VLAN_COEXIST_ERROR,
                "Illegal logic port type");
        }
    }

    private void checkVlanAndNonVlanSameSegment(LogicPortDto vlan, LogicPortDto nonVlan) {
        if (!StringUtils.equals(vlan.getIpType(), nonVlan.getIpType())) {
            return;
        }
        if (isSameNetworkSegment(vlan.getIpType(), vlan.getIp(), vlan.getMask(), nonVlan.getIp(), nonVlan.getMask())) {
            log.error("Vlan logic port can not coexist with non-vlan logic port.");
            throw new LegoCheckedException(InitConfigErrorCode.VLAN_AND_NON_VLAN_COEXIST_ERROR,
                "Illegal logic port type");
        }
    }

    private boolean isSameNetworkSegment(String ipType, String vlanIp, String vlanMask, String nonVlanIp,
        String nonVlanMask) {
        return IpType.IPV4.getValue().equals(ipType)
            ? networkService.isIpv4SameNetworkSegment(vlanIp, vlanMask, nonVlanIp, nonVlanMask)
            : networkService.isIpv6SameNetworkSegment(vlanIp, vlanMask, nonVlanIp, nonVlanMask);
    }

    private void checkSheetName(String sheetName, String sheetValue) {
        if (StringUtils.equals(sheetName, sheetValue)) {
            log.error("{} and {} not the same", sheetName, sheetValue);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, new String[] {"sheet_name"},
                sheetName + " and " + sheetValue + " not the same");
        }
    }

    private String getSheetValue(HSSFSheet sheet, int rowIndex, int cellNum) {
        HSSFRow row = sheet.getRow(rowIndex);
        HSSFCell cell = row.getCell(cellNum);
        return NetworkConfigUtils.getCellValue(cell).toString();
    }

    private void addInitNetworkBody(List<EthPort> data, ExcelNetworkConfigBean excelNetworkConfigBean,
        InitNetworkBody initNetworkBody) {
        InitNetworkCfg initNetworkCfg = new InitNetworkCfg();
        String servicePort = excelNetworkConfigBean.getServicePortType().split(SPLIT_SEPARATOR)[0];
        if (!NetPlaneType.NET_PLANE_TYPES.contains(servicePort)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"service_port_type_label", servicePort}, "Illegal service port type.");
        }
        NetPlaneType servicePortType = NetPlaneType.forValues(servicePort);
        BeanUtils.copyProperties(excelNetworkConfigBean, initNetworkCfg);
        switch (servicePortType) {
            case BACKUP:
                addBackupNetworkConfig(data, initNetworkCfg, initNetworkBody);
                break;
            case COPY:
                addCopyNetworkConfig(data, initNetworkCfg, initNetworkBody);
                break;
            case ARCHIVE:
                addArchiveNetworkConfig(data, initNetworkCfg, initNetworkBody);
                break;
        }
    }

    private void addBackupNetworkConfig(List<EthPort> data, InitNetworkCfg initNetworkCfg,
        InitNetworkBody initNetworkBody) {
        if (VerifyUtil.isEmpty(initNetworkBody.getBackupNetworkConfig())) {
            List<LogicPortDto> logicPortDtos = new ArrayList<>();
            BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
            backupNetworkConfig.setLogicPorts(logicPortDtos);
            initNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        }
        initNetworkBody.getBackupNetworkConfig().getLogicPorts().add(fillLogicPortDto(data, initNetworkCfg));
    }

    private void addCopyNetworkConfig(List<EthPort> data, InitNetworkCfg initNetworkCfg,
        InitNetworkBody initNetworkBody) {
        if (VerifyUtil.isEmpty(initNetworkBody.getCopyNetworkConfig())) {
            List<LogicPortDto> logicPortDtos = new ArrayList<>();
            CopyNetworkConfig copyNetworkConfig = new CopyNetworkConfig();
            copyNetworkConfig.setLogicPorts(logicPortDtos);
            initNetworkBody.setCopyNetworkConfig(copyNetworkConfig);
        }
        initNetworkBody.getCopyNetworkConfig().getLogicPorts().add(fillLogicPortDto(data, initNetworkCfg));
    }

    private void addArchiveNetworkConfig(List<EthPort> data, InitNetworkCfg initNetworkCfg,
        InitNetworkBody initNetworkBody) {
        if (VerifyUtil.isEmpty(initNetworkBody.getArchiveNetworkConfig())) {
            List<LogicPortDto> logicPortDtos = new ArrayList<>();
            ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
            archiveNetworkConfig.setLogicPorts(logicPortDtos);
            initNetworkBody.setArchiveNetworkConfig(archiveNetworkConfig);
        }
        initNetworkBody.getArchiveNetworkConfig().getLogicPorts().add(fillLogicPortDto(data, initNetworkCfg));
    }

    private LogicPortDto fillLogicPortDto(List<EthPort> data, InitNetworkCfg initNetworkCfg) {
        LogicPortDto logicPortDto = new LogicPortDto();
        fillLogicPortDtoType(logicPortDto, initNetworkCfg);
        fillLogicPortDtoName(data, logicPortDto, initNetworkCfg);
        fillLogicPortDtoIp(logicPortDto, initNetworkCfg);
        fillLogicPortDtoRole(logicPortDto, initNetworkCfg);
        fillLogicPortDtoController(logicPortDto, initNetworkCfg);
        fillLogicPortRoute(logicPortDto, initNetworkCfg);
        return logicPortDto;
    }

    private void fillLogicPortDtoType(LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        String vlanID = initNetworkCfg.getVlanID().split(SPLIT_SEPARATOR)[0];
        String portType = initNetworkCfg.getPortType().split(SPLIT_SEPARATOR)[0];
        if (StringUtils.isEmpty(vlanID)) {
            if (!PORT_TYPE.contains(portType)) {
                log.error("Vlan ID is empty, port type is: {}, need to be 1 or 7.", portType);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"port_type_label", portType}, "Port type is incorrect.");
            } else {
                logicPortDto.setHomePortType(HomePortType.forValues(portType));
            }
        } else {
            logicPortDto.setHomePortType(HomePortType.VLAN);
        }
    }

    private void fillLogicPortDtoName(List<EthPort> data, LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        HomePortType portType = logicPortDto.getHomePortType();
        switch (portType) {
            case ETHERNETPORT:
                fillEthernetPort(data, logicPortDto, initNetworkCfg);
                break;
            case BINDING:
                fillBondingPort(data, logicPortDto, initNetworkCfg);
                break;
            case VLAN:
                fillVlanPort(data, logicPortDto, initNetworkCfg);
                break;
            default:
                log.error("The port type: {} is illegal.", portType);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal port type");
        }
    }

    private void fillEthernetPort(List<EthPort> data, LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        List<String> ethernetPorts = Arrays.stream(initNetworkCfg.getEthernetPort().split(","))
            .collect(Collectors.toList());
        if (ethernetPorts.size() != 1) {
            log.error("Ethernet port size: {}, only can be 1.", ethernetPorts.size());
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"ethernet_port_label", initNetworkCfg.getEthernetPort()}, "Illegal ethernet port num.");
        }
        checkPortExist(data, ethernetPorts);
        logicPortDto.setHomePortName(ethernetPorts.get(0));
    }

    private void fillBondingPort(List<EthPort> data, LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        List<String> bondingPorts = Arrays.stream(initNetworkCfg.getEthernetPort().split(","))
            .collect(Collectors.toList());
        if (bondingPorts.size() < 2) {
            log.error("Bonding ports num: {}, less than 2.", bondingPorts.size());
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {"ethernet_port_label", initNetworkCfg.getEthernetPort()}, "Binding ports num illegal.");
        }
        checkPortExist(data, bondingPorts);
        BondPortPo bondPortPo = new BondPortPo();
        bondPortPo.setMtu(initNetworkCfg.getNetworkInfo().getMtu());
        bondPortPo.setPortNameList(bondingPorts);
        if (VerifyUtil.isEmpty(initNetworkCfg.getBondPortName())) {
            bondPortPo.setName(InitNetworkConfigConstants.BOND_PORT_PREFIX + System.currentTimeMillis());
        } else {
            bondPortPo.setName(initNetworkCfg.getBondPortName());
        }
        logicPortDto.setBondPort(bondPortPo);
    }

    private void fillVlanPort(List<EthPort> data, LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        List<String> vlanPorts = Arrays.stream(initNetworkCfg.getEthernetPort().split(","))
            .collect(Collectors.toList());
        Double portNum = Double.parseDouble(initNetworkCfg.getPortType());
        if (VlanPortType.BOND.equalsVlanPortType(String.valueOf(portNum.intValue()))) {
            fillBondingPort(data, logicPortDto, initNetworkCfg);
        }
        checkPortExist(data, vlanPorts);
        String[] vlanNums = initNetworkCfg.getVlanID().split(",");
        List<String> tags = new ArrayList<>();
        for (String vlanNum : vlanNums) {
            String tag = vlanNum.split(SPLIT_SEPARATOR)[0];
            if (!tag.matches("\\d+") || Long.parseLong(tag) > 4096 || Long.parseLong(tag) < 1) {
                log.error("Vlan ID value: {} is illegal, need to be num in range [1, 4096]", vlanNum);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[] {"vlan_id_init_system_label", tag}, "Illegal vlan-ID");
            }
            tags.add(tag);
        }
        VlanPo vlanPo = new VlanPo();
        vlanPo.setMtu(initNetworkCfg.getNetworkInfo().getMtu());
        vlanPo.setPortNameList(vlanPorts);
        vlanPo.setTags(tags);
        vlanPo.setPortType(VlanPortType.forValues(initNetworkCfg.getPortType().split(SPLIT_SEPARATOR)[0]));
        logicPortDto.setVlan(vlanPo);
    }

    private void checkPortExist(List<EthPort> data, List<String> ports) {
        List<String> ethPorts = data.stream().map(EthPort::getLocation).collect(Collectors.toList());
        ports.forEach(port -> {
            if (!ethPorts.contains(port)) {
                log.error("The ethernet port: {} does not exist.", port);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"ethernet_port_label", port}, "The ethernet port is illegal.");
            }
        });
    }

    private void fillLogicPortDtoIp(LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        if (VerifyUtil.isEmpty(initNetworkCfg)) {
            log.error("network info is not complete.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "network info is not complete");
        }
        String ip = initNetworkCfg.getNetworkInfo().getIp();
        String mask = initNetworkCfg.getNetworkInfo().getMask();
        String gateway = initNetworkCfg.getNetworkInfo().getGateway();
        if (Ipv4AddressUtil.isIPv4Address(ip)) {
            logicPortDto.setIp(ip);
            logicPortDto.setIpType(IpType.IPV4.getValue());
            logicPortDto.setMask(checkAndGetIpv4Mask(mask));
            logicPortDto.setGateWay(checkAndGetIpv4Gateway(gateway));
            return;
        }
        if (Ipv6AddressUtil.isIpv6Address(ip)) {
            logicPortDto.setIp(ip);
            logicPortDto.setIpType(IpType.IPV6.getValue());
            logicPortDto.setMask(checkAndGetIpv6Mask(mask));
            logicPortDto.setGateWay(checkAndGetIpv6Gateway(gateway));
            return;
        }
        log.error("The ip address: {} is illegal.", ip);
        throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
            new String[]{"logic_port_ip_label", ip}, "Illegal ip address.");
    }

    private void fillLogicPortDtoRole(LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        if (VerifyUtil.isEmpty(initNetworkCfg.getNetworkInfo())) {
            log.error("Network info is not complete.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Network info is not complete");
        }
        NetPlaneType portRole = NetPlaneType.forValues(initNetworkCfg.getServicePortType().split(SPLIT_SEPARATOR)[0]);
        switch (portRole) {
            case BACKUP:
                logicPortDto.setRole(PortRole.SERVICE);
                break;
            case COPY:
                logicPortDto.setRole(PortRole.TRANSLATE);
                break;
            case ARCHIVE:
                logicPortDto.setRole(PortRole.ARCHIVE);
                break;
        }
        log.info("Set port role success, portRole: {}", portRole.getType());
    }

    private void fillLogicPortDtoController(LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        if (VerifyUtil.isEmpty(initNetworkCfg.getController())) {
            log.error("Network info is not complete.");
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"controller_label", StringUtils.EMPTY}, "network info is not compelete");
        }
        String controller = initNetworkCfg.getController().split(SPLIT_SEPARATOR)[0];
        if (VerifyUtil.isEmpty(controller)) {
            log.error("The controller: {} is empty.", controller);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"controller_label", controller}, "Illegal controller.");
        }
        logicPortDto.setHomeControllerId(controller);
        logicPortDto.setCurrentControllerId(controller);
    }

    private void fillLogicPortRoute(LogicPortDto logicPortDto, InitNetworkCfg initNetworkCfg) {
        if (VerifyUtil.isEmpty(initNetworkCfg.getRoute())) {
            log.info("Port route info is empty.");
            return;
        }
        List<PortRouteInfo> portRouteInfos = new ArrayList<>();
        for (PortRouteInfo portRouteInfo : initNetworkCfg.getRoute()) {
            PortRouteInfo route = new PortRouteInfo();
            BeanUtils.copyProperties(portRouteInfo, route);
            RouteType routeType = route.getRouteType();
            checkRouteInfo(logicPortDto, route);
            switch (routeType) {
                case MASTER:
                    if (IpType.IPV4.getValue().equals(logicPortDto.getIpType())) {
                        route.setMask("255.255.255.255");
                    } else {
                        route.setMask("128");
                    }
                    break;
                case NETWORK:
                    break;
                case DEFAULT:
                    if (IpType.IPV4.getValue().equals(logicPortDto.getIpType())) {
                        route.setDestination("0.0.0.0");
                        route.setMask("0.0.0.0");
                    } else {
                        route.setDestination("::");
                        route.setMask("0");
                    }
                    break;
            }
            portRouteInfos.add(route);
        }
        logicPortDto.setRoute(portRouteInfos);
    }

    private void checkRouteInfo(LogicPortDto logicPortDto, PortRouteInfo route) {
        if (IpType.IPV4.getValue().equals(logicPortDto.getIpType())) {
            if (!Ipv4AddressUtil.isIPv4Address(route.getDestination())) {
                log.error("The master route destination: {} is invalid ipv4 format.", route.getDestination());
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"route_destination_label", route.getDestination()}, "Illegal route destination.");
            }
            // 校验路由与逻辑端口在同一网段
            route.setMask(checkAndGetIpv4Mask(route.getMask()));
            route.setGateway(checkAndGetIpv4Gateway(route.getGateway()));
        }
        if (IpType.IPV6.getValue().equals(logicPortDto.getIpType())) {
            if (!Ipv6AddressUtil.isIpv6Address(route.getDestination())) {
                log.error("The master route destination: {} is invalid ipv6 format.", route.getDestination());
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"route_destination_label", route.getDestination()}, "Illegal route destinations.");
            }
            route.setMask(checkAndGetIpv6Mask(route.getMask()));
            route.setGateway(checkAndGetIpv6Gateway(route.getGateway()));
        }
    }

    private String checkAndGetIpv4Mask(String mask) {
        if (mask.contains(".")) {
            if (!Ipv4AddressUtil.isIPv4Address(mask)) {
                log.error("mask: {} is illegal.", mask);
                throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                    new String[]{"sub_net_mask_init_system_label", mask}, "Illegal mask.");
            }
            return mask;
        }
        if (StringUtils.isEmpty(mask) || !mask.matches("\\d+")) {
            log.error("mask: {} is illegal.", mask);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"sub_net_mask_init_system_label", mask}, "Illegal mask.");
        }
        int prefix = Integer.parseInt(mask);
        if (prefix < 0 || prefix > 32) {
            log.error("The mask prefix is: {}, needs to be in range [0, 32]", prefix);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"sub_net_mask_init_system_label", mask}, "Illegal mask prefix.");
        }
        return convertPrefixToIpv4(prefix);
    }

    private String checkAndGetIpv4Gateway(String gateway) {
        if (!Ipv4AddressUtil.isIPv4Address(gateway)) {
            log.error("The gateway: {} is invalid ipv4 format.", gateway);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"get_way_init_system_label", gateway}, "Illegal gateway.");
        }
        return gateway;
    }

    private String checkAndGetIpv6Mask(String mask) {
        if (StringUtils.isEmpty(mask) || !mask.matches("\\d+")) {
            log.error("mask: {} is illegal.", mask);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"sub_net_mask_init_system_label", mask}, "Illegal mask.");
        }
        int prefix = Integer.parseInt(mask);
        if (prefix < 0 || prefix > 128) {
            log.error("The mask prefix is: {}, needs to be in range [0, 128]", prefix);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"sub_net_mask_init_system_label", mask}, "Illegal mask prefix.");
        }
        return mask;
    }

    private String checkAndGetIpv6Gateway(String gateway) {
        if (!Ipv6AddressUtil.isIpv6Address(gateway)) {
            log.error("The gateway: {} is not valid ipv6 format.", gateway);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"get_way_init_system_label", gateway}, "Illegal gateway.");
        }
        return gateway;
    }

    private String convertPrefixToIpv4(Integer prefix) {
        int mask = 0xffffffff << (32 - prefix);
        return ((mask >> 24) & 0xff) + "." + ((mask >> 16) & 0xff) + "." + ((mask >> 8) & 0xff) + "." + (mask & 0xff);
    }

    private void checkNetworkConfig(List<LogicPortDto> logicPortDtos) {
        if (CollectionUtils.isEmpty(logicPortDtos)) {
            return;
        }
        List<String> controllers = new ArrayList<>();
        controllers.add(CONTRONLLER_0A);
        controllers.add(CONTRONLLER_0B);
        if (DeployTypeEnum.X9000.equals(deployTypeService.getDeployType())) {
            controllers.add(CONTRONLLER_0C);
            controllers.add(CONTRONLLER_0D);
        }
        controllers.forEach(controller -> checkLogicPortNums(controller, logicPortDtos));
    }

    private void checkLogicPortNums(String controller, List<LogicPortDto> logicPortDtos) {
        List<LogicPortDto> logicPorts = logicPortDtos.stream()
            .filter(logicPortDto -> StringUtils.equals(controller, logicPortDto.getCurrentControllerId()))
            .collect(Collectors.toList());
        int logicPortsNum = logicPorts.size();
        if (logicPortsNum < 1) {
            log.error("Logic ports num: {} of controller: {} is illegal.", logicPortsNum, controller);
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{"controller_label", controller},
                ("Logic ports num " + logicPortsNum + " is illegal."));
        }
        PortRole portRole = logicPortDtos.get(0).getRole();
        String portTypeLabel = null;
        int logicPortLimit;
        switch (portRole) {
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
                log.error("The logic port role: {} is illegal.", portRole);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Illegal port role.");
        }
        if (logicPortsNum > logicPortLimit) {
            log.error("The service: {} port num: {} exceed limit: {}.", portRole, logicPortsNum, logicPortLimit);
            throw new LegoCheckedException(CommonErrorCode.SERVICE_PORT_NUM_EXCEED_LIMIT,
                new String[] {portTypeLabel, String.valueOf(logicPortLimit)},
                ("logical port role" + portRole.getRole() + "is illegal."));
        }
    }
}