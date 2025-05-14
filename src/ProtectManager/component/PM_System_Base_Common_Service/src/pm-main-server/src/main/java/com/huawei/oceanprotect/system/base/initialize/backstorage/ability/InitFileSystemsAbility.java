/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage.ability;

import com.huawei.oceanprotect.system.base.initialize.backstorage.InitializeFileSystems;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.filesystem.ApplicationWorkLoad;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.List;

/**
 * 初始化文件系统能力
 *
 * @author w00493811
 * @since 2020-12-21
 */
@Slf4j
@Service
public class InitFileSystemsAbility implements InitializeFileSystems {
    private static final String DPA_FILE_SYSTEM_APP_TYPE = "NAS_DPA_LARGE_FS";

    /**
     * 创建文件系统类型
     *
     * @param service 本地存储Service
     * @return 文件系统类型ID
     */
    @Override
    public long attainSetWorkloadTypeId(DeviceManagerService service) {
        DeviceManagerResponse<List<ApplicationWorkLoad>> fileSystemDeviceManagerResponse
            = service.getBatchWorkLoadType();
        log.info("begin to query special application id");
        for (ApplicationWorkLoad workLoad : fileSystemDeviceManagerResponse.getData()) {
            if (workLoad.getName().equals(DPA_FILE_SYSTEM_APP_TYPE)) {
                log.info("dpa file system app type: {}, id: {}", DPA_FILE_SYSTEM_APP_TYPE, workLoad.getId());
                return Long.parseLong(workLoad.getId());
            }
        }
        ApplicationWorkLoad applicationWorkLoad = new ApplicationWorkLoad();
        applicationWorkLoad.setName(DPA_FILE_SYSTEM_APP_TYPE);
        applicationWorkLoad.setBlockSize("3"); // 3表示应用请求大小是32kb
        applicationWorkLoad.setTemplateType("1"); // 1表示文件系统
        applicationWorkLoad.setEnableCompress(true);
        applicationWorkLoad.setEnableDedup(true);
        DeviceManagerResponse<ApplicationWorkLoad> fileSystemDeviceManagerAddResponse =
            service.addWorkLoadType(applicationWorkLoad);
        String id = fileSystemDeviceManagerAddResponse.getData().getId();
        log.info("dpa file system app type: {}, id: {}", DPA_FILE_SYSTEM_APP_TYPE, id);
        return Long.parseLong(id);
    }
}
