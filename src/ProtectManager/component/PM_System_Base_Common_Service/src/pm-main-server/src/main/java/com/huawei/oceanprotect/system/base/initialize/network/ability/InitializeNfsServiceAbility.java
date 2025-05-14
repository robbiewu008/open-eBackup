/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.ability;

import com.huawei.oceanprotect.system.base.initialize.network.InitializeNfsService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.NfsServiceRest;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;

/**
 * 添加DM端口
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-27
 */
@Component
public class InitializeNfsServiceAbility implements InitializeNfsService {
    private static final String V_STORE_ID = "vstoreId";

    private static final String SUPPORT_V41 = "SUPPORTV41";

    /**
     * 更行NFSService4.1的信息
     *
     * @param service dm 对象
     */
    @Override
    public void modifyNfsService(DeviceManagerService service) {
        NfsServiceRest apiRest = service.getApiRest(NfsServiceRest.class);
        Map<String, Object> modifyNfsServiceRequest = new HashMap<>();
        modifyNfsServiceRequest.put(V_STORE_ID, "0");
        modifyNfsServiceRequest.put(SUPPORT_V41, true);
        apiRest.modifyNfsService(service.getDeviceId(), modifyNfsServiceRequest);
    }
}
