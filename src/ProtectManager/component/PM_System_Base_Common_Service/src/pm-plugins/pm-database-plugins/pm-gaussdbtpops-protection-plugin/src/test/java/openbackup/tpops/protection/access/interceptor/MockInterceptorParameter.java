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
package openbackup.tpops.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GussDB下发参数工具类
 * <p>
 * * @author x30021699
 * * @since 2023-05-09
 */
public class MockInterceptorParameter {
    /**
     * GussDB下发备份参数
     *
     * @return 备份参数
     */
    public static BackupTask getBackupTask() {
        BackupTask backupTask = new BackupTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setEndpoint(new Endpoint());
        repositories.add(storageRepository);
        backupTask.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("aaaaaaaaaaaaaaaaa");
        taskEnvironment.setRootUuid("aaaaaaaaaaaaaaaaa");
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment clusterTaskEnvironment = new TaskEnvironment();
        clusterTaskEnvironment.setUuid("bbbbbbbbbbbbb");
        TaskEnvironment hostTaskEnvironment = new TaskEnvironment();
        hostTaskEnvironment.setUuid("ccccccccccc");
        nodes.add(clusterTaskEnvironment);
        nodes.add(hostTaskEnvironment);
        taskEnvironment.setNodes(nodes);
        taskEnvironment.setExtendInfo(new HashMap<>());
        backupTask.setProtectEnv(taskEnvironment);
        backupTask.setProtectObject(new TaskResource());
        backupTask.getProtectObject().setRootUuid("cccccccc");
        return backupTask;
    }

    /**
     * GussDB下发备份环境参数
     *
     * @return 备份环境参数
     */
    public static ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("aaaaaaaaaaaaaaaaa");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, getProtectedResources("bbbbbbbbbbbbb"));
        resource.setDependencies(dependencies);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        resource.setAuth(authentication);
        resource.setExtendInfo(new HashMap<>());
        return resource;
    }

    private static List<ProtectedResource> getProtectedResources(String uuid) {
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        list.add(protectedResource);
        return list;
    }

    /**
     * GussDB下发备份环境参数
     *
     * @return 备份环境参数
     */
    public static List<ProtectedResource> getProtectedResources2() {
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1111111");
        protectedResource.setEndpoint("10.10.10.10");
        protectedResource.setPort(123);
        list.add(protectedResource);
        ProtectedResource protectedResource2 = new ProtectedResource();
        protectedResource2.setUuid("2222222");
        protectedResource2.setEndpoint("10.10.10.11");
        protectedResource2.setPort(124);
        list.add(protectedResource2);
        return list;
    }

    /**
     * GussDB下发备份环境参数
     *
     * @return 备份环境参数
     */
    public static List<ProtectedResource> getProtectedEnvironment2() {
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("1111111");
        protectedResource.setEndpoint("10.10.10.10");
        protectedResource.setPort(123);
        list.add(protectedResource);
        ProtectedEnvironment protectedResource2 = new ProtectedEnvironment();
        protectedResource2.setUuid("2222222");
        protectedResource2.setEndpoint("10.10.10.11");
        protectedResource2.setPort(124);
        list.add(protectedResource2);
        return list;
    }

    /**
     * GussDB下发备份agent
     *
     * @return 备份agent
     */
    public static List<Endpoint> getEndpointFromProtectedEnvironment2() {
        List<Endpoint> list = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("1111111");
        endpoint.setIp("10.10.10.10");
        endpoint.setPort(123);
        list.add(endpoint);
        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId("2222222");
        endpoint2.setIp("10.10.10.11");
        endpoint2.setPort(124);
        list.add(endpoint2);
        return list;
    }

    /**
     * GussDB下发备份文件系统参数
     *
     * @return 备份文件系统参数
     */
    public static NasDistributionStorageDetail getNasDistributionStorageDetail() {
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setUnitList(new ArrayList<>());
        BackupUnitVo backupUnitVo = new BackupUnitVo();
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setIp("192.168.100.100");
        backupClusterVo.setPort(9527);
        backupClusterVo.setClusterId(10000);
        backupUnitVo.setBackupClusterVo(backupClusterVo);
        nasDistributionStorageDetail.getUnitList().add(backupUnitVo);
        return nasDistributionStorageDetail;
    }
}
