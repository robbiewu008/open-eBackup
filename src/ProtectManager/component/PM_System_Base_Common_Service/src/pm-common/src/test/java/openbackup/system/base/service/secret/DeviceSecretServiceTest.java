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

import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;

import openbackup.system.base.bean.DeviceUser;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.InfrastructureResponse;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.List;
import java.util.Optional;

@RunWith(PowerMockRunner.class)
public class DeviceSecretServiceTest {
    @InjectMocks
    private DeviceSecretService deviceSecretService;

    @Mock
    private InfrastructureRestApi infrastructureRestApi;

    @Mock
    private LockService lockService;

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

    /**
     * 用例场景：upsertSecret
     * 前置条件：正常运行
     * 检查点：操作成功
     */
    @Test
    public void test_upsertSecret_success() {
        Lock distributeLock = PowerMockito.mock(Lock.class);
        PowerMockito.when(lockService.createDistributeLock(any())).thenReturn(distributeLock);
        PowerMockito.when(distributeLock.tryLock(anyLong(), any())).thenReturn(true);

        DeviceUser deviceUser1 = new DeviceUser();
        deviceUser1.setId("123");
        deviceUser1.setUsername("name");
        deviceSecretService.upsertSecret("123", Collections.singletonList(deviceUser1));
        DeviceUser deviceUser2 = new DeviceUser();
        deviceUser2.setId("deviceId001");
        deviceUser2.setUsername("admin");
        deviceSecretService.upsertSecret("deviceId001", Collections.singletonList(deviceUser2));
    }

    @Test
    public void delete_secret_success() {
        InfrastructureResponse response = new InfrastructureResponse();
        response.setSuccess(true);
        PowerMockito.when(infrastructureRestApi.updateSecret(any())).thenReturn(new InfraResponseWithError<>());
        Assert.assertTrue(deviceSecretService.deleteSecretKey("deviceId001"));
    }

    /**
     * 用例场景：根据设备id和用户名删除secret信息
     * 前置条件：正常运行
     * 检查点：删除成功
     */
    @Test
    public void delete_secret_when_device_id_username_then_success() {
        InfrastructureResponse response = new InfrastructureResponse();
        response.setSuccess(true);
        PowerMockito.when(infrastructureRestApi.updateSecret(any())).thenReturn(new InfraResponseWithError<>());
        Assert.assertTrue(deviceSecretService.deleteSecret("deviceId001", "admin"));
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

    /**
     * 用例场景：根据设备id和用户名查询secret信息
     * 前置条件：正常运行
     * 检查点：查询成功
     */
    @Test
    public void query_secret_when_device_id_username_then_success() {
        List<DeviceUser> deviceUserList = deviceSecretService.querySecret("2102354JDK10MA000003",
            "admin");
        Assert.assertEquals("admin", deviceUserList.get(0).getUsername());
        Assert.assertEquals("Admin@storage1", deviceUserList.get(0).getPassword());
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

    /**
     * 用例场景：根据认证状态修改sec状态
     * 前置条件：用户不存在
     * 检查点：运行正常
     */
    @Test
    public void testChangeUseful_UserFound() throws Exception {
        // Given
        String deviceId = "device123";
        String username = "user123";
        boolean isAuthValid = true;
        DeviceUser user = new DeviceUser();
        user.setUsername(username);
        // When
        deviceSecretService.changeUseful(deviceId, username, isAuthValid);

        assertTrue(user.isUseful());
    }

    /**
     * 用例场景：根据认证状态修改sec状态
     * 前置条件：认证异常
     * 检查点：修改成功
     */
    @Test
    public void test_changeUseful_should_void_when_condition() throws Exception {
        String deviceId = "2102354JDK10MA000003";
        String userName = "admin";
        // run the test
        deviceSecretService.changeUseful(deviceId, "admin", false);
        // verify the results
        List<DeviceUser> deviceUsers = deviceSecretService.querySecret(deviceId);
        Optional optional = deviceUsers.stream().filter(deviceUser -> deviceUser.getUsername().equals(userName))
            .filter(deviceUser -> deviceUser.isUseful()).findFirst();
        assertTrue(optional.isPresent());
    }
}
