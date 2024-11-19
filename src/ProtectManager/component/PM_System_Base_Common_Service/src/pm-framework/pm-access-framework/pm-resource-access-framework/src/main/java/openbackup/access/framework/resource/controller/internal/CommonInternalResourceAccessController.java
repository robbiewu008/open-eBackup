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
package openbackup.access.framework.resource.controller.internal;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.dto.InternalResourceQueryParam;
import openbackup.access.framework.resource.service.ProtectedResourceEvent;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.FileSystemInfo;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.StorageInfo;
import openbackup.data.protection.access.provider.sdk.resource.TenantInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.Map;

import javax.validation.Valid;

/**
 * 通用资源接入REST API控制器，提供受保护资源相关REST接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/internal/resources")
public class CommonInternalResourceAccessController {
    private static final int MAX_SIZE = 200;

    private final ResourceService resourceService;
    private final ProtectedResourceMonitorService protectedResourceMonitorService;

    private final CyberEngineResourceService cyberEngineResourceService;

    /**
     * constructor
     *
     * @param resourceService resource service
     * @param cyberEngineResourceService cyberEngineResourceService
     * @param protectedResourceMonitorService protectedResourceMonitorService
     */
    public CommonInternalResourceAccessController(ResourceService resourceService,
        CyberEngineResourceService cyberEngineResourceService,
        ProtectedResourceMonitorService protectedResourceMonitorService) {
        this.resourceService = resourceService;
        this.cyberEngineResourceService = cyberEngineResourceService;
        this.protectedResourceMonitorService = protectedResourceMonitorService;
    }

    /**
     * page query protected resource
     *
     * @param queryParam query param
     * @return PageListResponse
     */
    @ExterAttack
    @GetMapping
    public PageListResponse<ProtectedResource> queryResources(
        @ModelAttribute
        @Valid InternalResourceQueryParam queryParam
    ) {
        Map<String, Object> map = JSONObject.fromObject(queryParam.getConditions()).toMap(Object.class);
        ResourceQueryParams context = new ResourceQueryParams();
        context.setPage(Math.max(queryParam.getPageNo(), 0));
        context.setSize(Math.max(Math.min(queryParam.getPageSize(), MAX_SIZE), 0));
        context.setConditions(map);
        context.setOrders(queryParam.getOrders());
        context.setShouldQueryDependency(queryParam.isShouldQueryDependency());
        PageListResponse<ProtectedResource> response = resourceService.query(context);
        response.getRecords().forEach(this::desensitize);
        return response;
    }

    /**
     * query resource by id
     *
     * @param resourceId resource id
     * @return protected resource
     */
    @ExterAttack
    @GetMapping("/{resourceId}")
    public ProtectedResource queryResource(@PathVariable("resourceId") String resourceId) {
        ProtectedResource resource =
                resourceService
                        .getResourceById(false, resourceId)
                        .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        desensitize(resource);
        return resource;
    }

    /**
     * 查询租户列表
     *
     * @param pageNo 页数
     * @param pageSize 页面大小
     * @param resourceId 文件系统ID
     * @return 租户列表
     */
    @ExterAttack
    @GetMapping("tenants")
    public PageListResponse<TenantInfo> listTenants(
        @RequestParam(value = "pageNo", defaultValue = "0", required = false) int pageNo,
        @RequestParam(value = "pageSize", defaultValue = "10", required = false) int pageSize,
        @RequestParam(value = "resourceId", defaultValue = "", required = false) String resourceId) {
        // 1、resourceId为空，获取所有租户列表
        // 2、resourceId不为空，获取当前文件系统的resourceId
        return StringUtils.isBlank(resourceId)
            ? cyberEngineResourceService.listAllTenants(pageNo, pageSize)
            : cyberEngineResourceService.listTenantByResourceId(pageNo, pageSize, resourceId);
    }

    /**
     * 查询用户域下所有资源id集合
     *
     * @param userId 用户id
     * @return 租户列表
     */
    @ExterAttack
    @GetMapping("/{userId}/user-domain")
    public List<String> getDomainResourceIdListByUserId(@PathVariable(value = "userId") String userId) {
        return resourceService.getDomainResourceIdListByUserId(userId);
    }

    /**
     * 查询存储设备信息
     *
     * @param tenantId 租户ID
     * @param deviceId 设备ID
     * @return 租户列表
     */
    @ExterAttack
    @GetMapping("storageInfo")
    public StorageInfo listStorageInfo(@RequestParam(value = "tenantId", required = true) String tenantId,
        @RequestParam(value = "deviceId", required = true) String deviceId) {
        return cyberEngineResourceService.listStorageInfo(deviceId, tenantId);
    }

    /**
     * 查询文件系统信息
     *
     * @param pageNo 页码
     * @param pageSize 页面大小
     * @param tenantId 租户Id
     * @param resourceId 文件系统ID
     * @return 文件系统列表
     */
    @ExterAttack
    @GetMapping("fileSystems")
    public PageListResponse<FileSystemInfo> listFileSystems(
        @RequestParam(value = "pageNo", defaultValue = "0", required = false) int pageNo,
        @RequestParam(value = "pageSize", defaultValue = "10", required = false) int pageSize,
        @RequestParam(value = "tenantId", required = false) String tenantId,
        @RequestParam(value = "resourceId", required = false) String resourceId) {
        return cyberEngineResourceService.listFileSystems(pageNo, pageSize, tenantId, resourceId);
    }

    private void desensitize(ProtectedResource resource) {
        protectedResourceMonitorService.invoke("desensitize", resource, ProtectedResourceEvent::getResource);
    }

    /**
     * 清除下次备份类型和原因
     *
     * @param resourceId 资源id
     */
    @ExterAttack
    @PutMapping("{resourceId}/action/clean-next-backup")
    public void cleanNextBackup(@PathVariable("resourceId") String resourceId) {
        resourceService.cleanNextBackup(resourceId);
    }

    /**
     * 查询下次备份类型和原因
     *
     * @param resourceId 资源Id
     * @return Map<String,String> map
     */
    @ExterAttack
    @GetMapping("{resourceId}/next-backup-type-and-cause")
    public NextBackupParams queryNextBackupTypeAndCause(@PathVariable("resourceId") String resourceId) {
        return resourceService.queryNextBackupTypeAndCause(resourceId);
    }
}
