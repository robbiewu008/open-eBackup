/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import com.huawei.oceanprotect.system.base.initialize.bean.DeviceManagerServiceMockBean;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeInfrModuleAbility;
import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeUserServiceAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.IntfModuleRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfoEncoder;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.inftmodule.InftModuleObject;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HealthStatus;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 测试DM 用户能力
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-26
 */
public class InitializeInfrModuleTest {
    private final DeviceManagerService service = Mockito.mock(DeviceManagerService.class);

    private final IntfModuleRest intfModuleRest = Mockito.mock(IntfModuleRest.class);

    private final DeviceManagerServiceMockBean deviceManagerServiceMockBean = new DeviceManagerServiceMockBean();

    private final InitializeInfrModuleAbility initializeInfrModuleAbility = new InitializeInfrModuleAbility();

    @Before
    public void init() {
        given(service.getApiRest(IntfModuleRest.class)).willReturn(intfModuleRest);
        given(service.getDeviceId()).willReturn("getDeviceId");
    }

    /**
     * 用例场景：获取DM用户成功
     * 前置条件：NA
     * 检查点：存在用户
     */
    @Test
    public void get_intfr_module_success() {
        List<InftModuleObject> list = new ArrayList<>();
        InftModuleObject inftModuleObject = new InftModuleObject();
        inftModuleObject.setHealthStatus(HealthStatus.NORMAL);
        list.add(inftModuleObject);
        given(intfModuleRest.getIntfrModule(any())).willReturn(list);
        Assert.assertEquals(1, initializeInfrModuleAbility.getInfrModule(service, new HashMap<>()).size());
    }

    /**
     * 用例场景：创建用户和设置密码永不过期
     * 前置条件：NA
     * 检查点：设置成功
     */
    @Test
    public void exist_filter_get_intfr_module_success() {
        List<InftModuleObject> list = new ArrayList<>();
        InftModuleObject inftModuleObject = new InftModuleObject();
        inftModuleObject.setHealthStatus(HealthStatus.NORMAL);
        inftModuleObject.setServiceMode(1);
        list.add(inftModuleObject);
        given(intfModuleRest.getIntfrModule(any())).willReturn(list);
        Map<String, String> filter = new HashMap<>();
        filter.put(InitConfigConstant.SERVICE_MODE, "1");
        Assert.assertEquals(1, initializeInfrModuleAbility.getInfrModule(service, filter).size());
    }
}
