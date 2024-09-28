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
package openbackup.exchange.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * ExchangeMailboxBackupInterceptor Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ExchangeMailboxBackupInterceptor.class, ExchangeService.class})
public class ExchangeMailboxBackupInterceptorTest {
    private ExchangeMailboxBackupInterceptor interceptor;

    @Mock
    private ExchangeService exchangeService;

    @Before
    public void setUp() {
        interceptor = new ExchangeMailboxBackupInterceptor(exchangeService);
    }

    /**
     * 用例场景：测试设置agent
     * 前置条件：无
     * 检查点：agent设置成功
     */
    @Test
    public void testSupplyAgent() {
        // 设置dependencies
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("7017bd24-1a4d-42fc-aaf4-3046eab88704");
        resource.setPort(5555);
        resource.setEndpoint("1.1.1.1");
        list.add(resource);
        dependencies.put("agents", list);

        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.8.8.8");
        environment.setPort(66);
        environment.setDependencies(dependencies);
        when(exchangeService.getEnvironmentById(anyString())).thenReturn(environment);
        BackupTask backupTask = getBackupTask();
        interceptor.supplyAgent(backupTask);
        Assert.assertEquals(resource.getUuid(), backupTask.getAgents().get(0).getId());
    }

    /**
     * 用例场景：测试可用资源类型
     * 前置条件：无
     * 检查点：检测成功
     */
    @Test
    public void testApplicable() {
        Assert.assertTrue(interceptor.applicable("Exchange-mailbox"));
        Assert.assertFalse(interceptor.applicable("Exchange-database"));
    }

    /**
     * 用例场景：测试设置任务属性
     * 前置条件：无
     * 检查点：设置成功
     */
    @Test
    public void testSupplyBackupTask() {
        BackupTask backupTask = getBackupTask();
        interceptor.supplyBackupTask(backupTask);
        Assert.assertEquals(backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SHARDING.getType());
        Assert.assertEquals(1, backupTask.getCopyFormat());
    }

    private BackupTask getBackupTask() {
        String json =
            "{\"requestId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"taskId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"copyId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"backupType\":\"fullBackup\",\"protectObject\":{\"uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"name\":\"database01\",\"type\":\"Database\",\"subType\":\"Exchange-database\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"server_name\":\"WIN-VUB15GTQID6\",\"db_name\":\"database01\",\"log_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\",\"next_backup_type\":\"full\",\"db_uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"state\":\"Mounted\",\"version\":\"15.1.2507.6\",\"edb_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\\\\database01.edb\",\"next_backup_change_cause\":\"bigdata_plugin_to_full_label\"},\"parentUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"parentName\":\"danji\"},\"protectEnv\":{\"uuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"name\":\"danji\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"isGroup\":\"0\",\"agentUuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"endpoint\":\"192.168.160.210\",\"port\":0},\"agents\":[{\"id\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"ip\":\"192.168.120.113\",\"port\":59530,\"agentOS\":\"linux\"},{\"id\":\"6f378a00-e81b-4307-8caf-15889a8ab9ca\",\"ip\":\"192.168.99.232\",\"port\":59533,\"agentOS\":\"windows\"},{\"id\":\"dbfd034e-c24f-493e-99ee-126766a3a9d9\",\"ip\":\"192.168.99.234\",\"port\":59534,\"agentOS\":\"windows\"},{\"id\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\",\"ip\":\"192.168.160.210\",\"port\":59530,\"agentOS\":\"windows\"},{\"id\":\"f61d9520-9f34-481d-ad8d-dac227e3eeed\",\"ip\":\"192.168.99.242\",\"port\":59529,\"agentOS\":\"linux\"},{\"id\":\"5741dd57-dff6-42e2-a614-12fe60a7a8c7\",\"ip\":\"192.168.99.240\",\"port\":59529,\"agentOS\":\"linux\"}],\"dataLayout\":{\"srcEncryption\":false,\"dstDeduption\":true,\"srcDeduption\":false,\"linkEncryption\":false,\"srcCompression\":false,\"dstEncryption\":false,\"dstCompression\":true,\"clientProtocolType\":0,\"characterSet\":0},\"scripts\":{},\"advanceParams\":{\"next_cause_param\":\"0\"},\"copyFormat\":0,\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"extendInfo\":{\"esn\":\"2102354DEY10M3000002\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"zI)9DsL#D44c\"},\"endpoint\":{\"ip\":\"8.40.99.244,8.40.99.245\",\"port\":8088},\"local\":true,\"isLocal\":true}]}\n";
        BackupTask read = JsonUtil.read(json, BackupTask.class);
        return read;
    }
}
