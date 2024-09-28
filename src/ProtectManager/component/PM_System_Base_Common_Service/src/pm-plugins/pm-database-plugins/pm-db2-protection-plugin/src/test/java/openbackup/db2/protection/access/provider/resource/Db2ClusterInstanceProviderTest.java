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
package openbackup.db2.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.service.Db2InstanceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link Db2ClusterInstanceProvider} 测试类
 *
 */
public class Db2ClusterInstanceProviderTest {
    private static final String ENDPOINT = "127.0.0.1";

    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final Db2InstanceService db2InstanceService = Mockito.mock(Db2InstanceService.class);

    private final InstanceResourceService instanceResourceService = Mockito.mock(InstanceResourceService.class);

    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);

    private Db2ClusterInstanceProvider provider = new Db2ClusterInstanceProvider(environmentService, db2InstanceService,
        instanceResourceService, encryptorService);

    @Before
    public void init() {
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_cluster_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertFalse(provider.applicable(resource));
    }

    /**
     * 用例场景：创建db集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_db2_cluster_instance_before_create_success() {
        PowerMockito.when(db2InstanceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        PowerMockito.when(encryptorService.decrypt(any())).thenReturn("");
        ProtectedResource resource = mockResource();
        provider.beforeCreate(resource);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：修改db2集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    @Ignore
    public void execute_before_update_success() {
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(mockInstance());
        ProtectedResource resource = mockResource();
        provider.beforeUpdate(resource);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
        Assert.assertEquals("db2inst1",
            resource.getDependencies().get(DatabaseConstants.CHILDREN).get(0).getAuth().getAuthPwd());
    }

    /**
     * 用例场景：获取db2集群实例名称重复配置
     * 前置条件：资源信息正确
     * 检查点：返回false
     */
    @Test
    public void get_resource_feature_db2_cluster_instance_name_duplicate_configure() {
        ResourceFeature resourceFeature = provider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isShouldCheckResourceNameDuplicate());
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(mockEnv());
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setUuid(UUID.randomUUID().toString());
        Authentication auth = new Authentication();
        auth.setAuthKey("db2inst1");
        auth.setAuthPwd("");
        ProtectedResource instance = new ProtectedResource();
        instance.setUuid(UUID.randomUUID().toString());
        instance.setAuth(auth);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(instance);
        Map<String, List<ProtectedResource>> childrenMap = new HashMap<>();
        childrenMap.put(DatabaseConstants.CHILDREN, children);
        resource.setDependencies(childrenMap);
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(ENDPOINT);
        return environment;
    }

    private AgentBaseDto mockCheckResult() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "v10.5.5.5");
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorMessage(JSONObject.fromObject(map).toString());
        return checkResult;
    }

    private ProtectedResource mockInstance() {
        Authentication auth = new Authentication();
        auth.setAuthKey("db2inst1");
        auth.setAuthPwd("db2inst1");
        ProtectedResource instance = new ProtectedResource();
        instance.setAuth(auth);
        return instance;
    }
}
