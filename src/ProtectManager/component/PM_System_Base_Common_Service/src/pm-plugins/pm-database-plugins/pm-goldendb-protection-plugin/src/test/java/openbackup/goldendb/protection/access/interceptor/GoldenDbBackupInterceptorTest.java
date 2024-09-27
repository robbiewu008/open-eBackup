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
package openbackup.goldendb.protection.access.interceptor;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.Assert.assertFalse;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.provider.GoldenDBAgentProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbBackupInterceptorTest {
    @Mock
    private GoldenDbService mockGoldenDbService;

    @Mock
    private GoldenDbTaskCheck goldenDbTaskCheck;

    @Mock
    private ResourceService resourceService;

    private GoldenDbBackupInterceptor goldenDbBackupInterceptorUnderTest;

    @Before
    public void setUp() {
        goldenDbBackupInterceptorUnderTest = new GoldenDbBackupInterceptor(mockGoldenDbService, goldenDbTaskCheck,
            resourceService, new GoldenDBAgentProvider(mockGoldenDbService));
    }

    @Test
    public void testApplicable() {
        assertFalse(goldenDbBackupInterceptorUnderTest.applicable("object"));
    }

    @Test
    public void testSupplyAgent() {
        // 设置dependencies
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111");
        resource.setEndpoint("10.10.10.10");
        resource.setPort(123);
        list.add(resource);

        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setUuid("22222");
        resource2.setEndpoint("10.10.10.11");
        resource2.setPort(124);
        list.add(resource2);
        dependencies.put("agents", list);

        // 设置mock返回值
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("uuid");
        environment.setRootUuid("00000");
        environment.setEndpoint("8.8.8.8");
        environment.setPort(66);
        environment.setDependencies(dependencies);

        when(mockGoldenDbService.getResourceById(anyString())).thenReturn(environment);
        when(mockGoldenDbService.getEnvironmentById(anyString())).thenReturn(environment);
        // Run the test
        BackupTask task = getBackUpTask();
        goldenDbBackupInterceptorUnderTest.supplyAgent(task);
        Endpoint endpoint1 = new Endpoint("11111", "10.10.10.10", 123);
        Endpoint endpoint2 = new Endpoint("22222", "10.10.10.11", 124);

        List<Endpoint> agents = Lists.newArrayList(endpoint1, endpoint2, endpoint1, endpoint2);

        assertThat(task.getAgents()).usingRecursiveComparison().isEqualTo(agents);
    }

    @Test
    public void testSupplyBackupTask() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        ProtectedEnvironment environment = getEnvironment();
        environment.setExtendInfoByKey("id", "3");
        pageListResponse.setRecords(Arrays.asList());
        pageListResponse.setRecords(Arrays.asList(environment));
        String clusterInfo = getEnvironment().getExtendInfo().get("clusterInfo");
        GoldenInstance goldenInstance = JsonUtil.read(clusterInfo, GoldenInstance.class);
        PowerMockito.when(goldenDbTaskCheck.checkEnvChange(any(), any())).thenReturn(goldenInstance);
        PowerMockito.when(mockGoldenDbService.getResourceById(any())).thenReturn(getEnvironment());

        // Run the test
        final BackupTask result = goldenDbBackupInterceptorUnderTest.supplyBackupTask(getBackUpTask());
        Assert.assertEquals(result.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SHARDING.getType());
        Assert.assertEquals(result.getCopyFormat(), CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
    }

    private BackupTask getBackUpTask() {
        String json
            = "{\"requestId\":\"6ba9c59c-0b45-4289-bdd4-34d107380029\",\"taskId\":\"6ba9c59c-0b45-4289-bdd4-34d107380029\",\"copyId\":\"6ba9c59c-0b45-4289-bdd4-34d107380029\",\"backupType\":\"fullBackup\",\"protectObject\":{\"uuid\":\"4ff4c80083434dcda4a4f3fe7ebcb0c2\",\"name\":\"cluster3\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"rootUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"cluster3\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"5\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"6\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb4\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb5\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"8\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb6\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]}],\\\"gtm\\\":[{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\",\\\"osUser\\\":\\\"zxgtm1\\\"},{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\",\\\"osUser\\\":\\\"zxgtm1\\\"}]}\"},\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"parentUuid\":\"5a9e688f541c4eb7a5017406c21839eb\"},\"protectEnv\":{\"uuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"name\":\"goldentest666222gg\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"rootUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"extendInfo\":{\"linkStatus\":\"0\",\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"nodeType\\\":\\\"managerNode\\\",\\\"parentUuid\\\":\\\"7017bd24-1a4d-42fc-aaf4-3046eab88704\\\",\\\"osUser\\\":\\\"zxmanager\\\"}]}\",\"description\":\"hello\"},\"endpoint\":\"8.8.8.8\",\"port\":0,\"nodes\":[{\"uuid\":\"8bc55739-8811-4b64-abac-35a49486a14c\",\"name\":\"localhost.localdomain\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.162.217,8.40.162.217,fe80::4f05:24c4:8d53:28f2,fe80::e4e8:ea76:d0cb:2a6f\"},\"endpoint\":\"192.168.162.217\",\"port\":59530},{\"uuid\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\",\"name\":\"localhost.localdomain\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.162.216,8.40.162.216,fe80::2287:e1e9:9215:14c6,fe80::b713:ab61:662:1189\"},\"endpoint\":\"192.168.162.216\",\"port\":59529},{\"uuid\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\",\"name\":\"localhost.localdomain\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.162.215,8.40.162.215,fe80::143f:609c:e47c:c9e4,fe80::725a:4840:47ce:9ac7\"},\"endpoint\":\"192.168.162.215\",\"port\":59530}]},\"agents\":[{\"id\":\"8bc55739-8811-4b64-abac-35a49486a14c\",\"ip\":\"192.168.162.217\",\"port\":59530},{\"id\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\",\"ip\":\"192.168.162.216\",\"port\":59529},{\"id\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\",\"ip\":\"192.168.162.215\",\"port\":59530}],\"dataLayout\":{\"srcEncryption\":false,\"dstDeduption\":true,\"srcDeduption\":false,\"linkEncryption\":false,\"dstEncryption\":false,\"srcCompression\":false,\"dstCompression\":true},\"scripts\":{},\"advanceParams\":{\"next_cause_param\":\"0\"},\"copyFormat\":0,\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"extendInfo\":{\"esn\":\"2102354JDK10N3100001\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"admin\",\"authPwd\":\"Admin@storage1\"},\"endpoint\":{\"ip\":\"8.40.129.171,8.40.129.170\",\"port\":8088},\"local\":true,\"isLocal\":true}]}";
        BackupTask read = JsonUtil.read(json, BackupTask.class);
        return read;
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"parentUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"name\":\"cluster3\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\"},\"port\":\"\",\"hostName\":\"\",\"ip\":\"\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"cluster3\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"5\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"6\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb4\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb5\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"8\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb6\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]}],\\\"gtm\\\":[{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\",\\\"osUser\\\":\\\"zxgtm1\\\"},{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\",\\\"osUser\\\":\\\"zxgtm1\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"8bc55739-8811-4b64-abac-35a49486a14c\"},{\"uuid\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertFalse(
            goldenDbBackupInterceptorUnderTest.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
