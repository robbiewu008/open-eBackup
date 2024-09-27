/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.saphana.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link SapHanaDatabaseProvider Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-18
 */
public class SapHanaDatabaseProviderTest {
    private static final long ACCESS_DB_ERROR = 1577213476L;

    private final ClusterEnvironmentService clusterEnvironmentService = PowerMockito.mock(
        ClusterEnvironmentService.class);

    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaDatabaseProvider hanaDbProvider = new SapHanaDatabaseProvider(clusterEnvironmentService,
        hanaResourceService);

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：applicable输入资源信息
     * 检查点：SAPHANA-database类型资源返回true；其他返回false
     */
    @Test
    public void applicable_sap_hana_database_provider_success() {
        ProtectedResource dbResource = new ProtectedResource();
        dbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        Assert.assertTrue(hanaDbProvider.applicable(dbResource));
        ProtectedResource instanceResource = new ProtectedResource();
        instanceResource.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        Assert.assertFalse(hanaDbProvider.applicable(instanceResource));
    }

    /**
     * 用例场景：集群租户数据库注册
     * 前置条件：集群租户数据库信息正确，检查连通性正常
     * 检查点: 集群租户数据库状态为在线
     */
    @Test
    public void should_db_online_if_create_cluster_tenant_db_success_when_beforeCreate() {
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .setDatabaseResourceInfo(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkDbIsRegistered(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkDbIsRegisteredInGeneralDb(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.when(hanaResourceService.getResourceById("5ded83e5f6364678a26f08bd8cccd2a9"))
            .thenReturn(mockInstanceResource());
        PowerMockito.when(hanaResourceService.queryEnvironments(ArgumentMatchers.anyList()))
            .thenReturn(mockClusterAgents());
        ProtectedResource tenantDbReq = mockCreateClusterTenantDbReqInfo();
        hanaDbProvider.beforeCreate(tenantDbReq);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            tenantDbReq.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    private ProtectedResource mockCreateClusterTenantDbReqInfo() {
        ProtectedResource dbReqResource = new ProtectedResource();
        dbReqResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        dbReqResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        dbReqResource.setName("tenant1");
        dbReqResource.setParentUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        extendInfo.put(SapHanaConstants.NODES, nodes);
        dbReqResource.setExtendInfo(extendInfo);
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        ProtectedResource secondAgent = new ProtectedResource();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        List<ProtectedResource> agents = Arrays.asList(firstAgent, secondAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        dbReqResource.setDependencies(dependencies);
        return dbReqResource;
    }

    private List<ProtectedEnvironment> mockClusterAgents() {
        List<ProtectedEnvironment> envs = new ArrayList<>();
        ProtectedEnvironment firstAgent = new ProtectedEnvironment();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        firstAgent.setEndpoint("10.10.10.10");
        firstAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        firstAgent.setOsType("linux");
        envs.add(firstAgent);
        ProtectedEnvironment secondAgent = new ProtectedEnvironment();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        secondAgent.setEndpoint("10.10.10.11");
        secondAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        secondAgent.setOsType("linux");
        envs.add(secondAgent);
        return envs;
    }

    private ProtectedResource mockInstanceResource() {
        ProtectedResource instance = new ProtectedResource();
        instance.setUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        instance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        instance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "s00");
        return instance;
    }

    private ResourceCheckContext mockClusterTenantDbCheckContext() {
        ResourceCheckContext checkContext = new ResourceCheckContext();
        ActionResult firResult = new ActionResult();
        firResult.setCode(DatabaseConstants.SUCCESS_CODE);
        ActionResult secResult = new ActionResult();
        firResult.setCode(DatabaseConstants.SUCCESS_CODE);
        checkContext.setActionResults(Arrays.asList(firResult, secResult));
        return checkContext;
    }

    /**
     * 用例场景：单机租户数据库注册
     * 前置条件：单机租户数据库信息不正确，检查连通性失败
     * 检查点: 抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_ex_if_update_single_tenant_db_fail_when_beforeUpdate() {
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .setDatabaseResourceInfo(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkDbIsRegistered(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkDbIsRegisteredInGeneralDb(ArgumentMatchers.any(ProtectedResource.class));
        PowerMockito.when(hanaResourceService.getResourceById("5ded83e5f6364678a26f08bd8cccd2a9"))
            .thenReturn(mockInstanceResource());
        PowerMockito.when(hanaResourceService.queryEnvironments(ArgumentMatchers.anyList()))
            .thenReturn(mockSingleAgentList());
        PowerMockito.doThrow(new LegoCheckedException(ACCESS_DB_ERROR, "check database connection failed."))
            .when(hanaResourceService)
            .checkDatabaseConnection(ArgumentMatchers.any(ProtectedResource.class));
        ProtectedResource singleTenantDbReq = mockUpdateSingleTenantDbReqInfo();
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> hanaDbProvider.beforeUpdate(singleTenantDbReq));
        Assert.assertEquals(ACCESS_DB_ERROR, legoCheckedException.getErrorCode());
    }

    private ProtectedResource mockUpdateSingleTenantDbReqInfo() {
        ProtectedResource dbReqResource = new ProtectedResource();
        dbReqResource.setUuid("7a6ef4f90eac496cb4a1a548d330b201");
        dbReqResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        dbReqResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        dbReqResource.setName("s-tenant1");
        dbReqResource.setParentUuid("5ded83e5f6364678a26f08bd8cccd2a9");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"}]";
        extendInfo.put(SapHanaConstants.NODES, nodes);
        dbReqResource.setExtendInfo(extendInfo);
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        List<ProtectedResource> agents = Collections.singletonList(firstAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        dbReqResource.setDependencies(dependencies);
        return dbReqResource;
    }

    private List<ProtectedEnvironment> mockSingleAgentList() {
        List<ProtectedEnvironment> envs = new ArrayList<>();
        ProtectedEnvironment firstAgent = new ProtectedEnvironment();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        firstAgent.setEndpoint("10.10.10.20");
        firstAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        firstAgent.setOsType("linux");
        envs.add(firstAgent);
        return envs;
    }

    /**
     * 用例场景：获取资源的特性，比如是否校验名称重复
     * 前置条件：资源为SAP HANA数据库
     * 检查点: 资源特性是否校验名称重复为false
     */
    @Test
    public void should_not_check_resource_name_duplicate_if_db_when_getResourceFeature() {
        ResourceFeature resourceFeature = hanaDbProvider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isShouldCheckResourceNameDuplicate());
    }
}
