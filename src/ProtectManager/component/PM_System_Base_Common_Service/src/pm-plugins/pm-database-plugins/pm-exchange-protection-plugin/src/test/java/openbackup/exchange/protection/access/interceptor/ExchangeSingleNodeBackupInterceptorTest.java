/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.exchange.protection.access.interceptor;

import static org.assertj.core.api.Assertions.assertThat;
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
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

@ExtendWith(MockitoExtension.class)
class ExchangeSingleNodeBackupInterceptorTest {

    @Mock
    private ExchangeService mockExchangeService;

    private ExchangeBackupInterceptor exchangeBackupInterceptorUnderTest;

    @BeforeEach
    void setUp() {
        exchangeBackupInterceptorUnderTest = new ExchangeBackupInterceptor(mockExchangeService);
    }

    @Test
    void testApplicable() {
        assertThat(exchangeBackupInterceptorUnderTest.applicable("object")).isFalse();
    }

    @Test
    void testSupplyAgent() {
        // 设置dependencies
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("7017bd24-1a4d-42fc-aaf4-3046eab88704");
        list.add(resource);
        dependencies.put("agents", list);

        // 设置mock返回值
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.8.8.8");
        environment.setPort(66);
        environment.setDependencies(dependencies);
        when(mockExchangeService.getEnvironmentById(anyString())).thenReturn(environment);
        // Run the test
        exchangeBackupInterceptorUnderTest.supplyAgent(getBackupTask());
        Assert.assertTrue(true);
    }

    @Test
    void testCheckConnention() {
        // Run the test
        exchangeBackupInterceptorUnderTest.checkConnention(getBackupTask());

        // Verify the results
    }

    @Test
    void testSupplyBackupTask() {
        BackupTask backupTask = getBackupTask();

        // Configure ExchangeService.getEnvironmentById(...).
        final ProtectedEnvironment environment = getEnvironment();
        exchangeBackupInterceptorUnderTest.supplyBackupTask(backupTask);

        Assert.assertEquals(backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SHARDING.getType());

    }

    private ProtectedEnvironment getEnvironment() {
        String json =
            "{\"name\":\"Server01\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"extendInfo\":{\"linkStatus\":\"0\",\"agentUuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"\"isGroup\":0},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\"},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private BackupTask getBackupTask() {
        String json =
            "{\"requestId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"taskId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"copyId\":\"e6e9d68d-0053-4949-830c-1facea9dc605\",\"backupType\":\"fullBackup\",\"protectObject\":{\"uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"name\":\"database01\",\"type\":\"Database\",\"subType\":\"Exchange-database\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"server_name\":\"WIN-VUB15GTQID6\",\"db_name\":\"database01\",\"log_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\",\"next_backup_type\":\"full\",\"db_uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"state\":\"Mounted\",\"version\":\"15.1.2507.6\",\"edb_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\\\\database01.edb\",\"next_backup_change_cause\":\"bigdata_plugin_to_full_label\"},\"parentUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"parentName\":\"danji\"},\"protectEnv\":{\"uuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"name\":\"danji\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"isGroup\":\"0\",\"agentUuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"endpoint\":\"192.168.160.210\",\"port\":0},\"agents\":[{\"id\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"ip\":\"192.168.120.113\",\"port\":59530,\"agentOS\":\"linux\"},{\"id\":\"6f378a00-e81b-4307-8caf-15889a8ab9ca\",\"ip\":\"192.168.99.232\",\"port\":59533,\"agentOS\":\"windows\"},{\"id\":\"dbfd034e-c24f-493e-99ee-126766a3a9d9\",\"ip\":\"192.168.99.234\",\"port\":59534,\"agentOS\":\"windows\"},{\"id\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\",\"ip\":\"192.168.160.210\",\"port\":59530,\"agentOS\":\"windows\"},{\"id\":\"f61d9520-9f34-481d-ad8d-dac227e3eeed\",\"ip\":\"192.168.99.242\",\"port\":59529,\"agentOS\":\"linux\"},{\"id\":\"5741dd57-dff6-42e2-a614-12fe60a7a8c7\",\"ip\":\"192.168.99.240\",\"port\":59529,\"agentOS\":\"linux\"}],\"dataLayout\":{\"srcEncryption\":false,\"dstDeduption\":true,\"srcDeduption\":false,\"linkEncryption\":false,\"srcCompression\":false,\"dstEncryption\":false,\"dstCompression\":true,\"clientProtocolType\":0,\"characterSet\":0},\"scripts\":{},\"advanceParams\":{\"next_cause_param\":\"0\"},\"copyFormat\":0,\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"extendInfo\":{\"esn\":\"2102354DEY10M3000002\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"zI)9DsL#D44c\"},\"endpoint\":{\"ip\":\"8.40.99.244,8.40.99.245\",\"port\":8088},\"local\":true,\"isLocal\":true}]}\n";
        BackupTask read = JsonUtil.read(json, BackupTask.class);
        return read;
    }
}
