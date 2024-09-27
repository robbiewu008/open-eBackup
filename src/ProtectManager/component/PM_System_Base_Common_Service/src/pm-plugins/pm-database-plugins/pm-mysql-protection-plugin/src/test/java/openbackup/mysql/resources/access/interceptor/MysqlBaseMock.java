/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.interceptor;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * mysql llt 基本参数mock类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/6/23
 */
public class MysqlBaseMock {
    public static ProtectedResource mockGetGrantParentResource(RestoreTask task, String type, MysqlBaseService mysqlBaseService) {
        ProtectedResource singleInstanceRes = new ProtectedResource();
        singleInstanceRes.setParentUuid(UUID.randomUUID().toString());
        singleInstanceRes.setExtendInfo(new HashMap<>());
        singleInstanceRes.setUuid(UUID.randomUUID().toString());
        singleInstanceRes.getExtendInfo().put(DatabaseConstants.CLUSTER_TYPE, "PXC");
        PowerMockito.when(mysqlBaseService.getResource(task.getTargetObject().getParentUuid())).thenReturn(singleInstanceRes);
        ProtectedResource grantParentResource;
        if ("0".equals(type)) {
            grantParentResource = new ProtectedEnvironment();
        } else {
            grantParentResource = new ProtectedResource();
        }
        grantParentResource.setUuid(singleInstanceRes.getParentUuid());
        grantParentResource.setParentUuid(UUID.randomUUID().toString());
        PowerMockito.when(mysqlBaseService.getResource(singleInstanceRes.getParentUuid())).thenReturn(grantParentResource);
        return singleInstanceRes;
    }

    public static void mockGetAgentBySingleInstanceRes(ProtectedResource singleInstanceRes, MysqlBaseService mysqlBaseService) {
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        agentEnv.setPort(8081);
        agentEnv.setEndpoint("8.40.99.101");
        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceRes.getUuid())).thenReturn(agentEnv);
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentEnv.getEndpoint());
        endpoint.setPort(agentEnv.getPort());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(agentEnv)).thenReturn(endpoint);
        HostDto hostDto = new HostDto();
        hostDto.setEndpoint(agentEnv.getEndpoint());
        hostDto.setPort(agentEnv.getPort());
    }

    public static RestoreTask getDatabaseRestoreTask(ResourceSubTypeEnum subTypeEnum) {
        RestoreTask task = new RestoreTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositories.add(storageRepository);
        task.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setNodes(new ArrayList<>());
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        task.setTargetEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setParentUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subTypeEnum.getType());
        final Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.DATA_DIR, "123");
        extendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "Centos");
        taskResource.setExtendInfo(extendInfo);
        task.setTargetObject(taskResource);
        task.setAgents(new ArrayList<>());
        return task;
    }

    public static Copy getCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        copy.setResourceProperties("{\"name\":\"MyCluster01\",\"path\":null,\"root_uuid\":\"99031776d0944ed6826612c41540f837\",\"parent_name\":null,\"parent_uuid\":\"99031776d0944ed6826612c41540f837\",\"children_uuids\":null,\"type\":\"Database\",\"sub_type\":\"MySQL-clusterInstance\",\"uuid\":\"e285e4478927475b975b3170f5245337\",\"created_time\":\"2022-10-06T09:40:27.882000\",\"ext_parameters\":{\"pre_script\":null,\"post_script\":null,\"failed_script\":null},\"authorized_user\":null,\"user_id\":null,\"version\":\"5.5.5-10.2.43-MariaDB-log\",\"sla_id\":\"0d002c8e-eef8-47d9-8f77-ca21b5151856\",\"sla_name\":\"MySQL_restore_0013\",\"sla_status\":true,\"sla_compliance\":true,\"protection_status\":1,\"environment_uuid\":\"99031776d0944ed6826612c41540f837\",\"environment_name\":\"MyCluster01\",\"environment_endpoint\":\"192.168.162.242\",\"environment_os_type\":null,\"environment_type\":\"Database\",\"environment_sub_type\":\"MySQL-cluster\",\"environment_is_cluster\":\"False\",\"environment_os_name\":null,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterType\":\"AP\",\"isTopInstance\":\"1\"}}");
        copy.setResourceSubType(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        return copy;
    }
}
