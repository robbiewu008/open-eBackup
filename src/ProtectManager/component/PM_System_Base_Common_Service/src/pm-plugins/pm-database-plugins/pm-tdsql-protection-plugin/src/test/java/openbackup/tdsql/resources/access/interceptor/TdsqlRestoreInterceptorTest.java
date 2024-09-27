package openbackup.tdsql.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.provider.TdsqlAgentProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;

import com.google.common.collect.Lists;

import org.assertj.core.api.Assertions;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-04
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlRestoreInterceptorTest {
    @Mock
    private TdsqlService mockTdsqlService;

    @Mock
    private CopyRestApi mockCopyRestApi;

    private TdsqlRestoreInterceptor tdsqlRestoreInterceptorUnderTest;

    @Before
    public void setUp() {
        tdsqlRestoreInterceptorUnderTest = new TdsqlRestoreInterceptor(mockTdsqlService, mockCopyRestApi,
            new TdsqlAgentProvider(mockTdsqlService));
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            tdsqlRestoreInterceptorUnderTest.applicable(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()));
        Assert.assertFalse(tdsqlRestoreInterceptorUnderTest.applicable("object"));
    }

    /**
     * 用例场景：下发恢复任务
     * 前置条件：资源备份成功
     * 检查点：恢复任务下发是否成功
     */
    @Test
    public void test_intercept() {
        // Configure getEnvironmentById(...).
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getEnvironment());

        // Configure getResourceById(...).
        when(mockTdsqlService.getResourceById(any())).thenReturn(getEnvironment());

        // Configure CopyRestApi.queryCopyByID(...).
        final Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        copy.setUuid("d4815037-7fcf-4d0d-8c56-e0ed0935c47d");
        copy.setAmount(0);
        copy.setGn(0);
        copy.setPrevCopyId("prevCopyId");
        copy.setNextCopyId("nextCopyId");
        copy.setPrevCopyGn(0);
        copy.setNextCopyGn(0);
        when(mockCopyRestApi.queryCopyByID(any())).thenReturn(copy);

        // Run the test
        RestoreTask task = getRestoreTask();
        final RestoreTask result = tdsqlRestoreInterceptorUnderTest.initialize(task);
        Endpoint endpoint0 = new Endpoint("16f74c9f-915c-4af6-91f6-40c643f13fd5", "192.168.147.40", 59531);
        Endpoint endpoint1 = new Endpoint("7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7", "192.168.147.38", 59530);
        Endpoint endpoint2 = new Endpoint("c1099cbe-9fa2-42be-95a9-b629c29d8b48", "192.168.147.39", 59522);

        List<Endpoint> agents = Lists.newArrayList(endpoint0, endpoint1, endpoint2);
        Assertions.assertThat(result.getAgents()).usingRecursiveComparison().isEqualTo(agents);

        // Verify the results
        Assert.assertEquals(result.getRestoreType(), "normalRestore");
        Assert.assertEquals(result.getRestoreMode(), "DownloadRestore");
    }

    private RestoreTask getRestoreTask() {
        String json
            = " {\"requestId\":\"fb751b71-476c-41c9-969c-69904c158320\",\"taskId\":\"fb751b71-476c-41c9-969c-69904c158320\",\"copyId\":\"af591202-9ffe-4e99-bd74-9496a335b65f\",\"targetEnv\":{\"uuid\":\"a5a3351c-33a2-3d95-8dfb-14a5a3152345\",\"name\":\"cluster60\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"rootUuid\":\"a5a3351c-33a2-3d95-8dfb-14a5a3152345\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"d4815037-7fcf-4d0d-8c56-e0ed0935c47d\\\",\\\"ip\\\":\\\"8.40.160.60\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"d1bd8dac-d280-4792-a3b8-e89d6567b1aa\\\",\\\"ip\\\":\\\"8.40.160.61\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"d4815037-7fcf-4d0d-8c56-e0ed0935c47d\\\",\\\"ip\\\":\\\"8.40.160.60\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"d1bd8dac-d280-4792-a3b8-e89d6567b1aa\\\",\\\"ip\\\":\\\"8.40.160.61\\\"}]}\"},\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"endpoint\":\"192.168.160.60,192.168.160.61,192.168.160.60,192.168.160.61\",\"port\":0},\"targetObject\":{\"uuid\":\"cf864da881f346eda15e81544f0ff225\",\"name\":\"set_1685694422_9\",\"type\":\"Database\",\"subType\":\"TDSQL-clusterInstance\",\"rootUuid\":\"a5a3351c-33a2-3d95-8dfb-14a5a3152345\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInstanceInfo\":\"{\\\"id\\\":\\\"set_1685694422_9\\\",\\\"name\\\":\\\"set1\\\",\\\"cluster\\\":\\\"a5a3351c-33a2-3d95-8dfb-14a5a3152345\\\",\\\"type\\\":\\\"0\\\",\\\"groups\\\":[{\\\"setId\\\":\\\"set_1685694422_9\\\",\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.160.60\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"1\\\",\\\"priority\\\":\\\"3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"d4815037-7fcf-4d0d-8c56-e0ed0935c47d\\\"},{\\\"ip\\\":\\\"8.40.160.61\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"0\\\",\\\"priority\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"d1bd8dac-d280-4792-a3b8-e89d6567b1aa\\\"},{\\\"ip\\\":\\\"8.40.160.62\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"0\\\",\\\"priority\\\":\\\"2\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"872a77ba-3d18-4751-91df-812831c86acc\\\"}]}]}\"},\"auth\":{\"authType\":2,\"authKey\":\"backup\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"parentUuid\":\"a5a3351c-33a2-3d95-8dfb-14a5a3152345\"},\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Database_0fc6cb490c73476bb90aa69e40f3c931/source_policy_0fc6cb490c73476bb90aa69e40f3c931_Context_Global_MD\",\"id\":\"482\"},{\"type\":1,\"path\":\"/Database_0fc6cb490c73476bb90aa69e40f3c931/source_policy_0fc6cb490c73476bb90aa69e40f3c931_Context\",\"id\":\"482\"}],\"extendInfo\":{\"esn\":\"2102354JDK10MA000003\",\"copy_format\":0},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAlgrOk2934vtTmRd9co5P2ypE0TrJP/UTiXwMVAAAAAAAAAAAAAAAAAAAAAHKzaVI44jQzLZlpGdqMJcQdxccFtHrV7yASdqjs=\"},\"endpoint\":{\"ip\":\"8.40.102.103,8.40.102.104\",\"port\":8088},\"local\":true,\"isLocal\":true},{\"type\":2,\"protocol\":1,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Database_CacheDataRepository/0fc6cb490c73476bb90aa69e40f3c931\",\"id\":\"483\"}],\"extendInfo\":{\"esn\":\"2102354JDK10MA000003\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAlgrOk2934vtYqNATvsLXB6O/CZaJ8199GXBytEAAAAAAAAAAAAAAAAAAAAHJLZdd924BIwuySlhEVlVRvgp0PcRGgAr/DVsEw=\"},\"endpoint\":{\"ip\":\"8.40.102.103,8.40.102.104\",\"port\":8088},\"local\":true,\"isLocal\":true}],\"agents\":[{\"id\":\"d1bd8dac-d280-4792-a3b8-e89d6567b1aa\",\"ip\":\"192.168.160.61\",\"port\":59534},{\"id\":\"d4815037-7fcf-4d0d-8c56-e0ed0935c47d\",\"ip\":\"192.168.160.60\",\"port\":59525},{\"id\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"ip\":\"192.168.160.62\",\"port\":59534},{\"id\":\"d1bd8dac-d280-4792-a3b8-e89d6567b1aa\",\"ip\":\"192.168.160.61\",\"port\":59534},{\"id\":\"d4815037-7fcf-4d0d-8c56-e0ed0935c47d\",\"ip\":\"192.168.160.60\",\"port\":59525}],\"dataLayout\":{\"clientProtocolType\":0,\"characterSet\":0},\"restoreType\":\"normalRestore\",\"restoreMode\":\"LocalRestore\",\"targetLocation\":\"new\"}";
        RestoreTask read = JsonUtil.read(json, RestoreTask.class);
        return read;
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"test\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
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
