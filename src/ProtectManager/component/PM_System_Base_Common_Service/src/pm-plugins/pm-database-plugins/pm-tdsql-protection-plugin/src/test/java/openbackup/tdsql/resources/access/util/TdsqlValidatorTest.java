package openbackup.tdsql.resources.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-25
 */
public class TdsqlValidatorTest {
    /**
     * 用例场景：创建/更新集群，进行环境参数检验
     * 前置条件：无
     * 检查点：校验成功不报错
     */
    @Test
    public void test_check_tdsql_success() {
        ProtectedEnvironment protectedEnvironment = getEnvironment();
        TdsqlValidator.checkTdsql(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getUuid(), "96590445-0df7-31f4-806b-9fb9e4ed548d");
    }

    /**
     * 用例场景：创建/更新集群，进行环境参数检验
     * 前置条件：环境名为空
     * 检查点：校验失败
     */
    @Test
    public void test_check_tdsql_failed_no_environment_name() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> TdsqlValidator.checkTdsql(protectedEnvironment));
        Assert.assertEquals("environment name is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建/更新集群，进行环境参数检验
     * 前置条件：环境extendInfo值为空
     * 检查点：校验失败
     */
    @Test
    public void test_check_tdsql_failed() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("test");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> TdsqlValidator.checkTdsql(protectedEnvironment));
        Assert.assertEquals("TDSQL cluster extendInfo is null.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    private ProtectedEnvironment getEnvironment() {
        String json = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
