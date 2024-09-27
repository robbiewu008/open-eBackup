/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述
 *
 * @author z00427109
 * @since 2023-11-18
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlClusterGroupBackupInterceptorTest {
    @Mock
    private TdsqlService mockTdsqlService;

    private TdsqlClusterGroupBackupInterceptor tdsqlClusterGroupBackupInterceptor;

    @Before
    public void setUp() {
        tdsqlClusterGroupBackupInterceptor = new TdsqlClusterGroupBackupInterceptor(mockTdsqlService,
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
            tdsqlClusterGroupBackupInterceptor.applicable(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType()));
        Assert.assertFalse(tdsqlClusterGroupBackupInterceptor.applicable("object"));
    }

    /**
     * 用例场景：测试补充agent信息成功
     * 前置条件：无
     * 检查点：agent信息正常添加到backupTask对象中
     */
    @Test
    public void test_supply_agent_success() {
        // 设置dependencies
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111");
        resource.setEndpoint("1.1.1.1");
        resource.setPort(123);
        list.add(resource);

        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setUuid("22222");
        resource2.setEndpoint("2.2.2.2");
        resource2.setPort(124);
        list.add(resource2);
        dependencies.put("agents", list);

        // 设置mock返回值
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("uuid");
        environment.setRootUuid("00000");
        environment.setEndpoint("102.168.175.42");
        environment.setPort(9116);
        environment.setDependencies(dependencies);
        when(mockTdsqlService.getResourceById(anyString())).thenReturn(environment);
        when(mockTdsqlService.getEnvironmentById(anyString())).thenReturn(environment);

        // Run the test
        BackupTask backupTask = getBackUpTask();
        tdsqlClusterGroupBackupInterceptor.supplyAgent(backupTask);
        List<StorageRepository> repositories = backupTask.getRepositories();
        Assert.assertEquals(backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SHARDING.getType());
        Assert.assertEquals(repositories.size(), 2);
        Assert.assertEquals(repositories.get(0).getType().intValue(), 1);
        Assert.assertEquals(repositories.get(1).getType().intValue(), 2);

        Endpoint endpoint1 = new Endpoint("11111", "1.1.1.1", 123);
        Endpoint endpoint2 = new Endpoint("22222", "2.2.2.2", 124);
        List<Endpoint> agents = Lists.newArrayList(endpoint1, endpoint2);
        Assertions.assertThat(backupTask.getAgents()).usingRecursiveComparison().isEqualTo(agents);
    }

    /**
     * 用例场景：测试备份任务填充backupTask参数
     * 前置条件：集群、实例注册成功，下发备份任务
     * 检查点：填充backupTask参数成功
     */
    @Test
    public void test_supply_backup_task_success() {
        ProtectedEnvironment environment = getEnvironment();
        environment.setExtendInfoByKey("id", "3");
        when(mockTdsqlService.getEnvironmentById("96590445-0df7-31f4-806b-9fb9e4ed548d")).thenReturn(getEnvironment());
        List<OssNode> ossNodes = new ArrayList<>();
        ossNodes.add(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);

        // Run the test
        final BackupTask result = tdsqlClusterGroupBackupInterceptor.supplyBackupTask(getBackUpTask());
        Assert.assertEquals(result.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        Assert.assertEquals(result.getCopyFormat(), CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
    }

    private BackupTask getBackUpTask() {
        String json
            = "{\"requestId\":\"ab07b626-d39d-4e68-93d8-2f72701bf860\",\"taskId\":\"ab07b626-d39d-4e68-93d8-2f72701bf860\",\"copyId\":\"ab07b626-d39d-4e68-93d8-2f72701bf860\",\"backupType\":\"fullBackup\",\"protectObject\":{\"uuid\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"name\":\"set1\",\"type\":\"Database\",\"subType\":\"TDSQL-clusterGroup\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterGroupInfo\":\"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[\\\"8.40.168.191\\\",\\\"8.40.168.190\\\",\\\"8.40.168.192\\\"]}}\",\"ossUrl\":\"http://8.40.147.38:8080/tdsql\",\"next_backup_type\":\"full\",\"next_backup_change_cause\":\"bigdata_plugin_to_full_label\"},\"parentUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"protectEnv\":{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\",\"deployType\":\"4\"},\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"nodes\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.40\",\"port\":59531},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.38\",\"port\":59530},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.39\",\"port\":59522},{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.40\",\"port\":59531},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.38\",\"port\":59530},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"agent\",\"subType\":\"UBackupAgent\",\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\"},\"endpoint\":\"192.168.147.39\",\"port\":59522}]},\"agents\":[{\"id\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"ip\":\"192.168.147.40\",\"port\":59531},{\"id\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"ip\":\"192.168.147.38\",\"port\":59530},{\"id\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"ip\":\"192.168.147.39\",\"port\":59522},{\"id\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"ip\":\"192.168.147.40\",\"port\":59531},{\"id\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"ip\":\"192.168.147.38\",\"port\":59530},{\"id\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"ip\":\"192.168.147.39\",\"port\":59522}],\"dataLayout\":{\"srcEncryption\":false,\"dstDeduption\":true,\"srcDeduption\":false,\"linkEncryption\":false,\"dstCompression\":true,\"srcCompression\":false,\"dstEncryption\":false,\"clientProtocolType\":0,\"characterSet\":0},\"scripts\":{},\"advanceParams\":{\"backupTask_sla\":\"{\\\"uuid\\\": \\\"ad75aef8-9e55-4347-89a8-3e5c3d08a2bb\\\", \\\"name\\\": \\\"SLA_1685535293705\\\", \\\"created_time\\\": \\\"2023-05-31T20:15:46.753+08:00\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Common\\\", \\\"policy_list\\\": [{\\\"uuid\\\": \\\"0fa26506-9752-4a9f-a7ca-1cf2703a4522\\\", \\\"name\\\": \\\"\\\\u5168\\\\u91cf01\\\", \\\"type\\\": \\\"backup\\\", \\\"action\\\": \\\"full\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 3, \\\"duration_unit\\\": \\\"d\\\", \\\"retention_quantity\\\": null, \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 2, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2023-05-31T00:00:00\\\", \\\"end_time\\\": null, \\\"window_start\\\": \\\"00:00:00\\\", \\\"window_end\\\": \\\"00:00:00\\\", \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"alarm_after_failure\\\": true, \\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"active\\\": true, \\\"is_active\\\": true}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null, \\\"is_global\\\": false}\",\"next_cause_param\":\"0\",\"alarm_after_failure\":\"true\"},\"copyFormat\":0,\"repositories\":[{\"type\":1,\"protocol\":1,\"role\":0,\"extendInfo\":{\"esn\":\"2102354JDK10MA000003\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"gY^0#!ag#i4s\"},\"endpoint\":{\"ip\":\"8.40.102.103,8.40.102.104\",\"port\":8088},\"local\":true,\"isLocal\":true},{\"type\":2,\"protocol\":1,\"role\":0,\"extendInfo\":{\"esn\":\"2102354JDK10MA000003\"},\"extendAuth\":{\"authType\":2,\"authKey\":\"dataprotect_admin\",\"authPwd\":\"gY^0#!ag#i4s\"},\"endpoint\":{\"ip\":\"8.40.102.103,8.40.102.104\",\"port\":8088},\"local\":true,\"isLocal\":true}]}";
        BackupTask task = JsonUtil.read(json, BackupTask.class);
        return task;
    }

    private ProtectedResource getResource() {
        String json = "{\\\"uuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"name\\\":\\\"test1\\\",\\\"type\\\":\\\"Database\\\",\\\"subType\\\":\\\"TDSQL-clusterGroup\\\",\\\"parentName\\\":\\\"\\\",\\\"parentUuid\\\":\\\"\\\",\\\"extendInfo\\\":{\\\"clusterGroupInfo\\\":\\\"{\\\\\\\"id\\\\\\\":\\\\\\\"group_1698889827_3\\\\\\\",\\\\\\\"type\\\\\\\":\\\\\\\"1\\\\\\\",\\\\\\\"group\\\\\\\":{\\\\\\\"setIds\\\\\\\":[\\\\\\\"set_1698890174_3\\\\\\\",\\\\\\\"set_1698890103_1\\\\\\\"],\\\\\\\"dataNodes\\\\\\\":[\\\\\\\"8.40.168.191\\\\\\\",\\\\\\\"8.40.168.190\\\\\\\",\\\\\\\"8.40.168.192\\\\\\\"]}}\\\"}}";
        return JsonUtil.read(json, ProtectedResource.class);
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"test\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private OssNode getOssNode() {
        String json
            = "{\"nodeType\":\"ossNode\",\"parentUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"ip\":\"8.40.147.38\",\"port\":\"8080\"}";
        return JsonUtil.read(json, OssNode.class);
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(tdsqlClusterGroupBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
