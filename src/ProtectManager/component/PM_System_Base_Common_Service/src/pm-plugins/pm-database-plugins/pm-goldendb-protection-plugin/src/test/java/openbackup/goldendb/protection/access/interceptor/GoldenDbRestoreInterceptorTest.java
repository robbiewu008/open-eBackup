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

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import openbackup.goldendb.protection.access.provider.GoldenDBAgentProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.assertj.core.api.Assertions;
import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.List;
import java.util.stream.Collectors;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbRestoreInterceptorTest {

    @Mock
    private GoldenDbService mockGoldenDbService;

    @Mock
    private CopyRestApi mockCopyRestApi;

    private GoldenDbRestoreInterceptor goldenDbRestoreInterceptorUnderTest;

    @Before
    public void setUp() {
        goldenDbRestoreInterceptorUnderTest = new GoldenDbRestoreInterceptor(mockGoldenDbService, mockCopyRestApi,
            new GoldenDBAgentProvider(mockGoldenDbService));
    }

    @Test
    public void testBuildRestoreMode() {
        final Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        when(mockCopyRestApi.queryCopyByID(any())).thenReturn(copy);

        RestoreTask task = new RestoreTask();
        goldenDbRestoreInterceptorUnderTest.buildRestoreMode(task);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), task.getRestoreMode());
    }

    @Test
    public void testApplicable() {
        assertTrue(
            goldenDbRestoreInterceptorUnderTest.applicable(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType()));
    }

    @Test
    public void testIntercept() {
        // Configure GoldenDbService.getEnvironmentById(...).
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getEnvironment());

        // Configure GoldenDbService.getResourceById(...).
        when(mockGoldenDbService.getResourceById(any())).thenReturn(getEnvironment());

        // Configure CopyRestApi.queryCopyByID(...).
        final Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        copy.setUuid("d24696d6-1a4a-47ef-a0dd-adec7f32aa6a");
        copy.setAmount(0);
        copy.setGn(0);
        copy.setPrevCopyId("prevCopyId");
        copy.setNextCopyId("nextCopyId");
        copy.setPrevCopyGn(0);
        copy.setNextCopyGn(0);
        when(mockCopyRestApi.queryCopyByID(any())).thenReturn(copy);

        // Run the test
        RestoreTask task = getRestoreTask();
        final RestoreTask result = goldenDbRestoreInterceptorUnderTest.initialize(task);
        Endpoint endpoint1 = new Endpoint("1a996406-f51f-4ed3-8221-dd1ef7dae3f4", "10.10.10.10", 123);
        Endpoint endpoint2 = new Endpoint("d862703c-876f-4ca6-b068-ebc0867da1bc", "10.10.10.11", 124);

        List<Endpoint> agents = Lists.newArrayList(endpoint1, endpoint2, endpoint1, endpoint2);
        Assertions.assertThat(result.getAgents()).usingRecursiveComparison().isEqualTo(agents);
        // Verify the results
        Assert.assertTrue(true);
    }

    private RestoreTask getRestoreTask() {
        String json
            = "{\"requestId\":\"a8b218bc-d99e-416e-a1be-e1e819d1f1ea\",\"taskId\":\"a8b218bc-d99e-416e-a1be-e1e819d1f1ea\",\"copyId\":\"f6e1b393-fc93-4cb0-8188-ec0986156b77\",\"targetEnv\":{\"uuid\":\"6efa4312b7c74c96bbdf3b5cd35bdd14\",\"name\":\"集群109\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"rootUuid\":\"6efa4312b7c74c96bbdf3b5cd35bdd14\",\"extendInfo\":{\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"parentUuid\\\":\\\"037ec067-88b8-4b5e-b854-9ba10105c108\\\",\\\"parentName\\\":\\\"golden-109(192.168.165.109)\\\",\\\"osUser\\\":\\\"zxmanager\\\",\\\"nodeType\\\":\\\"managerNode\\\"}]}\",\"$citations_agents_a73bf715d93646d0ae59851a66ef4200\":\"16debdbcfcc146f6b8bec0e339a936eb\"},\"endpoint\":\"192.168.165.109\",\"port\":0},\"targetObject\":{\"uuid\":\"7e7b219da15242ea9e8036c3f5f7b578\",\"name\":\"实例109\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"rootUuid\":\"6efa4312b7c74c96bbdf3b5cd35bdd14\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"cluster1\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"DN1\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.165.110\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"233df7d4-5f43-4650-8a9c-12c0c30e2403\\\",\\\"parentName\\\":\\\"golden-110(192.168.165.110)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"2\\\",\\\"name\\\":\\\"DN2\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.165.111\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"e7305cf3-02f4-47c1-bdcf-889b3adbcc53\\\",\\\"parentName\\\":\\\"golden-111(192.168.165.111)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"DN3\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.165.111\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"e7305cf3-02f4-47c1-bdcf-889b3adbcc53\\\",\\\"parentName\\\":\\\"golden-111(192.168.165.111)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"4\\\",\\\"name\\\":\\\"DN4\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.165.110\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"233df7d4-5f43-4650-8a9c-12c0c30e2403\\\",\\\"parentName\\\":\\\"golden-110(192.168.165.110)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null}]}],\\\"gtm\\\":[{\\\"parentUuid\\\":\\\"233df7d4-5f43-4650-8a9c-12c0c30e2403\\\",\\\"parentName\\\":\\\"golden-110(192.168.165.110)\\\",\\\"osUser\\\":\\\"zxgtm1\\\",\\\"nodeType\\\":\\\"gtmNode\\\"}]}\"},\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"parentUuid\":\"6efa4312b7c74c96bbdf3b5cd35bdd14\"},\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Database_7e7b219da15242ea9e8036c3f5f7b578/source_policy_7e7b219da15242ea9e8036c3f5f7b578_Context_Global_MD\",\"id\":\"72\"},{\"type\":1,\"path\":\"/Database_7e7b219da15242ea9e8036c3f5f7b578/source_policy_7e7b219da15242ea9e8036c3f5f7b578_Context\",\"id\":\"72\"}],\"extendInfo\":{\"esn\":\"2102352TRW36K9300001\",\"copy_format\":0},\"extendAuth\":{\"authType\":2,\"authKey\":\"admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAlY47r2WDecHcMyX6MCHckMeQbvbO55wyB5qF+tAAAAAAAAAAAAAAAAAAAAGck6UdQrKSqmjtrxVy5JF7Rg2VXGIp9ixMs=\"},\"endpoint\":{\"ip\":\"8.40.101.98,8.40.101.99,2016:8:40:96:c11::131\",\"port\":8088},\"local\":true,\"isLocal\":true},{\"type\":2,\"protocol\":1,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Database_CacheDataRepository/7e7b219da15242ea9e8036c3f5f7b578\",\"id\":\"13\"}],\"extendInfo\":{\"esn\":\"2102352TRW36K9300001\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAlY47r2WDecHeTPHdXq6dnrg3Pv3bhaXSNgmgWEAAAAAAAAAAAAAAAAAAAAGYk4H+CN6OMQkEWz15Cz+KRD39XvHvYAq5E=\"},\"endpoint\":{\"ip\":\"8.40.101.98,8.40.101.99,2016:8:40:96:c11::131\",\"port\":8088},\"local\":true,\"isLocal\":true}],\"agents\":[{\"id\":\"233df7d4-5f43-4650-8a9c-12c0c30e2403\",\"ip\":\"192.168.165.110\",\"port\":59520},{\"id\":\"037ec067-88b8-4b5e-b854-9ba10105c108\",\"ip\":\"192.168.165.109\",\"port\":59520},{\"id\":\"e7305cf3-02f4-47c1-bdcf-889b3adbcc53\",\"ip\":\"192.168.165.111\",\"port\":59525}],\"restoreType\":\"normalRestore\",\"restoreMode\":\"LocalRestore\",\"targetLocation\":\"original\"}";
        RestoreTask read = JsonUtil.read(json, RestoreTask.class);
        return read;
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"name\":\"cluster1\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"parentUuid\":\"e370a676073340349a1322672b9ada50\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\"},\"extendInfo\":{\"clusterInfo\":\"{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"cluster1\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"DN1\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.165.212\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"1a996406-f51f-4ed3-8221-dd1ef7dae3f4\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.212)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"2\\\",\\\"name\\\":\\\"DN2\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.165.213\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"d862703c-876f-4ca6-b068-ebc0867da1bc\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.213)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"DN3\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.165.213\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"d862703c-876f-4ca6-b068-ebc0867da1bc\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.213)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"4\\\",\\\"name\\\":\\\"DN4\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.165.212\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"1a996406-f51f-4ed3-8221-dd1ef7dae3f4\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.212)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null}]}],\\\"gtm\\\":[{\\\"parentUuid\\\":\\\"1a996406-f51f-4ed3-8221-dd1ef7dae3f4\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.212)\\\",\\\"osUser\\\":\\\"zxgtm1\\\",\\\"nodeType\\\":\\\"gtmNode\\\"},{\\\"parentUuid\\\":\\\"d862703c-876f-4ca6-b068-ebc0867da1bc\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.165.213)\\\",\\\"osUser\\\":\\\"zxgtm1\\\",\\\"nodeType\\\":\\\"gtmNode\\\"}]}\",\"local_ini_cnf\":\"W2NvbW1vbl0KI1Jvb3QgZGlyIG9mIGJhY2t1cGluZzsKI3VuaXQ6IE5BLCByYW5nZTogTkEsIGRlZmF1bHQ6ICRIT01FL2JhY2t1cF9yb290CmJhY2t1cF9yb290ZGlyID0K\"},\"dependencies\":{\"agents\":[{\"uuid\":\"1a996406-f51f-4ed3-8221-dd1ef7dae3f4\",\"endpoint\":\"10.10.10.10\",\"port\":123},{\"uuid\":\"d862703c-876f-4ca6-b068-ebc0867da1bc\",\"endpoint\":\"10.10.10.11\",\"port\":124}],\"-agents\":[]}}";

        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        read.getDependencies()
            .put("agents", read.getDependencies()
                .get("agents")
                .stream()
                .map(item -> BeanTools.copy(item, ProtectedEnvironment::new))
                .collect(Collectors.toList()));
        return read;
    }
}
