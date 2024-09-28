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
package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.VolInfo;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.IntStream;

/**
 * {@link OpenStackResourceManager} 测试类
 *
 */
public class OpenStackResourceManagerTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final OpenStackResourceManager resourceManager = new OpenStackResourceManager(resourceService);

    /**
     * 用例场景：查询资源返回正确资源个数
     * 前置条件：无
     * 检查点：  能够正确返回资源个数
     */
    @Test
    public void should_returnCorrectSize_when_queryResourcesByProjectId() {
        List<ProtectedResource> firstResources = new ArrayList<>();
        List<ProtectedResource> secondResources = new ArrayList<>();
        IntStream.range(0, 50).forEach(i -> firstResources.add(new ProtectedResource()));
        IntStream.range(0, 20).forEach(i -> secondResources.add(new ProtectedResource()));
        PageListResponse<ProtectedResource> firstResp = new PageListResponse<>(70, firstResources);
        PageListResponse<ProtectedResource> secondResp = new PageListResponse<>(70, secondResources);
        Mockito.when(resourceService.query(any())).thenReturn(firstResp).thenReturn(secondResp);
        List<ProtectedResource> result = resourceManager.queryResourcesByProjectId(UUIDGenerator.getUUID(), true);
        assertThat(result.size()).isEqualTo(70);
        Mockito.verify(resourceService, Mockito.times(2)).query(any());
    }

    /**
     * 用例场景：根据卷id查询资源返回成功
     * 前置条件：无
     * 检查点：  能够正确返回资源
     */
    @Test
    public void should_returnResource_when_queryResourceByVolumeId_given_existVolumeId() {
        List<ProtectedResource> firstResources = new ArrayList<>();
        List<ProtectedResource> secondResources = new ArrayList<>();
        IntStream.range(0, 50).forEach(i -> firstResources.add(new ProtectedResource()));
        IntStream.range(0, 1).forEach(i -> secondResources.add(new ProtectedResource()));

        PageListResponse<ProtectedResource> firstResponse = new PageListResponse<>();
        PageListResponse<ProtectedResource> secondResponse = new PageListResponse<>();

        Map<String, String> extendInfo = new HashMap<>();
        VolInfo volInfo = new VolInfo();
        String volumeId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        volInfo.setId(volumeId);
        List<VolInfo> volInfos = Collections.singletonList(volInfo);
        extendInfo.put(OpenstackConstant.VOLUME_INFO_KEY, JSONArray.fromObject(volInfos).toString());
        secondResources.get(0).setUuid(resourceId);
        secondResources.get(0).setExtendInfo(extendInfo);

        Mockito.when(resourceService.query(any()))
            .thenReturn(new PageListResponse<>(51, firstResources))
            .thenReturn(new PageListResponse<>(51, secondResources));

        Optional<ProtectedResource> result = resourceManager.queryResourceByVolumeId(volumeId);
        assertThat(result).isPresent();
        assertThat(result.get().getUuid()).isEqualTo(resourceId);
    }

    /**
     * 用例场景：根据卷id查询资源返回成功
     * 前置条件：无
     * 检查点：  能够正确返回资源
     */
    @Test
    public void should_returnEmpty_when_queryResourceByVolumeId_given_nonExistVolumeId() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        IntStream.range(0, 3).forEach(i -> resources.add(new ProtectedResource()));
        String volumeId = UUIDGenerator.getUUID();
        resources.get(0).setExtendInfo(Collections.emptyMap());
        response.setTotalCount(3);
        response.setRecords(resources);

        Mockito.when(resourceService.query(any())).thenReturn(response);
        Optional<ProtectedResource> result = resourceManager.queryResourceByVolumeId(volumeId);
        assertThat(result).isEmpty();
    }
}
