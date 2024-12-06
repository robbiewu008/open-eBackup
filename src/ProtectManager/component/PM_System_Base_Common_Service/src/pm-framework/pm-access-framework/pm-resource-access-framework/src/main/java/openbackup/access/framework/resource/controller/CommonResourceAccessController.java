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
package openbackup.access.framework.resource.controller;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.lock.ResourceDistributedLockService;
import openbackup.access.framework.resource.util.ResourceUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.common.model.AllowRestoreObject;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.resource.model.UpdateCopyUserObjectReq;
import openbackup.system.base.sdk.resource.model.UpdateRestoreObjectReq;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import org.apache.commons.lang3.StringUtils;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;

import javax.validation.Valid;

/**
 * 通用资源接入REST API控制器，提供受保护资源相关REST接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/resources")
public class CommonResourceAccessController {
    private static final int MAX_SIZE = 200;

    private final ResourceService resourceService;

    private final SessionService sessionService;

    private final ProviderManager providerManager;

    private final ResourceDistributedLockService distributedLockService;

    private final ResourceExtendInfoService resourceExtendInfoService;

    /**
     * constructor
     *
     * @param resourceService resource service
     * @param sessionService sessionService
     * @param providerManager providerManager
     * @param distributedLockService distributedLockService
     * @param resourceExtendInfoService resourceExtendInfoService
     */
    public CommonResourceAccessController(ResourceService resourceService, SessionService sessionService,
        ProviderManager providerManager, ResourceDistributedLockService distributedLockService,
        ResourceExtendInfoService resourceExtendInfoService) {
        this.resourceService = resourceService;
        this.sessionService = sessionService;
        this.providerManager = providerManager;
        this.distributedLockService = distributedLockService;
        this.resourceExtendInfoService = resourceExtendInfoService;
    }

    /**
     * create resources
     *
     * @param resource resource
     * @return resource uuid list
     */
    @ExterAttack
    @PostMapping(consumes = MediaType.APPLICATION_JSON_VALUE, value = "")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.CREATE,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE})
    @Logging(name = "0x20640332002F", target = "Resource",
        details = {"$1.name", "=resource_sub_type_#{#snake($1.subType)}_label"})
    public UuidObject createResource(@Valid @RequestBody ProtectedResource resource) {
        checkDuplicatedName(resource, Collections.emptyMap());
        ResourceUtil.supplySourceTypeWhenUuidNull(resource, ResourceConstants.SOURCE_TYPE_REGISTER);
        String lockKey = distributedLockService.getResourceLockKey(ResourceConstants.UPDATE_RESOURCE_LOCK, resource);
        String[] uuids = (String[]) distributedLockService.tryLockAndGet(lockKey, resource, (res) ->
            resourceService.create(new ProtectedResource[]{res}, true));
        return new UuidObject(uuids[0]);
    }

    private void checkDuplicatedName(ProtectedResource resource, Map<String, Object> filter) {
        ResourceProvider resourceProvider = providerManager.findProvider(ResourceProvider.class, resource, null);
        if (resourceProvider != null && !resourceProvider.getResourceFeature().isShouldCheckResourceNameDuplicate()) {
            return;
        }
        JSONObject json = new JSONObject().set("name", resource.getName()).set("subType", resource.getSubType());
        Map<String, Object> condition = json.toMap(Object.class);
        condition.putAll(filter);
        PageListResponse<ProtectedResource> response = sessionService.call(() -> resourceService.query(0, 1, condition),
            Constants.Builtin.ROLE_SYS_ADMIN);
        long count = response.getRecords().size();
        if (count > 0) {
            throw new LegoCheckedException(CommonErrorCode.DUPLICATE_NAME, "Duplicate resource name exists");
        }
    }

    /**
     * 检查资源连通性
     *
     * @param resource 资源
     * @return 检查结果
     */
    @ExterAttack
    @PostMapping("/action/check")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "resource:$1.uuid",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resource.uuid")
    @Logging(name = "0x20640332003A", target = "Resource",
        details = {"=resource_sub_type_#{#snake($1.subType)}_label", "$1.name"})
    public ActionResult[] checkResourceConnectivity(@Valid @RequestBody ProtectedResource resource) {
        return resourceService.check(resource, true);
    }

    /**
     * update resources
     *
     * @param resourceId resource id
     * @param resource resource
     */
    @ExterAttack
    @PutMapping("/{resourceId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resourceId")
    @Logging(name = "0x206403320030", target = "Resource",
        details = {"$1", "$resource.name", "=resource_sub_type_#{#snake($resource.subType)}_label"},
        context = @Context(name = "resource", statement = "@resource_dao_select_by_id.call($1)"))
    public void updateResource(@PathVariable("resourceId") String resourceId,
        @RequestBody @Valid ProtectedResource resource) {
        if (resource == null) {
            return;
        }
        ProtectedResource content = new ProtectedResource();
        content.setUuid(resourceId);
        content.setName(resource.getName());
        content.setAuth(resource.getAuth());
        content.setExtendInfo(resource.getExtendInfo());
        List<ProtectedResource> oldResources = resourceService.query(0, 1, Collections.singletonMap("uuid", resourceId))
            .getRecords();
        if (oldResources.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        if (oldResources.size() > 1) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        }
        ProtectedResource oldResource = oldResources.get(0);
        content.setSubType(oldResource.getSubType());

        if (content.getName() != null && !Objects.equals(oldResource.getName(), content.getName())) {
            checkDuplicatedName(content,
                Collections.singletonMap("uuid", Arrays.asList(Collections.singletonList("!="), resourceId)));
        }
        content.setDependencies(resource.getDependencies());
        ResourceUtil.supplySourceTypeWhenUuidNull(content, ResourceConstants.SOURCE_TYPE_REGISTER);
        String lockKey = distributedLockService.getResourceLockKey(ResourceConstants.UPDATE_RESOURCE_LOCK, oldResource);
        distributedLockService.tryLockAndRun(lockKey, content, (res) ->
            resourceService.update(new ProtectedResource[]{res}));
    }

    /**
     * page query protected resource
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @param shouldQueryDependency shouldQueryDependency
     * @return protected resource page data
     */
    @ExterAttack
    @GetMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, enableCheckAuth = false)
    public PageListResponse<ProtectedResource> queryResources(
        @RequestParam(value = "pageNo", defaultValue = "0", required = false) int page,
        @RequestParam(value = "pageSize", defaultValue = "10", required = false) int size,
        @RequestParam(value = "conditions", defaultValue = "{}", required = false) String conditions,
        @RequestParam(value = "queryDependency", defaultValue = "false", required = false)
        boolean shouldQueryDependency,
        @RequestParam(value = "orders", defaultValue = "[]", required = false) String... orders) {
        Map<String, Object> map = JSONObject.fromObject(conditions).toMap(Object.class);
        ResourceQueryParams context = new ResourceQueryParams();
        context.setPage(Math.max(page, 0));
        context.setSize(Math.max(Math.min(size, MAX_SIZE), 0));
        context.setConditions(map);
        context.setOrders(orders);
        context.setShouldQueryDependency(shouldQueryDependency);
        String isDesesitizationKey = "isDesesitization";
        context.setDesesitization(Boolean.parseBoolean(map.getOrDefault(isDesesitizationKey, "false").toString()));
        map.remove(isDesesitizationKey);
        PageListResponse<ProtectedResource> response = resourceService.query(context);
        response.getRecords().forEach(resourceService::desensitize);
        response.getRecords().forEach(resourceService::appendGroupInfo);
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
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.QUERY,
        authOperations = AuthOperationEnum.MANAGE_RESOURCE, target = "#resourceId")
    public ProtectedResource queryResource(@PathVariable("resourceId") String resourceId) {
        ProtectedResource resource = resourceService.getResourceById(false, resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        resourceService.desensitize(resource);
        return resource;
    }

    /**
     * delete resource
     *
     * @param resourceId resource id
     */
    @ExterAttack
    @DeleteMapping("/{resourceId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.DELETE,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resourceId")
    @Logging(name = "0x206403320031", target = "Resource", requires = "$resource",
        details = {"$1", "$resource?.name", "=resource_sub_type_#{#snake($resource?.subType)}_label"},
        context = @Context(name = "resource", statement = "@resource_dao_select_by_id.call($1)"))
    public void deleteResource(@PathVariable("resourceId") String resourceId) {
        Optional<ProtectedResource> resourceOptional = resourceService.getBasicResourceById(false, resourceId);
        if (!resourceOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        ProtectedResource resource = resourceOptional.get();
        if ("autoscan".equals(resource.getSourceType())) {
            throw new LegoCheckedException("Resources which source type is autoscan cannot be deleted.");
        }
        resourceService.delete(new String[] {resourceId});
    }

    /**
     * 检查受保护资源的连通性
     *
     * @param resourceId 资源ID
     * @return 检查结果
     */
    @ExterAttack
    @GetMapping("/{resourceId}/action/check")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resourceId")
    @Logging(name = "0x20640332003A", target = "Resource", requires = "$resource",
        details = {"=resource_sub_type_#{#snake($resource?.subType)}_label", "$resource?.name"},
        context = @Context(name = "resource", statement = "@resource_dao_select_by_id.call($1)"))
    public ActionResult[] checkProtectedResourceConnectivity(@PathVariable("resourceId") String resourceId) {
        log.info("Check connection start, resourceId is: {}.", resourceId);
        ProtectedResource protectedResource = resourceService.getResourceById(resourceId)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "The protected resource not exists."));
        return resourceService.check(protectedResource, true);
    }

    /**
     * scan protected resource
     *
     * @param resId resource id
     */
    @ExterAttack
    @PutMapping("/{resId}/action/scan")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_CLIENT, AuthOperationEnum.MANAGE_RESOURCE}, target = "#resId")
    @Logging(name = "0x206403320032", target = "Resource",
        details = {"$1", "$resource?.name", "=resource_sub_type_#{#snake($resource?.subType)}_label"},
        context = {@Context(name = "resource", statement =
            "@scan_resource_log_utils_parse_resource_log_param.call($1)")})
    public void scanProtectedResource(@PathVariable("resId") String resId) {
        resourceService.getBasicResourceById(resId)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "resource is not exist"));
        if (resourceService.checkEnvScanTaskIsRunning(resId)) {
            throw new LegoCheckedException(CommonErrorCode.EXIST_SAME_TYPE_JOB_IN_RUNNING, "same scan job is running");
        }
        resourceService.createProtectedResourceScanTask(resId, sessionService.getCurrentUser().getId());
    }

    /**
     * query resource by id
     *
     * @param resourceIds resourceIds
     * @return protected resource
     */
    @ExterAttack
    @GetMapping("/allow-restore")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.QUERY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resourceIds")
    public List<AllowRestoreObject> queryResourceAllowRestore(@RequestParam("resourceIds") String resourceIds) {
        ArrayList<AllowRestoreObject> allowRestoreObjects = new ArrayList<>();
        String[] resourceIdBatch = resourceIds.split(",");
        for (String resourceId : resourceIdBatch) {
            // 当前资源及其父子资源都支持恢复时，允许此次恢复
            String isAllowRestoreFlag = resourceService.judgeResourceRestoreLevel(resourceId);
            AllowRestoreObject allowRestoreObject = new AllowRestoreObject(resourceId, isAllowRestoreFlag);
            allowRestoreObjects.add(allowRestoreObject);
        }
        return allowRestoreObjects;
    }

    /**
     * update resources
     *
     * @param updateRestoreObjectReq updateRestoreObjectReq
     */
    @ExterAttack
    @PutMapping("/allow-restore")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#updateRestoreObjectReq.resourceIds")
    public void updateResourceAllowRestoreFlag(@RequestBody UpdateRestoreObjectReq updateRestoreObjectReq) {
        String isAllowRestore = updateRestoreObjectReq.getIsAllowRestore();
        List<String> resourceIds = updateRestoreObjectReq.getResourceIds();
        log.info("Start to update resource, allowRestoreFlag is {}", isAllowRestore);
        for (String resourceId : resourceIds) {
            // 如果是设置允许恢复，需要校验父资源是否支持
            if (StringUtils.equals(isAllowRestore, ResourceConstants.ALLOW_RESTORE)) {
                resourceService.verifyParentResourceRestoreLevel(resourceId);
            }
            resourceExtendInfoService.saveOrUpdateExtendInfo(resourceId,
                Collections.singletonMap(ResourceConstants.IS_ALLOW_RESTORE_KEY, isAllowRestore));

            // 如果是设置禁止恢复，需要同步将子资源也设置为不可恢复
            if (StringUtils.equals(isAllowRestore, ResourceConstants.NOT_ALLOW_RESTORE)) {
                Set<String> relatedResourceUuids = resourceService.queryRelatedResourceUuids(resourceId,
                    new String[] {});
                for (String relatedResourceUuid : relatedResourceUuids) {
                    resourceExtendInfoService.saveOrUpdateExtendInfo(relatedResourceUuid,
                        Collections.singletonMap(ResourceConstants.IS_ALLOW_RESTORE_KEY, isAllowRestore));
                }
            }
        }
    }

    /**
     * 修改副本归属用户（同时修改配额）
     *
     * @param updateCopyUserObjectReq 更新副本归属用户请求体
     */
    @ExterAttack
    @PutMapping("/action/update-copy-user")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = "0x20640332004C", target = "Resource",
        details = {"$1?.resourceId", "$1?.userId"})
    public void updateCopyUser(@RequestBody UpdateCopyUserObjectReq updateCopyUserObjectReq) {
        log.info("Start update resourceId: {}, userId: {}", updateCopyUserObjectReq.getResourceId(),
            updateCopyUserObjectReq.getUserId());
        resourceService.updateCopyUser(updateCopyUserObjectReq);
    }
}
