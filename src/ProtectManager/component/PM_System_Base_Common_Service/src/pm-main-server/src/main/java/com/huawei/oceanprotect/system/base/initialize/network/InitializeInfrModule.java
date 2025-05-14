/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.inftmodule.InftModuleObject;

import java.util.List;
import java.util.Map;

/**
 * 获取接口模块信息列表
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
public interface InitializeInfrModule {
    /**
     * 批量查询接口模块信息
     *
     * @param service dm 对象
     * @param filter 过滤条件
     * @return 接口模块信息列表
     */
    List<InftModuleObject> getInfrModule(DeviceManagerService service, Map<String, String> filter);
}
