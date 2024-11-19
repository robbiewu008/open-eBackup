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

import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.VolInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 资源相关操作管理器
 *
 */
@Slf4j
@Component
public class OpenStackResourceManager {
    private static final int PAGE_SIZE = 50;
    private static final String KEY_PARENT_UUID = "parentUuid";
    private static final String KEY_STATUS = "status";
    private static final String KEY_SUB_TYPE = "subType";
    private static final String KEY_PROTECTED_OBJECT = "protectedObject";

    private final ResourceService resourceService;

    public OpenStackResourceManager(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 查询资源
     *
     * @param resourceId 资源id
     * @return {@link ProtectedResource} 资源信息
     */
    public Optional<ProtectedResource> queryResourceById(String resourceId) {
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldIgnoreOwner(true);
        params.setShouldLoadEnvironment(false);
        params.setConditions(Collections.singletonMap("uuid", resourceId));
        params.setSize(1);
        PageListResponse<ProtectedResource> result = resourceService.query(params);
        if (result.getRecords().isEmpty()) {
            return Optional.empty();
        }
        return Optional.ofNullable(result.getRecords().get(0));
    }

    /**
     * 更新资源保护状态
     *
     * @param resourceId 资源id
     * @param protectionStatus 保护状态
     */
    public void updateProtectionStatus(String resourceId, int protectionStatus) {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(resourceId);
        resource.setProtectionStatus(protectionStatus);
        resourceService.update(new ProtectedResource[] {resource});
    }

    /**
     * 根据父资源id查询资源
     *
     * @param parentUuid 父资源id
     * @param isNeedResourceProtected 是否只查询已保护的资源
     * @return 资源列表
     */
    public List<ProtectedResource> queryResourcesByProjectId(String parentUuid, boolean isNeedResourceProtected) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(KEY_PARENT_UUID, parentUuid);
        if (isNeedResourceProtected) {
            // 级联查询保护对象表status为protected（已保护已激活）或unprotected（已保护未激活）的数据
            JSONObject cascadedFilter = new JSONObject();
            cascadedFilter.put(
                KEY_STATUS,
                Arrays.asList(ProtectionStatusEnum.UNPROTECTED.getType(), ProtectionStatusEnum.PROTECTED.getType()));
            conditions.put(KEY_PROTECTED_OBJECT, cascadedFilter);
        }

        ResourceQueryParams queryParams = new ResourceQueryParams();
        queryParams.setSize(PAGE_SIZE);
        queryParams.setConditions(conditions);
        queryParams.setShouldIgnoreOwner(true);
        queryParams.setShouldLoadEnvironment(false);

        List<ProtectedResource> resources = new ArrayList<>();
        PageListResponse<ProtectedResource> result;
        int pageNo = 0;
        do {
            queryParams.setPage(pageNo);
            result = resourceService.query(queryParams);
            resources.addAll(result.getRecords());
            pageNo++;
        } while (result.getRecords().size() == PAGE_SIZE);
        return resources;
    }

    /**
     * 根据卷id查询资源
     *
     * @param volumeId 卷id
     * @return {@link ProtectedResource} 资源
     */
    public Optional<ProtectedResource> queryResourceByVolumeId(String volumeId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(KEY_SUB_TYPE, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());

        ResourceQueryParams queryParams = new ResourceQueryParams();
        queryParams.setSize(PAGE_SIZE);
        queryParams.setConditions(conditions);
        queryParams.setShouldIgnoreOwner(true);

        int pageNo = 0;
        PageListResponse<ProtectedResource> resources;
        do {
            queryParams.setPage(pageNo);
            resources = resourceService.query(queryParams);
            Optional<ProtectedResource> result =
                    resources.getRecords().stream().filter(resource -> containVolume(resource, volumeId)).findFirst();
            if (result.isPresent()) {
                return result;
            }
            pageNo++;
        } while (resources.getRecords().size() == PAGE_SIZE);
        return Optional.empty();
    }

    private boolean containVolume(ProtectedResource resource, String volumeId) {
        Map<String, String> extendInfo = resource.getExtendInfo();
        if (extendInfo == null || !extendInfo.containsKey(OpenstackConstant.VOLUME_INFO_KEY)) {
            return false;
        }
        String volInfo = extendInfo.get(OpenstackConstant.VOLUME_INFO_KEY);
        List<VolInfo> volInfos = JSONArray.fromObject(volInfo).toBean(VolInfo.class);

        return volInfos.stream().anyMatch(volume -> Objects.equals(volume.getId(), volumeId));
    }
}
