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
package com.huawei.oceanprotect.system.base.service.impl.strategy.deploy;

import com.huawei.oceanprotect.system.base.enums.InitEnum;
import com.huawei.oceanprotect.system.base.initialize.network.common.ArchiveNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.BackupNetworkConfig;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.util.NetworkConfigUtils;
import com.huawei.oceanprotect.system.base.service.InitConfigService;
import com.huawei.oceanprotect.system.base.service.impl.pacific.PacificInitNetworkServiceImpl;
import com.huawei.oceanprotect.system.base.service.strategy.deploy.InitDeployTypeStrategyService;

import io.jsonwebtoken.lang.Strings;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LLDConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.devicemanager.request.IpInfo;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * E6000InitDeployTypeStrategyServiceImpl
 *
 */
@Component
@Slf4j
public class E6000InitDeployTypeStrategyServiceImpl implements InitDeployTypeStrategyService {
    @Autowired
    private InitConfigService initConfigService;

    @Autowired
    private PacificInitNetworkServiceImpl pacificInitNetworkService;

    /**
     * 根据lld获取初始化网络配置信息
     *
     * @param lld lld
     * @return 网络配置信息
     */
    @Override
    public InitNetworkBody getInitNetworkBodyByLLD(MultipartFile lld) {
        log.info("Start get init xls");
        try (InputStream inputStream = lld.getInputStream()) {
            if (!Objects.requireNonNull(lld.getOriginalFilename()).endsWith(LLDConstants.LLD_SUFFIX)) {
                log.error("Lld file type do not .xls");
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Lld file type do not .xls");
            }
            return getInitXls(new HSSFWorkbook(inputStream));
        } catch (IOException e) {
            log.error("Get lld file failed.", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "get lld file failed");
        }
    }

