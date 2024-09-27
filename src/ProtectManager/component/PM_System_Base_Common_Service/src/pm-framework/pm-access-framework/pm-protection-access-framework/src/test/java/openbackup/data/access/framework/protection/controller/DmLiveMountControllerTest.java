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
package openbackup.data.access.framework.protection.controller;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.provider.AbstractLiveMountProvider;
import openbackup.data.access.framework.livemount.provider.mock.TestAbstractLiveMountProvider;
import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.livemount.model.Identity;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
@RunWith(SpringRunner.class)
@PrepareForTest({DmLiveMountController.class, AbstractLiveMountProvider.class, TestAbstractLiveMountProvider.class})
@SpringBootTest(classes = {DmLiveMountController.class, TestAbstractLiveMountProvider.class})
public class DmLiveMountControllerTest {

    @Autowired
    DmLiveMountController dmLiveMountController;

    @Autowired
    TestAbstractLiveMountProvider testAbstractLiveMountProvider;

    @MockBean
    ProviderManager providerManager;

    @MockBean
    protected DeployTypeService deployTypeService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private PerformanceValidator performanceValidator;

    @MockBean
    private EnvironmentRestApi environmentRestApi;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private DeeLiveMountRestApi deeLiveMountRestApi;

    @Test
    public void create_live_mount_pre_check_test_return_false(){
        LiveMountCreateCheckParam liveMountCreateCheckParam = new LiveMountCreateCheckParam();
        liveMountCreateCheckParam.setLiveMountObject(new LiveMountObject());
        liveMountCreateCheckParam.setResource(new CopyResourceSummary());
        liveMountCreateCheckParam.setOperationEnums(OperationEnums.MODIFY);
        liveMountCreateCheckParam.setCopy(new Copy());
        liveMountCreateCheckParam.setTargetResources(new ArrayList<ResourceEntity>());
        Identity<LiveMountCreateCheckParam> identity = new Identity<>("",liveMountCreateCheckParam);
        Mockito.when(providerManager.findProvider(any(),any())).thenReturn(new TestAbstractLiveMountProvider());
    }

    @Test
    public void executeLive_mount_success(){
        LiveMountExecuteParam liveMountExecuteParam = new LiveMountExecuteParam();
        Identity<LiveMountExecuteParam> identity = new Identity<>("",liveMountExecuteParam);
        TestAbstractLiveMountProvider mockProvider = Mockito.mock(TestAbstractLiveMountProvider.class);
        Mockito.when(providerManager.findProvider(any(), any())).thenReturn(mockProvider);
        dmLiveMountController.executeLiveMount(identity);
        Mockito.verify(mockProvider, Mockito.times(1)).executeLiveMount(any());
    }

    @Test
    public void unmount_live_mount_success(){
        LiveMountUnmountParam liveMountUnmountParam = new LiveMountUnmountParam();
        Identity<LiveMountUnmountParam> identity = new Identity<>("",liveMountUnmountParam);
        TestAbstractLiveMountProvider mockProvider = Mockito.mock(TestAbstractLiveMountProvider.class);
        Mockito.when(providerManager.findProvider(any(), any())).thenReturn(mockProvider);
        dmLiveMountController.unmountLiveMount(identity);
        Mockito.verify(mockProvider, Mockito.times(1)).unmountLiveMount(any());
    }

    @Test
    public void clone_copy_success(){
        LiveMountCloneRequest liveMountCloneRequest = new LiveMountCloneRequest();
        Copy copy = new Copy();
        liveMountCloneRequest.setSourceCopy(copy);
        Identity<LiveMountCloneRequest> identity = new Identity<>("",liveMountCloneRequest);
        Mockito.when(providerManager.findProvider(any(),any())).thenReturn(testAbstractLiveMountProvider);
        CopyInfo copyInfo = dmLiveMountController.cloneCopy(identity);
        assertThat(copyInfo).isExactlyInstanceOf(CopyInfo.class);
    }
}
