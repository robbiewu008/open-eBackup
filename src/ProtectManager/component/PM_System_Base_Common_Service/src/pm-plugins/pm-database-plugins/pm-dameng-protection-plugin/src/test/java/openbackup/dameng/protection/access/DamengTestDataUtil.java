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
package openbackup.dameng.protection.access;

import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 提供dameng测试类所需的数据
 *
 */
public class DamengTestDataUtil {
    public static ProtectedEnvironment buildProtectedEnvironment(String uuid, String subType) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(uuid);
        environment.setName("Dameng");
        environment.setType(ResourceTypeEnum.DATABASE.getType());
        environment.setSubType(subType);
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8080);
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Authentication auth = new Authentication();
        auth.setAuthType(0);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.NODES, JSONObject.writeValueAsString(buildNodeInfo()));
        extendInfo.put(DamengConstant.VERSION, "V8");
        environment.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, getSubProtectedResourceList());
        environment.setDependencies(dependencies);
        return environment;
    }

    public static List<NodeInfo> buildNodeInfo() {
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setUuid("uuid");
        nodeInfo.setName("localhost");
        nodeInfo.setEndpoint("127.0.0.1");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.INSTANCESTATUS, "1");
        extendInfo.put(DatabaseConstants.PORT, "8080");
        extendInfo.put(DamengConstant.AUTH_TYPE, "0");
        extendInfo.put(DamengConstant.ROLE, DamengConstant.PRIMARY);
        nodeInfo.setExtendInfo(extendInfo);
        return Collections.singletonList(nodeInfo);
    }

    public static List<ProtectedResource> getSubProtectedResourceList() {
        List<ProtectedResource> subProtectedResourceList = new ArrayList<>();
        ProtectedResource subProtectedResource = new ProtectedResource();
        subProtectedResource.setUuid("uuid");

        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8080);
        Authentication auth = new Authentication();
        auth.setAuthKey("SYS");
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.NODES, JSONObject.writeValueAsString(buildNodeInfo()));
        environment.setExtendInfo(extendInfo);
        subProtectedResource.setEnvironment(environment);

        Map<String, String> resourceExtendInfo = new HashMap<>();
        resourceExtendInfo.put(DatabaseConstants.PORT, "8080");
        subProtectedResource.setExtendInfo(resourceExtendInfo);
        subProtectedResource.setAuth(auth);

        Map<String, List<ProtectedResource>> agentDependencies = new HashMap<>();
        agentDependencies.put(DatabaseConstants.AGENTS, getAgentResource());
        subProtectedResource.setDependencies(agentDependencies);
        subProtectedResourceList.add(subProtectedResource);
        return subProtectedResourceList;
    }

    public static BackupTask buildBackupTask(String backupType, String subType) {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(backupType);
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());
        repositories.add(dataStorageRepository);
        backupTask.setRepositories(repositories);

        TaskResource objectTask = new TaskResource();
        objectTask.setSubType(subType);
        objectTask.setUuid("aaa");
        backupTask.setProtectObject(objectTask);
        backupTask.setAdvanceParams(new HashMap<>());

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        taskEnvironment.setExtendInfo(extendInfo);
        taskEnvironment.setNodes(getNodes());
        backupTask.setProtectEnv(taskEnvironment);
        return backupTask;
    }

    public static RestoreTask buildRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        restoreTask.setTargetEnv(taskEnvironment);

        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parentUuid");
        restoreTask.setTargetObject(taskResource);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(DamengConstant.DB_PATH, "/dbPath");
        advanceParams.put(DamengConstant.DB_NAME, "dbName");
        restoreTask.setAdvanceParams(advanceParams);
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        return restoreTask;
    }

    private static List<TaskEnvironment> getNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setEndpoint("127.0.0.1");
        taskEnvironment.setPort(8080);
        return Collections.singletonList(taskEnvironment);
    }

    private static List<ProtectedResource> getAgentResource() {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8080);
        resource.setEnvironment(environment);
        protectedResourceList.add(resource);
        return protectedResourceList;
    }
}