    private InitNetworkBody getInitXls(HSSFWorkbook workBook) {
        HSSFSheet sheet = workBook.getSheetAt(LLDConstants.E6000LLDConstants.SHEET_INDEX);
        if (VerifyUtil.isEmpty(sheet)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Target sheet not existed");
        }
        Map<String, List<IpInfo>> backupNetworkConfigMap = new HashMap<>();
        int curRowIndex = LLDConstants.E6000LLDConstants.CONFIG_STRAT_ROW_INDEX;
        String backupNetworkTypeParam = sheet.getRow(curRowIndex)
            .getCell(LLDConstants.E6000LLDConstants.NETWORK_TYPE_CELL_INDEX)
            .toString();
        if (!InitEnum.E6000NetworkTypeEnum.BACKUP.getValue().equals(backupNetworkTypeParam)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {LLDConstants.E6000LLDConstants.E6000_NETWORK_TYPE_INIT_BY_LLD_SYSTEM_LABEL,
                    backupNetworkTypeParam}, "Wrong network type.");
        }
        while (true) {
            HSSFRow curRow = sheet.getRow(curRowIndex);
            if (isNetworkConfigEnd(curRow, InitEnum.E6000NetworkTypeEnum.BACKUP)) {
                break;
            }
            // 每一行对应一条业务网络
            buildNetworkConfigMap(curRow, backupNetworkConfigMap);
            curRowIndex += 1;
        }
        Map<String, List<IpInfo>> archiveNetworkConfigMap = new HashMap<>();
        String archiveNetworkTypeParam = sheet.getRow(curRowIndex)
            .getCell(LLDConstants.E6000LLDConstants.NETWORK_TYPE_CELL_INDEX)
            .toString();
        if (!InitEnum.E6000NetworkTypeEnum.ARCHIVE.getValue().equals(archiveNetworkTypeParam)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[] {LLDConstants.E6000LLDConstants.E6000_NETWORK_TYPE_INIT_BY_LLD_SYSTEM_LABEL,
                    archiveNetworkTypeParam}, "Wrong network type.");
        }
        while (true) {
            HSSFRow curRow = sheet.getRow(curRowIndex);
            if (isNetworkConfigEnd(curRow, InitEnum.E6000NetworkTypeEnum.ARCHIVE)) {
                break;
            }
            // 每一行对应一条业务网络
            buildNetworkConfigMap(curRow, archiveNetworkConfigMap);
            curRowIndex += 1;
        }
        InitNetworkBody initNetworkBody = buildInitNetworkBody(backupNetworkConfigMap, archiveNetworkConfigMap);

        StorageAuth storageAuth = initConfigService.getLocalStorageAuth();
        String deviceId = initConfigService.getLocalStorageDeviceId();
        try {
            pacificInitNetworkService.unifiedCheck(deviceId, storageAuth.getUsername(), initNetworkBody, true);
            return initNetworkBody;
        } finally {
            StringUtil.clean(storageAuth.getPassword());
        }
    }

    private boolean isNetworkConfigEnd(HSSFRow curRow, InitEnum.E6000NetworkTypeEnum networkTypeEnum) {
        // 空行直接结束
        if (ObjectUtils.isEmpty(curRow)) {
            return true;
        }
        // 网络类型既不是networkTypeEnum指定的网络，也不是空，直接返回
        String networkType = curRow.getCell(LLDConstants.E6000LLDConstants.NETWORK_TYPE_CELL_INDEX).toString();
        return !networkTypeEnum.getValue().equals(networkType) && !Strings.EMPTY.equals(networkType);
    }

    private void buildNetworkConfigMap(HSSFRow curRow, Map<String, List<IpInfo>> networkConfigMap) {
        String manageIp = curRow.getCell(LLDConstants.E6000LLDConstants.MANAGE_IP_CELL_INDEX).toString();
        String ifaceName = curRow.getCell(LLDConstants.E6000LLDConstants.PORT_CELL_INDEX).toString();
        String ipAddress = curRow.getCell(LLDConstants.E6000LLDConstants.BUSINESS_IP_CELL_INDEX).toString();
        if (StringUtils.isAllEmpty(ifaceName, ipAddress)) {
            return;
        }
        if (StringUtils.isEmpty(manageIp)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{LLDConstants.E6000LLDConstants.E6000_MANAGER_IP_INIT_BY_LLD_SYSTEM_LABEL, Strings.EMPTY},
                "manager ip is illegal");
        }
        if (StringUtils.isEmpty(ifaceName)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{LLDConstants.E6000LLDConstants.E6000_IFACENAME_INIT_BY_LLD_SYSTEM_LABEL, Strings.EMPTY},
                "ifaceName is illegal");
        }
        if (StringUtils.isEmpty(ipAddress)) {
            throw new LegoCheckedException(CommonErrorCode.LLD_FILE_PARSED_ERROR,
                new String[]{LLDConstants.E6000LLDConstants.E6000_IP_ADDRESS_INIT_BY_LLD_SYSTEM_LABEL, Strings.EMPTY},
                "ip address is illegal");
        }
        IpInfo ipInfo = new IpInfo();
        ipInfo.setIpAddress(ipAddress);
        ipInfo.setIfaceName(ifaceName);
        List<IpInfo> ipInfoList = networkConfigMap.getOrDefault(manageIp, new ArrayList<>());
        ipInfoList.add(ipInfo);
        networkConfigMap.put(manageIp, ipInfoList);
    }

    // 创建InitNetworkBody
    private InitNetworkBody buildInitNetworkBody(Map<String, List<IpInfo>> backupNetworkConfigMap,
        Map<String, List<IpInfo>> archiveNetworkConfigMap) {
        List<NodeNetworkInfoRequest> backupNetWorkInfoList = backupNetworkConfigMap.entrySet().stream().map(entry -> {
            NodeNetworkInfoRequest nodeNetworkInfoRequest = new NodeNetworkInfoRequest();
            nodeNetworkInfoRequest.setManageIp(entry.getKey());
            nodeNetworkInfoRequest.setIpInfoList(entry.getValue());
            return nodeNetworkInfoRequest;
        }).collect(Collectors.toList());

        List<NodeNetworkInfoRequest> archiveNetWorkInfoList = archiveNetworkConfigMap.entrySet().stream().map(entry -> {
            NodeNetworkInfoRequest nodeNetworkInfoRequest = new NodeNetworkInfoRequest();
            nodeNetworkInfoRequest.setManageIp(entry.getKey());
            nodeNetworkInfoRequest.setIpInfoList(entry.getValue());
            return nodeNetworkInfoRequest;
        }).collect(Collectors.toList());

        BackupNetworkConfig backupNetworkConfig = new BackupNetworkConfig();
        backupNetworkConfig.setPacificInitNetWorkInfoList(backupNetWorkInfoList);

        ArchiveNetworkConfig archiveNetworkConfig = new ArchiveNetworkConfig();
        archiveNetworkConfig.setPacificInitNetWorkInfoList(archiveNetWorkInfoList);

        InitNetworkBody initNetworkBody = new InitNetworkBody();
        initNetworkBody.setBackupNetworkConfig(backupNetworkConfig);
        initNetworkBody.setArchiveNetworkConfig(archiveNetworkConfig);
        return initNetworkBody;
    }

    private void checkSheetName(String sheetName, String sheetValue) {
        if (!StringUtils.equals(sheetName, sheetValue)) {
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
}
