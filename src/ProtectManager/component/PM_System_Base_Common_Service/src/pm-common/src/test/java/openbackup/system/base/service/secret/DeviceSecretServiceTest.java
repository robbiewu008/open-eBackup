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
package openbackup.system.base.service.secret;

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.InfrastructureResponse;
import openbackup.system.base.service.secret.DeviceSecretService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;

@RunWith(PowerMockRunner.class)
@ContextConfiguration(classes = {DeviceSecretService.class})
@PowerMockRunnerDelegate(SpringRunner.class)
public class DeviceSecretServiceTest {
    @Autowired
    private DeviceSecretService deviceSecretService;

    @MockBean
    private InfrastructureRestApi infrastructureRestApi;
    @Before
    public void before() {
        PowerMockito.when(infrastructureRestApi.updateSecret(any())).thenReturn(new InfraResponseWithError<>());
        PowerMockito.when(infrastructureRestApi.createSecret(any(), any(), any(), any()))
                .thenReturn(new InfraResponseWithError<>());
        InfraResponseWithError<String> deleteRes = new InfraResponseWithError<>();
        deleteRes.setData("success");
        PowerMockito.when(infrastructureRestApi.deleteSecret(any(), any(), any()))
                .thenReturn(deleteRes);
        InfraResponseWithError<JSONArray> response = new InfraResponseWithError<>();
        response.setData(JSONArray.fromObject("[{\"2102354JDK10MA000003\":[{\"ip\":\"8.40.102.103\",\"port\":8088,\"username\":\"admin\",\"password\":\"Admin@storage1\",\"device_id\":\"2102354JDK10MA000003\",\"device_type\":\"op\",\"device_version\":\"\"},{\"ip\":\"8.40.102.103\",\"port\":8088,\"username\":\"dataprotect_admin\",\"password\":\"Admin@storage1\",\"device_id\":\"2102354JDK10MA000003\",\"device_type\":\"op\",\"device_version\":\"\"}]},\n" +
                "{\"2102354JDK10MA000002\":[{\"ip\":\"8.40.102.103\",\"port\":8088,\"username\":\"admin\",\"password\":\"Admin@storage1\",\"device_id\":\"2102354JDK10MA000002\",\"device_type\":\"op\",\"device_version\":\"\"},{\"ip\":\"8.40.102.103\",\"port\":8088,\"username\":\"dataprotect_admin\",\"password\":\"Admin@storage1\",\"device_id\":\"2102354JDK10MA000002\",\"device_type\":\"op\",\"device_version\":\"\"}]}]"));
        PowerMockito.when(infrastructureRestApi.getSecret(Mockito.anyString(), Mockito.anyString()))
                .thenReturn(response);
    }

    @Test
    public void create_secret_success() {
        DeviceUser deviceUser1 = new DeviceUser();
        deviceUser1.setId("123");
        deviceUser1.setUsername("name");
        deviceSecretService.createSecret(deviceUser1);
        DeviceUser deviceUser2 = new DeviceUser();
        deviceUser2.setId("deviceId001");
        deviceUser2.setUsername("admin");
        deviceSecretService.createSecret(deviceUser2);
    }

    @Test
    public void delete_secret_success() {
        InfrastructureResponse response = new InfrastructureResponse();
        response.setSuccess(true);
        PowerMockito.when(infrastructureRestApi.updateSecret(any())).thenReturn(new InfraResponseWithError<>());
        Assert.assertTrue(deviceSecretService.deleteSecretKey("deviceId001"));
    }

    @Test
    public void delete_secret_key_success() {
        InfrastructureResponse response = new InfrastructureResponse();
        response.setSuccess(true);
        deviceSecretService.deleteSecretKey("deviceId001");
    }

    @Test
    public void query_secret() {
        List<DeviceUser> deviceUserList = deviceSecretService.querySecret("2102354JDK10MA000003");
        Assert.assertEquals("admin", deviceUserList.get(0).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(0).getPassword());
        Assert.assertEquals("dataprotect_admin", deviceUserList.get(1).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(1).getPassword());
    }

    @Test
    public void query_all_secret() {
        List<DeviceUser> deviceUserList = deviceSecretService.queryAllSecret();
        Assert.assertEquals("admin", deviceUserList.get(0).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(0).getPassword());
        Assert.assertEquals("dataprotect_admin", deviceUserList.get(1).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(1).getPassword());
        Assert.assertEquals("admin", deviceUserList.get(2).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(2).getPassword());
        Assert.assertEquals("dataprotect_admin", deviceUserList.get(3).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(3).getPassword());
    }
}
