package openbackup.access.framework.resource.controller;

import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupResultDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupRequestConverter;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupDetailVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.service.ResourceGroupService;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonOperationCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

/**
 * 资源组Contoller
 *
 * @author c0061681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-23
 */

@RestController
@RequestMapping("/v2/resource/group")
@Slf4j
@Validated
public class ResourceGroupController {
    @Autowired
    private ResourceGroupService resourceGroupService;

    /**
     * 资源组创建
     *
     * @param createResourceGroupReq 资源组创建请求体
     * @return 资源组id
     */
    @ExterAttack
    @PostMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.CREATE,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE})
    @Logging(name = CommonOperationCode.CREATE_RESOURCE_GROUP, target = "Resource", details = {"$return?.uuid"})
    public UuidObject createResourceGroup(@Validated @RequestBody
                                                CreateResourceGroupRequest createResourceGroupReq) {
        ResourceGroupDto resourceGroupDto =
                ResourceGroupRequestConverter.resourceGroupCreateReqToDto(createResourceGroupReq);
        return new UuidObject(resourceGroupService.createResourceGroup(resourceGroupDto));
    }

    /**
     * 资源组修改
     *
     * @param updateResourceGroupReq 资源组修改请求体
     * @param resourceGroupId resourceGroupId
     * @return 资源组id
     */
    @ExterAttack
    @PutMapping("/{resourceGroupId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE}, target = "#resourceGroupId")
    @Logging(name = CommonOperationCode.UPDATE_RESOURCE_GROUP, target = "Resource", details = {"$return?.uuid"})
    public UuidObject updateResourceGroup(@PathVariable("resourceGroupId") String resourceGroupId,
            @Validated @RequestBody UpdateResourceGroupRequest updateResourceGroupReq) {
        ResourceGroupDto resourceGroupDto =
                ResourceGroupRequestConverter.resourceGroupUpdateReqToDto(updateResourceGroupReq);
        resourceGroupDto.setUuid(resourceGroupId);
        return new UuidObject(resourceGroupService.updateResourceGroup(resourceGroupDto));
    }

    /**
     * 资源组列表查询
     *
     * @param conditions 查询条件
     * @param pageNo 页号
     * @param pageSize 每页大小
     * @param orders 排序条件
     * @return 资源组列表
     */
    @ExterAttack
    @GetMapping
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    public PageListResponse<ResourceGroupVo> queryResourceGroups(
            @RequestParam(value = "conditions", defaultValue = "{}", required = false) String conditions,
            @RequestParam(value = "pageNo", defaultValue = "0") Integer pageNo,
            @RequestParam(value = "pageSize", defaultValue = "20") Integer pageSize,
            @RequestParam(value = "orders", required = false) String orders) {
        ResourceGroupQueryParams resourceGroupQueryParams = new ResourceGroupQueryParams();
        resourceGroupQueryParams.setConditions(conditions);
        resourceGroupQueryParams.setPageNo(pageNo);
        resourceGroupQueryParams.setPageSize(pageSize);
        resourceGroupQueryParams.setOrders(orders);
        return resourceGroupService.queryResourceGroups(resourceGroupQueryParams);
    }

    /**
     * 资源组删除
     *
     * @param resourceGroupId 资源组id
     * @return 资源组id
     */
    @ExterAttack
    @DeleteMapping("/{resourceGroupId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.DELETE,
        authOperations = AuthOperationEnum.MANAGE_RESOURCE, target = "#resourceGroupId")
    @Logging(name = CommonOperationCode.DELETE_RESOURCE_GROUP, target = "Resource", details = {"$1"})
    public UuidObject deleteResourceGroup(@PathVariable String resourceGroupId) {
        return new UuidObject(resourceGroupService.deleteResourceGroup(resourceGroupId));
    }

    /**
     * 查询资源组详情(目前最多5个)
     *
     * @param resourceGroupId 演练id
     * @return 资源列表
     */
    @ExterAttack
    @GetMapping("/{resourceGroupId}")
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.QUERY,
        target = "#resourceGroupId")
    public ResourceGroupDetailVo queryResourceGroupDetail(@PathVariable String resourceGroupId) {
        return resourceGroupService.queryResourceGroupDetail(resourceGroupId);
    }

    /**
     * 创建资源组保护
     *
     * @param groupProtectedObjectReq 创建资源组保护请求体
     * @return 资源组id
     */
    @ExterAttack
    @PostMapping("/protected-object")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
            AuthOperationEnum.ARCHIVE},
        target = "#groupProtectedObjectReq.resourceGroupId")
    @Logging(name = CommonOperationCode.CREATE_RESOURCE_GROUP_PROTECTION, target = "Resource",
            details = {"$resourceGroupProtectedObject?.name", "$resourceGroupProtectedObject?.label"},
            context = @Context(name = "resourceGroupProtectedObject",
            statement = "@resource_group_get_resource_group_label.call($1)"))
    public List<ResourceGroupResultDto> createResourceGroupProtectedObject(
            @Validated @RequestBody CreateResourceGroupProtectedObjectRequest groupProtectedObjectReq) {
        return resourceGroupService.createProtectedObject(groupProtectedObjectReq);
    }

    /**
     * 修改资源组保护
     *
     * @param groupProtectedObjectReq 创建资源组保护请求体
     * @return 资源组id
     */
    @ExterAttack
    @PutMapping("/protected-object")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
            AuthOperationEnum.ARCHIVE},
        target = "#groupProtectedObjectReq.resourceGroupId")
    @Logging(name = CommonOperationCode.UPDATE_RESOURCE_GROUP_PROTECTION, target = "Resource",
            details = {"$1.resourceGroupId"})
    public List<ResourceGroupResultDto> updateResourceGroupProtectedObject(
        @Validated @RequestBody UpdateResourceGroupProtectedObjectRequest groupProtectedObjectReq) {
        return resourceGroupService.updateProtectedObject(groupProtectedObjectReq);
    }

    /**
     * 删除资源组保护
     *
     * @param resourceGroupId 资源组ID
     * @return resourceGroupId
     */
    @ExterAttack
    @DeleteMapping("/protected-object/{resourceGroupId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
            AuthOperationEnum.ARCHIVE},
        target = "#resourceGroupId")
    @Logging(name = CommonOperationCode.DELETE_RESOURCE_GROUP_PROTECTION, target = "Resource",
            details = {"$return?.uuid"})
    public UuidObject deleteResourceGroupProtectedObject(@PathVariable String resourceGroupId) {
        return new UuidObject(resourceGroupService.deleteProtectedObject(resourceGroupId));
    }

    /**
     * 资源组批量激活保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/activate-resource-group")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
            AuthOperationEnum.ARCHIVE},
        target = "#batchOperationReq.resourceIds")
    @Logging(name = CommonOperationCode.ACTIVATE_PROTECTION, target = "Protection",
        details = {"$activeResourceGroupNameString"},
        context = @Context(name = "activeResourceGroupNameString",
            statement = "@resource_group_get_resource_group_name_by_id.call($1)"))
    public void activeResourceGroup(
        @Validated @RequestBody ProtectionBatchOperationReq batchOperationReq) {
        resourceGroupService.activateResourceGroup(batchOperationReq);
    }

    /**
     * 资源组批量禁用保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/deactivate-resource-group")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE_GROUP, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.BACKUP, AuthOperationEnum.REPLICATION,
            AuthOperationEnum.ARCHIVE},
        target = "#batchOperationReq.resourceIds")
    @Logging(name = CommonOperationCode.DEACTIVATE_PROTECTION, target = "Protection",
        details = {"$deactivateResourceGroupNameString"},
        context = @Context(name = "deactivateResourceGroupNameString",
            statement = "@resource_group_get_resource_group_name_by_id.call($1)"))
    public void deactivateResourceGroup(
        @Validated @RequestBody ProtectionBatchOperationReq batchOperationReq) {
        resourceGroupService.deactivateResourceGroup(batchOperationReq);
    }
}