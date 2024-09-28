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
package openbackup.cnware.protection.access.livemount;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述
 *
 */
@Slf4j
@RunWith(MockitoJUnitRunner.class)
public class CnwareLiveMountIntercreptorProviderTest {
    @Mock
    private CopyRestApi mockCopyRestApi;

    @Mock
    private CnwareCommonService cnwareCommonService;

    private CnwareLiveMountIntercreptorProvider cnwareLiveMountIntercreptorProviderTest;
    @Before
    public void setUp() {
        cnwareLiveMountIntercreptorProviderTest =
            new CnwareLiveMountIntercreptorProvider(cnwareCommonService, mockCopyRestApi);
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        Assert.assertTrue(cnwareLiveMountIntercreptorProviderTest.applicable("CNwareVm"));
    }

    /**
     * 用例场景：即时挂载参数准备
     * 前置条件：参数正常
     * 检查点：不报错，参数正确
     */
    @Test
    public void test_init_liveMount_create_task_param_success() {
        when(cnwareCommonService.getEnvironmentById(any())).thenReturn(getEnvironment());
        LiveMountCreateTask task = getLiveMountCreateTask();
        cnwareLiveMountIntercreptorProviderTest.initialize(task);
        Assert.assertEquals(CnwareConstant.SINGLE, task.getTargetEnv().getExtendInfo()
            .get(CnwareConstant.DEPLOY_TYPE));
    }

    /**
     * 用例场景：即时挂载参数准备
     * 前置条件：参数正常
     * 检查点：不报错，参数正确
     */
    @Test
    public void test_init_liveMount_cancel_task_param_success() {
        when(cnwareCommonService.getEnvironmentById(any())).thenReturn(getEnvironment());
        LiveMountCancelTask task = getLiveMountCancelTask();
        cnwareLiveMountIntercreptorProviderTest.finalize(task);
        Assert.assertEquals(CnwareConstant.SINGLE, task.getTargetEnv().getExtendInfo()
            .get(CnwareConstant.DEPLOY_TYPE));
    }

    private LiveMountCreateTask getLiveMountCreateTask() {
        LiveMountCreateTask liveMountCreateTask = new LiveMountCreateTask();
        liveMountCreateTask.setTaskId("test01");
        ProtectedEnvironment protectedEnvironment = getEnvironment();
        liveMountCreateTask.setTargetEnv(BeanTools.copy(protectedEnvironment, TaskEnvironment::new));
        HashMap<String, Object> advanceParams = new HashMap<>();
        List<LiveMountFileSystemShareInfo> fileSystemShareInfoList = new ArrayList<>();
        LiveMountFileSystemShareInfo fileSystemShareInfo = new LiveMountFileSystemShareInfo();
        fileSystemShareInfo.setType(1);
        fileSystemShareInfo.setFileSystemName("test");
        fileSystemShareInfo.setAccessPermission(1);
        fileSystemShareInfo.setAdvanceParams(new HashMap<>());
        fileSystemShareInfoList.add(fileSystemShareInfo);
        advanceParams.put("fileSystemShareInfo", fileSystemShareInfoList);
        advanceParams.put("preScript", "test.sh");
        liveMountCreateTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        taskResource.setSubType("CNwareVm");
        liveMountCreateTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        storageRepository.setExtendInfo(new HashMap<>());
        liveMountCreateTask.setRepositories(Collections.singletonList(storageRepository));
        return liveMountCreateTask;
    }

    private LiveMountCancelTask getLiveMountCancelTask() {
        LiveMountCancelTask liveMountCancelTask = new LiveMountCancelTask();
        HashMap<String, Object> advanceParams = new HashMap<>();
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setType(1);
        advanceParams.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO,
            JSONObject.writeValueAsString(Collections.singletonList(liveMountFileSystemShareInfo)));
        liveMountCancelTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        taskResource.setRootUuid("root_uuid");
        liveMountCancelTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        liveMountCancelTask.setRepositories(Collections.singletonList(storageRepository));
        liveMountCancelTask.getTargetObject().setSubType("CNwareVm");
        return liveMountCancelTask;
    }

    private ProtectedEnvironment getEnvironment() {
        String json = "{\"uuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"name\":\"8-40-160-62\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"path\":\"\",\"createdTime\":\"2023-06-02 22:31:19.0\",\"rootUuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"sourceType\":\"\",\"version\":\"1.5.RC1.027\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.160.62,8.40.160.62,fe80::d143:5c06:d2cf:2287,fe80::41af:571c:a2e4:f061,fe80::9bf9:6e9c:d1a2:f15e,fe80::d094:f372:f7ba:9d2d,fe80::4655:46db:f055:4a5,fe80::5c03:1bd0:4b26:4a95\",\"agentId\":\"1665020602290819074\",\"$citations_agents_aaf26dfe643f40b2acf54cc74cf9d8b6\":\"34a90a3dd9ec48ef9319479ec62a57f8\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"agentUpgradeable\":\"1\",\"agentUpgradeableVersion\":\"1.5.RC1.029\"},\"endpoint\":\"192.168.160.62\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"location\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }





}
