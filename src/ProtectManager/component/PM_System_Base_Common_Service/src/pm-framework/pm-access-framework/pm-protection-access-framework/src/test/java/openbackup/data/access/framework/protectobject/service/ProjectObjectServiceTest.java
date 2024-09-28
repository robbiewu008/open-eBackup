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
package openbackup.data.access.framework.protectobject.service;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.data.access.framework.protectobject.model.ProtectionExecuteCheckReq;
import openbackup.data.access.framework.protectobject.service.ProjectObjectService;
import openbackup.data.access.framework.protectobject.service.impl.ProjectObjectServiceImpl;
import openbackup.data.protection.access.provider.sdk.protection.ProtectObjectProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

/**
 * ProjectObjectService测试类
 *
 */
public class ProjectObjectServiceTest {
    private ProjectObjectService projectObjectService;
    private ProtectObjectProvider protectObjectProvider;

    @Before
    public void init() {
        ResourceService resourceService = Mockito.mock(ResourceService.class, Mockito.RETURNS_DEEP_STUBS);
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);

        final CopyRestApi copyRestApi= Mockito.mock(CopyRestApi.class);

        final SlaQueryService slaQueryService= Mockito.mock(SlaQueryService.class);

        final BackupStorageApi backupStorageApi= Mockito.mock(BackupStorageApi.class);

        final StorageUnitService storageUnitService= Mockito.mock(StorageUnitService.class);

        final ReplicationService replicationService= Mockito.mock(ReplicationService.class);

        projectObjectService = new ProjectObjectServiceImpl(resourceService, providerManager, copyRestApi,
            slaQueryService, backupStorageApi, storageUnitService, replicationService);

        protectObjectProvider = Mockito.mock(ProtectObjectProvider.class);
        Mockito.when(providerManager.findProviderOrDefault(Mockito.any(), Mockito.any(), Mockito.any()))
                .thenReturn(protectObjectProvider);

        List<ProtectedResource> resourceList = Collections.singletonList(new ProtectedResource());
        Mockito.when(resourceService.basicQuery(anyBoolean(), anyInt(), anyInt(), anyMap()).getRecords())
                .thenReturn(resourceList);
    }

    /**
     * 用例名称：保护对象回调检查成功<br/>
     * 前置条件：无<br/>
     * check点：BEFORE CREATE流程调用成功<br/>
     */
    @Test
    public void protect_object_before_create() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = getProtectionExecuteCheckReq();

        protectionExecuteCheckReq.setType(ProtectionExecuteCheckReq.ProtectionPhaseType.BEFORE_CREATE.getValue());
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);

        Mockito.verify(protectObjectProvider,Mockito.times(1)).beforeCreate(any());
    }

    /**
     * 用例名称：保护对象回调检查成功<br/>
     * 前置条件：无<br/>
     * check点：BEFORE UPDATE流程调用成功<br/>
     */
    @Test
    public void protect_object_before_update() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = getProtectionExecuteCheckReq();

        protectionExecuteCheckReq.setType(ProtectionExecuteCheckReq.ProtectionPhaseType.BEFORE_UPDATE.getValue());
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);

        Mockito.verify(protectObjectProvider,Mockito.times(1)).beforeUpdate(any());
    }

    /**
     * 用例名称：保护对象回调检查成功<br/>
     * 前置条件：无<br/>
     * check点：FAILED ON CREATE OR UPDATE流程调用成功<br/>
     */
    @Test
    public void protect_object_failed_on_create_or_update() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = getProtectionExecuteCheckReq();

        protectionExecuteCheckReq.setType(ProtectionExecuteCheckReq.ProtectionPhaseType.FAILED_ON_CREATE_OR_UPDATE.getValue());
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);

        Mockito.verify(protectObjectProvider,Mockito.times(1)).failedOnCreateOrUpdate(any());
    }

    /**
     * 用例名称：保护对象回调检查成功<br/>
     * 前置条件：无<br/>
     * check点：REMOVE流程调用成功<br/>
     */
    @Test
    public void protect_object_remove() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = getProtectionExecuteCheckReq();

        protectionExecuteCheckReq.setType(ProtectionExecuteCheckReq.ProtectionPhaseType.REMOVE.getValue());
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);

        Mockito.verify(protectObjectProvider,Mockito.times(1)).remove(any());
    }

    /**
     * 用例名称：保护对象回调检查成功<br/>
     * 前置条件：无<br/>
     * check点：无效值不触发provider调用<br/>
     */
    @Test
    public void protect_object_invalid() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = getProtectionExecuteCheckReq();

        protectionExecuteCheckReq.setType("aaaaa");
        projectObjectService.checkProtectObject(protectionExecuteCheckReq);

        Mockito.verify(protectObjectProvider,Mockito.times(0)).beforeCreate(any());
        Mockito.verify(protectObjectProvider,Mockito.times(0)).beforeUpdate(any());
        Mockito.verify(protectObjectProvider,Mockito.times(0)).failedOnCreateOrUpdate(any());
        Mockito.verify(protectObjectProvider,Mockito.times(0)).remove(any());
    }

    private ProtectionExecuteCheckReq getProtectionExecuteCheckReq() {
        ProtectionExecuteCheckReq protectionExecuteCheckReq = new ProtectionExecuteCheckReq();
        protectionExecuteCheckReq.setResourceIds(Collections.singletonList("1"));
        return protectionExecuteCheckReq;
    }
}
