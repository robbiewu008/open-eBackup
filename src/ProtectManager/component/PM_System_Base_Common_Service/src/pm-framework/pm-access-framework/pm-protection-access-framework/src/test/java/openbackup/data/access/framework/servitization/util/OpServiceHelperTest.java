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
package openbackup.data.access.framework.servitization.util;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OpServiceUtil;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * Op服务化测试用例
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(OpServiceUtil.class)
public class OpServiceHelperTest {
    @InjectMocks
    private OpServiceHelper opServiceHelper;

    @Mock
    private ResourceService resourceService;

    @Mock
    private IVpcService vpcService;

    @Test
    public void test_inject_vpc_info() throws Exception {
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setUuid(UUID.randomUUID().toString());
        vpcInfoEntity.setMarkId("1");
        List<VpcInfoEntity> vpcInfoEntities = Arrays.asList(vpcInfoEntity);
        PowerMockito.when(vpcService.getVpcInfos()).thenReturn(vpcInfoEntities);
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(vpcInfoEntities);
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.FILESET.getType());
        taskResource.setExtendInfo(new HashMap<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectObject(taskResource);
        Endpoint endpoint = new Endpoint();
        endpoint.setId("id");
        backupTask.setAgents(Collections.singletonList(endpoint));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Host");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "0");
        extendInfo.put("vpcId", "vpcId");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optional = Optional.ofNullable(protectedResource);
        PowerMockito.when(resourceService.getBasicResourceById(false, "id")).thenReturn(optional);
        opServiceHelper.injectVpcInfo(backupTask);
    }

    @Test
    public void test_inject_vpc_info_for_restore() throws Exception {
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setUuid(UUID.randomUUID().toString());
        vpcInfoEntity.setMarkId("1");
        List<VpcInfoEntity> vpcInfoEntities = Arrays.asList(vpcInfoEntity);
        PowerMockito.when(vpcService.getVpcInfos()).thenReturn(vpcInfoEntities);
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(vpcInfoEntities);
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.FILESET.getType());
        taskResource.setExtendInfo(new HashMap<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        RestoreTask restoreTask = new RestoreTask();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("id");
        restoreTask.setAgents(Collections.singletonList(endpoint));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Host");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "0");
        extendInfo.put("vpcId", "vpcId");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optional = Optional.ofNullable(protectedResource);
        PowerMockito.when(resourceService.getBasicResourceById(false, "id")).thenReturn(optional);
        opServiceHelper.injectVpcInfoForRestore(restoreTask);
    }

    @Test
    public void test_inject_vpc_info_for_livemount() throws Exception {
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setUuid(UUID.randomUUID().toString());
        vpcInfoEntity.setMarkId("1");
        List<VpcInfoEntity> vpcInfoEntities = Arrays.asList(vpcInfoEntity);
        PowerMockito.when(vpcService.getVpcInfos()).thenReturn(vpcInfoEntities);
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(vpcInfoEntities);
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.FILESET.getType());
        taskResource.setExtendInfo(new HashMap<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        LiveMountCreateTask liveMountCreateTask = new LiveMountCreateTask();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("id");
        liveMountCreateTask.setAgents(Collections.singletonList(endpoint));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Host");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "0");
        extendInfo.put("vpcId", "vpcId");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optional = Optional.ofNullable(protectedResource);
        PowerMockito.when(resourceService.getBasicResourceById(false, "id")).thenReturn(optional);
        opServiceHelper.injectVpcInfoForLiveMount(liveMountCreateTask);
    }

    @Test
    public void test_inject_vpc_info_for_un_livemount() throws Exception {
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setUuid(UUID.randomUUID().toString());
        vpcInfoEntity.setMarkId("1");
        List<VpcInfoEntity> vpcInfoEntities = Arrays.asList(vpcInfoEntity);
        PowerMockito.when(vpcService.getVpcInfos()).thenReturn(vpcInfoEntities);
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(vpcInfoEntities);
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.FILESET.getType());
        taskResource.setExtendInfo(new HashMap<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        LiveMountCancelTask liveMountCancelTask = new LiveMountCancelTask();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("id");
        liveMountCancelTask.setAgents(Collections.singletonList(endpoint));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Host");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "0");
        extendInfo.put("vpcId", "vpcId");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optional = Optional.ofNullable(protectedResource);
        PowerMockito.when(resourceService.getBasicResourceById(false, "id")).thenReturn(optional);
        opServiceHelper.injectVpcInfoForUnLiveMount(liveMountCancelTask);
    }

    @Test
    public void test_inject_vpc_info_for_copyverify_task() throws Exception {
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setUuid(UUID.randomUUID().toString());
        vpcInfoEntity.setMarkId("1");
        List<VpcInfoEntity> vpcInfoEntities = Arrays.asList(vpcInfoEntity);
        PowerMockito.when(vpcService.getVpcInfos()).thenReturn(vpcInfoEntities);
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(vpcInfoEntities);
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.FILESET.getType());
        taskResource.setExtendInfo(new HashMap<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        CopyVerifyTask task = new CopyVerifyTask();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("id");
        task.setAgents(Collections.singletonList(endpoint));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType("Host");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("scenario", "0");
        extendInfo.put("vpcId", "vpcId");
        protectedResource.setExtendInfo(extendInfo);
        Optional<ProtectedResource> optional = Optional.ofNullable(protectedResource);
        PowerMockito.when(resourceService.getBasicResourceById(false, "id")).thenReturn(optional);
        opServiceHelper.injectVpcInfoForCopyVerify(task);
    }
}
