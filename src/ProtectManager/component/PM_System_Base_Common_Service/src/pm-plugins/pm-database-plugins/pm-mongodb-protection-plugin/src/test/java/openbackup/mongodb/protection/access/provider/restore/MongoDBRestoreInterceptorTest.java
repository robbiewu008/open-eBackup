/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mongodb.protection.access.provider.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.BDDMockito.given;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * mongo集群恢复任务下发provider 测试类
 *
 * @author lWX1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-24
 */
public class MongoDBRestoreInterceptorTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    private final MongoDBBaseService mongoDBBaseService = Mockito.mock(MongoDBBaseService.class);

    private final ProtectedEnvironmentRetrievalsService envRetrievalsService = Mockito.mock(ProtectedEnvironmentRetrievalsService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final MongoDBRestoreInterceptor mongoDBRestoreInterceptor = new MongoDBRestoreInterceptor(copyRestApi,
        envRetrievalsService,agentUnifiedService,mongoDBBaseService);

    /**
     * 用例场景：MongoDB恢复下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBRestoreInterceptor.applicable(ResourceSubTypeEnum.MONGODB_CLUSTER.getType()));
        Assert.assertTrue(mongoDBRestoreInterceptor.applicable(ResourceSubTypeEnum.MONGODB_SINGLE.getType()));
    }

    /**
     * 用例场景：MongoDB备份下发
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void intercept_success() {
        String str = "{\"name\":\"sunrui27101\",\"path\":\"8.40.96.71:27101\",\"root_uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"parent_name\":null,\"parent_uuid\":null,\"children_uuids\":null,\"type\":\"Database\",\"sub_type\":\"MongoDB-single\",\"uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"created_time\":\"2023-05-13T17:48:53.365000\",\"ext_parameters\":{},\"authorized_user\":null,\"user_id\":null,\"version\":\"4.4\",\"sla_id\":\"9ac3ae72-cfaa-49bd-a709-1f65b5ca4f42\",\"sla_name\":\"SLA_1683787861823\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"environment_name\":\"sunrui27101\",\"environment_endpoint\":\"8.40.96.71:27101\",\"environment_os_type\":null,\"environment_type\":\"Database\",\"environment_sub_type\":\"MongoDB-single\",\"environment_is_cluster\":\"False\",\"environment_os_name\":null,\"user_name\":null,\"endpoint\":\"8.40.96.71:27101\",\"port\":0,\"os_type\":null,\"os_name\":null,\"is_cluster\":false,\"scan_interval\":3600,\"link_status\":1,\"cert_name\":null,\"agent_version\":null,\"agent_timestamp\":null,\"password\":null,\"extendInfo\":{\"serviceIp\":\"8.40.96.71\",\"clusterType\":\"3\",\"clusterNodes\":\"[{\\\"dataPath\\\":\\\"/mongodb/single/data\\\",\\\"role\\\":\\\"1\\\",\\\"voteRight\\\":\\\"\\\",\\\"priority\\\":\\\"\\\",\\\"configPath\\\":\\\"/home/mongo4.4/single.conf\\\",\\\"hostUrl\\\":\\\"8.40.96.71:27101\\\",\\\"agentHost\\\":\\\"8.40.96.71:27101\\\",\\\"nodeStatus\\\":\\\"1\\\",\\\"shardClusterType\\\":\\\"single\\\",\\\"stateStr\\\":\\\"\\\",\\\"argv\\\":\\\"['/home/mongo4.4/mongo/bin/mongod', '-f', '/home/mongo4.4/single.conf']\\\",\\\"parsed\\\":\\\"{'config': '/home/mongo4.4/single.conf', 'net': {'bindIp': '0.0.0.0', 'port': 27101}, 'processManagement': {'fork': True}, 'storage': {'dbPath': '/mongodb/single/data', 'journal': {'enabled': True}, 'wiredTiger': {'engineConfig': {'cacheSizeGB': 1.0}}}, 'systemLog': {'destination': 'file', 'logAppend': True, 'path': '/mongodb/single/log/single.log'}}\\\",\\\"clusterInstanceName\\\":\\\"\\\",\\\"instanceNameList\\\":\\\"\\\"}]\",\"servicePort\":\"27101\",\"cluster_type\":\"3\",\"agentUuid\":\"17c34028-c338-4072-b0af-076b55ed3569\",\"version\":\"4.4\",\"isTopInstance\":\"1\"}}";
        Copy copy = new Copy();
        copy.setResourceProperties(str);
        given(copyRestApi.queryCopyByID(any())).willReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("111");
        taskEnvironment.setExtendInfo(new HashMap<>());
        restoreTask.setTargetEnv(taskEnvironment);
        restoreTask.setTargetObject(new TaskResource());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(DatabaseConstants.VERSION, "4.4");
        given(mongoDBBaseService.getEnvironmentById(any())).willReturn(protectedEnvironment);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("4.2");
        given(mongoDBBaseService.getResource(any())).willReturn(protectedResource);

        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        mongoDBRestoreInterceptor.initialize(restoreTask);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap
            = new HashMap<>();
        List<ProtectedEnvironment> protectedList = new ArrayList<>();
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setUuid("1111");
        env.setEndpoint("192.168.100.1");
        env.setPort(25080);
        protectedList.add(env);
        protectedResourceMap.put(new ProtectedResource(), protectedList);
        given(envRetrievalsService.collectConnectableResources("111")).willReturn(protectedResourceMap);
        HostDto hostDto = new HostDto();
        hostDto.setExtendInfo("{\"name\":\"sunrui27101\"}");
        hostDto.setUuid("22222");
        given(agentUnifiedService.getHost(any(),anyInt())).willReturn(hostDto);
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        mongoDBRestoreInterceptor.initialize(restoreTask);
        Assert.assertEquals(null,restoreTask.getTargetObject().getUuid());
    }

    /**
     * 用例场景：MongoDB备份下发
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void should_throw_LegoCheckedException_if_check_db_version_fail_when_intercept() {
        String resourceProperties = "{\"name\":\"sunrui27101\",\"path\":\"8.40.96.71:27101\",\"root_uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"parent_name\":null,\"parent_uuid\":null,\"children_uuids\":null,\"type\":\"Database\",\"sub_type\":\"MongoDB-single\",\"uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"created_time\":\"2023-05-13T17:48:53.365000\",\"ext_parameters\":{},\"authorized_user\":null,\"user_id\":null,\"version\":\"4.4\",\"sla_id\":\"9ac3ae72-cfaa-49bd-a709-1f65b5ca4f42\",\"sla_name\":\"SLA_1683787861823\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"e14ff17c-52aa-3494-b622-7377927e0d13\",\"environment_name\":\"sunrui27101\",\"environment_endpoint\":\"8.40.96.71:27101\",\"environment_os_type\":null,\"environment_type\":\"Database\",\"environment_sub_type\":\"MongoDB-single\",\"environment_is_cluster\":\"False\",\"environment_os_name\":null,\"user_name\":null,\"endpoint\":\"8.40.96.71:27101\",\"port\":0,\"os_type\":null,\"os_name\":null,\"is_cluster\":false,\"scan_interval\":3600,\"link_status\":1,\"cert_name\":null,\"agent_version\":null,\"agent_timestamp\":null,\"password\":null,\"extendInfo\":{\"serviceIp\":\"8.40.96.71\",\"clusterType\":\"3\",\"clusterNodes\":\"[{\\\"dataPath\\\":\\\"/mongodb/single/data\\\",\\\"role\\\":\\\"1\\\",\\\"voteRight\\\":\\\"\\\",\\\"priority\\\":\\\"\\\",\\\"configPath\\\":\\\"/home/mongo4.4/single.conf\\\",\\\"hostUrl\\\":\\\"8.40.96.71:27101\\\",\\\"agentHost\\\":\\\"8.40.96.71:27101\\\",\\\"nodeStatus\\\":\\\"1\\\",\\\"shardClusterType\\\":\\\"single\\\",\\\"stateStr\\\":\\\"\\\",\\\"argv\\\":\\\"['/home/mongo4.4/mongo/bin/mongod', '-f', '/home/mongo4.4/single.conf']\\\",\\\"parsed\\\":\\\"{'config': '/home/mongo4.4/single.conf', 'net': {'bindIp': '0.0.0.0', 'port': 27101}, 'processManagement': {'fork': True}, 'storage': {'dbPath': '/mongodb/single/data', 'journal': {'enabled': True}, 'wiredTiger': {'engineConfig': {'cacheSizeGB': 1.0}}}, 'systemLog': {'destination': 'file', 'logAppend': True, 'path': '/mongodb/single/log/single.log'}}\\\",\\\"clusterInstanceName\\\":\\\"\\\",\\\"instanceNameList\\\":\\\"\\\"}]\",\"servicePort\":\"27101\",\"cluster_type\":\"3\",\"agentUuid\":\"17c34028-c338-4072-b0af-076b55ed3569\",\"version\":\"4.4\",\"isTopInstance\":\"1\"}}";
        Copy copy = new Copy();
        copy.setResourceProperties(resourceProperties);
        given(copyRestApi.queryCopyByID(any())).willReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetEnv(new TaskEnvironment());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(DatabaseConstants.VERSION, "4.2");
        given(mongoDBBaseService.getEnvironmentById(any())).willReturn(protectedEnvironment);
        Assert.assertThrows(LegoCheckedException.class,
            () ->  mongoDBRestoreInterceptor.restoreTaskCreationPreCheck(restoreTask));
    }
}
