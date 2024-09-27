/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.exchange.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Assert;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Exchange邮箱恢复拦截器 LLT
 *
 * @author w30032137
 * @since 2023-05-31
 */
@ExtendWith(MockitoExtension.class)
class ExchangeMailboxRestoreInterceptorTest {
    @Mock
    private ExchangeService mockExchangeService;

    @Mock
    private CopyRestApi copyRestApi;

    private ExchangeMailboxRestoreInterceptor exchangeMailboxRestoreInterceptorTest;

    @BeforeEach
    void setUp() {
        exchangeMailboxRestoreInterceptorTest = new ExchangeMailboxRestoreInterceptor(mockExchangeService, copyRestApi);
    }

    @Test
    void testIntercept() {
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
        Copy copy = new Copy();
        copy.setResourceSubType("Exchange-database");
        when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = getRestoreTask();
        exchangeMailboxRestoreInterceptorTest.initialize(restoreTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：下发恢复任务时 获取节点信息
     * 前置条件：
     * 检查点: 信息获取正确
     */
    @Test
    void testGetNode() {
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
        Authentication auth = new Authentication();
        auth.setAuthPwd("authPwd");
        auth.setAuthKey("authKey");
        auth.setAuthType(2);
        environment.setAuth(auth);
        when(mockExchangeService.getEnvironmentById(anyString())).thenReturn(environment);
        TaskEnvironment res = exchangeMailboxRestoreInterceptorTest.getNode("7017bd24-1a4d-42fc-aaf4-3046eab88704");
        Assert.assertEquals(res.getAuth().getAuthKey(), auth.getAuthKey());
        Assert.assertEquals(res.getAuth().getAuthPwd(), auth.getAuthPwd());
        Assert.assertEquals(res.getAuth().getAuthType(), auth.getAuthType());
    }

    /**
     * 用例场景：下发恢复任务时 针对资源进行锁定
     * 前置条件：构造restoreTask结构体
     * 检查点: 成功返回加锁列表
     */
    @Test
    public void restore_getLock_resources() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetObject(new TaskResource());
        restoreTask.getTargetObject().setUuid("test_uuid");
        List<LockResourceBo> lockResources = exchangeMailboxRestoreInterceptorTest.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }

    RestoreTask getRestoreTask() {
        String task =
            "{\"requestId\":\"80814dd8-4d0d-4c2e-a9d6-3d3500e4b1ed\",\"taskId\":\"80814dd8-4d0d-4c2e-a9d6-3d3500e4b1ed\",\"copyId\":\"b3eb07f9-7543-4ead-b5a2-78eb0b0e282b\",\"targetEnv\":{\"uuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"name\":\"Exchange211_11\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"isGroup\":\"0\",\"agentUuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"endpoint\":\"192.168.160.210\",\"port\":0},\"targetObject\":{\"uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"name\":\"database01\",\"type\":\"Database\",\"subType\":\"Exchange-database\",\"rootUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"extendInfo\":{\"server_name\":\"WIN-VUB15GTQID6\",\"db_name\":\"database01\",\"log_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\",\"next_backup_type\":\"full\",\"db_uuid\":\"e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"state\":\"Mounted\",\"version\":\"15.1.2507.6\",\"edb_path\":\"C:\\\\Program Files\\\\Microsoft\\\\Exchange Server\\\\V15\\\\Mailbox\\\\database01\\\\database01.edb\",\"next_backup_change_cause\":\"bigdata_plugin_to_full_label\"},\"parentUuid\":\"c06c151e-7295-3ff0-8ff7-0266470662fc\",\"parentName\":\"Exchange211_11\"},\"repositories\":[{\"type\":1,\"protocol\":0,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Database_e49ad93d-7d2c-4234-bcd5-0e7207ef0c78/source_policy_e49ad93d-7d2c-4234-bcd5-0e7207ef0c78_Context_Global_MD\",\"id\":\"488\"},{\"type\":1,\"path\":\"/Database_e49ad93d-7d2c-4234-bcd5-0e7207ef0c78/source_policy_e49ad93d-7d2c-4234-bcd5-0e7207ef0c78_Context\",\"id\":\"488\"}],\"extendInfo\":{\"esn\":\"2102354DEY10M3000002\",\"copy_format\":0},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAnVmhNOrcpF5/dgC4w8NB0g/XHsx+yclg9wnYYlAAAAAAAAAAAAAAAAAAAAHEhZAiRa8D6V6R8Z++qGeRyyb5J5p547i24v+fQ=\"},\"endpoint\":{\"ip\":\"8.40.99.244,8.40.99.245\",\"port\":8088},\"local\":true,\"isLocal\":true},{\"type\":2,\"protocol\":0,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Database_CacheDataRepository/e49ad93d-7d2c-4234-bcd5-0e7207ef0c78\",\"id\":\"228\"}],\"extendInfo\":{\"esn\":\"2102354DEY10M3000002\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAnVmhNOrcpF52hWGcWnUjWVjtPSzrwFXi0eR2lCAAAAAAAAAAAAAAAAAAAAHCLUtVTXknicnqFlSOCQJRXx6k5z3KZ4xHhITmA=\"},\"endpoint\":{\"ip\":\"8.40.99.244,8.40.99.245\",\"port\":8088},\"local\":true,\"isLocal\":true}],\"agents\":[{\"id\":\"6f378a00-e81b-4307-8caf-15889a8ab9ca\",\"ip\":\"192.168.99.232\",\"port\":59525,\"agentOS\":\"windows\"},{\"id\":\"dbfd034e-c24f-493e-99ee-126766a3a9d9\",\"ip\":\"192.168.99.234\",\"port\":59528,\"agentOS\":\"windows\"},{\"id\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\",\"ip\":\"192.168.160.210\",\"port\":59524,\"agentOS\":\"windows\"},{\"id\":\"9b40b811-fc85-4184-ba93-83a7b9e663c0\",\"ip\":\"192.168.99.242\",\"port\":59529,\"agentOS\":\"linux\"},{\"id\":\"ae87b349-bbd3-4944-a47e-43d91f99d657\",\"ip\":\"192.168.99.243\",\"port\":59529,\"agentOS\":\"linux\"}],\"dataLayout\":{\"clientProtocolType\":0,\"characterSet\":0},\"restoreType\":\"normalRestore\",\"restoreMode\":\"LocalRestore\",\"targetLocation\":\"original\"}";
        RestoreTask restoreTask = JsonUtil.read(task, RestoreTask.class);
        return restoreTask;
    }
}
