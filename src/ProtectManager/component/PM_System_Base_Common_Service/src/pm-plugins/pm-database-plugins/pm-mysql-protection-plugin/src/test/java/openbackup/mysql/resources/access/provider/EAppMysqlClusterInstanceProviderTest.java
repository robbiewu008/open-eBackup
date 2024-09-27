/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author fWX1071802
 * @since 2023/6/16
 */
@RunWith(PowerMockRunner.class)
public class EAppMysqlClusterInstanceProviderTest {

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final MysqlBaseService mysqlBaseService = Mockito.mock(MysqlBaseService.class);

    private EAppMysqlClusterInstanceProvider instance;

    @Rule
    ExpectedException exception = ExpectedException.none();

    @Before
    public void init() {
        instance = new EAppMysqlClusterInstanceProvider();
        instance.setAgentUnifiedService(agentUnifiedService);
        instance.setMysqlBaseService(mysqlBaseService);
    }

    @Test
    public void test_checkIsCluster_success() {
        ProtectedResource oldResource = new ProtectedResource();
        oldResource.setAuth(new Authentication());
        PowerMockito.when(mysqlBaseService.getResource("eapp_instance_1")).thenReturn(oldResource);
        PowerMockito.when(mysqlBaseService.getEnvironmentById("instance1")).thenReturn(new ProtectedEnvironment());
        AppEnvResponse response = new AppEnvResponse();
        response.setExtendInfo(
            ImmutableMap.of(MysqlConstants.MASTER_LIST, "ip1", MysqlConstants.CURRENT_IP_LIST, "ip2"));
        PowerMockito.when(
            agentUnifiedService.getClusterInfo(any(ProtectedResource.class), any(ProtectedEnvironment.class)))
            .thenReturn(response);
        exception.expect(LegoCheckedException.class);
        instance.checkIsCluster(getResource());
    }



    @Test
    public void test_checkIsCluster_when_error() {
        ProtectedResource oldResource = new ProtectedResource();
        oldResource.setAuth(new Authentication());
        PowerMockito.when(mysqlBaseService.getResource("eapp_instance_1")).thenReturn(oldResource);
        PowerMockito.when(mysqlBaseService.getEnvironmentById("instance1")).thenReturn(new ProtectedEnvironment());
        AppEnvResponse response = new AppEnvResponse();
        response.setExtendInfo(
            ImmutableMap.of(MysqlConstants.ERROR_CODE, "1"));
        PowerMockito.when(
            agentUnifiedService.getClusterInfo(any(ProtectedResource.class), any(ProtectedEnvironment.class)))
            .thenReturn(response);
        exception.expect(DataProtectionAccessException.class);
        instance.checkIsCluster(getResource());
    }

    private ProtectedResource getResource() {
        ProtectedResource resource = new ProtectedResource();
        ProtectedResource instance = new ProtectedResource();
        instance.setExtendInfo(ImmutableMap.of());
        Authentication authentication = new Authentication();
        instance.setAuth(authentication);
        instance.setName("eapp_instance_1");
        instance.setUuid("eapp_instance_1");
        instance.setExtendInfo(ImmutableMap.of(DatabaseConstants.HOST_ID, "instance1"));
        resource.setDependencies(ImmutableMap.of(DatabaseConstants.CHILDREN, Lists.newArrayList(instance)));
        return resource;
    }
}
