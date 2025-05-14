/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeFileSystems;
import com.huawei.oceanprotect.system.base.initialize.network.action.DeviceManagerHandler;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.process.ProcessException;
import openbackup.system.base.common.process.ProcessResult;
import openbackup.system.base.common.process.ProcessUtil;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.network.Ipv4AddressUtil;
import openbackup.system.base.common.utils.network.Ipv6AddressUtil;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 内部接口，用于服务初始化及进度更新
 *
 * @author xwx1016404
 * @since 2021-10-07
 */
@Slf4j
@RestController
@RequestMapping("/v1/internal/service")
public class InitServiceController {
    // 查找浮动ip的命令
    private static final List<String> FIND_FLOAT_IP_COMMAND_ARR =
        Arrays.asList("/bin/sh", "-c",
            "cat /opt/network/network_config.ini | grep float_ip | awk -F '=' '{print $2}'");

    // 查找浮动ip的命令
    private static final List<String> FIND_MANAGER_IP_COMMAND_ARR =
        Arrays.asList("/bin/sh", "-c", "netstat -nlp | grep 25080  | awk '{print $4}' | cut -d: -f1");

    // 查找浮动ip的命令，1分钟超时时间
    private static final long TIMEOUT_IN_MINUTES = 1L;

    @Autowired
    private IStorageDeviceRepository repository;

    @Autowired
    private DeviceManagerHandler handler;

    @Autowired
    private InitializeFileSystems fileSystems;

    /**
     * 创建文件系统类型
     *
     * @return 新建文件系统类型的ID
     */
    @ExterAttack
    @GetMapping("/filesystem/workload/id")
    public long creatFileSystemType() {
        long typeId;
        StorageDevice storageDevice = null;
        try {
            storageDevice = repository.findLocalStorage(true);
            StorageAuth storageAuth = new StorageAuth();
            storageAuth.setUsername(storageDevice.getUserName());
            storageAuth.setPassword(storageDevice.getPassword());
            DeviceManagerService deviceManagerService = handler.achiveDeviceManagerService(storageAuth);
            typeId = fileSystems.attainSetWorkloadTypeId(deviceManagerService);
        } catch (DeviceManagerException | LegoCheckedException exception) {
            log.error("Create dpa fileSystem type error.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "create dpa fileSystem type error.");
        } finally {
            if (storageDevice != null) {
                StringUtil.clean(storageDevice.getPassword());
            }
        }
        return typeId;
    }

    /**
     * 获取E1000的管理ip
     *
     * @return 管理ip
     */
    @ExterAttack
    @GetMapping("/manager-ip")
    public String getManagerIp() {
        try {
            ProcessResult processResult = ProcessUtil.executeInMinutes(FIND_MANAGER_IP_COMMAND_ARR, TIMEOUT_IN_MINUTES);
            if (processResult.isNok()) {
                log.error("Get manager ip failed.");
                return Strings.EMPTY;
            }
            List<String> outputList = processResult.getOutputList();
            if (outputList.size() == LegoNumberConstant.ZERO) {
                log.error("Get manager ip failed.");
                return Strings.EMPTY;
            }
            String managerIp = outputList.get(LegoNumberConstant.ZERO);
            log.info("Get manager ip success, {}.", managerIp);
            return managerIp;
        } catch (ProcessException e) {
            log.error("Get manager ip failed.", ExceptionUtil.getErrorMessage(e));
            return Strings.EMPTY;
        }
    }

    /**
     * 获取pacific的浮动ip
     *
     * @return 浮动ip
     */
    @ExterAttack
    @GetMapping("/float-ip")
    public String findFloatIp() {
        try {
            ProcessResult processResult = ProcessUtil.executeInMinutes(FIND_FLOAT_IP_COMMAND_ARR, TIMEOUT_IN_MINUTES);
            if (processResult.isNok()) {
                log.error("Find pacific float ip failed.");
                return Strings.EMPTY;
            }
            List<String> outputList = processResult.getOutputList();
            if (outputList.size() == LegoNumberConstant.ZERO) {
                log.error("Find pacific float ip failed.");
                return Strings.EMPTY;
            }
            List<String> floatIpList = outputList.stream()
                .filter(floatIp -> checkFloatIp(floatIp))
                .collect(Collectors.toList());
            if (VerifyUtil.isEmpty(floatIpList)) {
                return Strings.EMPTY;
            }
            String floatIp = floatIpList.get(LegoNumberConstant.ZERO);
            log.info("Find pacific float ip success, {}.", floatIp);
            return floatIp;
        } catch (ProcessException e) {
            log.error("Find float ip failed.", ExceptionUtil.getErrorMessage(e));
            return Strings.EMPTY;
        }
    }

    private boolean checkFloatIp(String floatIp) {
        if (!Ipv4AddressUtil.isIPv4Address(floatIp) && Ipv6AddressUtil.isIpv6Address(floatIp)) {
            log.error("Check pacific float ip failed:{}.", floatIp);
            return false;
        }
        log.info("Check float ip success:{}.", floatIp);
        return true;
    }
}
