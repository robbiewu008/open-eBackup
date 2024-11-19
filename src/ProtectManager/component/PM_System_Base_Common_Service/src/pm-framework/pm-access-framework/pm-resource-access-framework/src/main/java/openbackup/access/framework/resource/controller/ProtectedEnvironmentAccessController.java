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
import openbackup.access.framework.resource.dto.BaseProtectedEnvironmentDto;
import openbackup.access.framework.resource.dto.ProtectedEnvironmentDto;
import openbackup.access.framework.resource.dto.ProtectedResourceDto;
import openbackup.access.framework.resource.service.lock.ResourceDistributedLockService;
import openbackup.access.framework.resource.util.ResourceUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentDeleteProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.system.base.common.aspect.verifier.CommonOwnershipVerifier;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.util.DefaultRoleHelper;

import org.springframework.beans.BeanUtils;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.stream.Collectors;

import javax.validation.Valid;

/**
 * 受保护环境接入REST API控制器，提供受保护环境相关REST接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/environments")
public class ProtectedEnvironmentAccessController {
    private final ProtectedEnvironmentService environmentService;

    private final SessionService sessionService;

    private final CommonOwnershipVerifier resourceOwnershipVerifier;

    private final ResourceDistributedLockService distributedLockService;

    private final ProviderManager providerManager;

    /**
     * constructor
     *
     * @param environmentService environment service
     * @param sessionService session service
     * @param resourceOwnershipVerifier resourceOwnershipVerifier service
     * @param distributedLockService distributedLockService
     * @param providerManager providerManager
     */
    public ProtectedEnvironmentAccessController(ProtectedEnvironmentService environmentService,
        SessionService sessionService, CommonOwnershipVerifier resourceOwnershipVerifier,
        ResourceDistributedLockService distributedLockService, ProviderManager providerManager) {
        this.environmentService = environmentService;
        this.sessionService = sessionService;
        this.resourceOwnershipVerifier = resourceOwnershipVerifier;
        this.distributedLockService = distributedLockService;
        this.providerManager = providerManager;
    }

    /**
     * 注册受保护环境
     *
     * @param environmentDto 受保护环境
     * @return 受保护环境UUID
     */
    @ExterAttack
    @PostMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.CREATE,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE})
    @Logging(name = "0x206403320035", target = "Environment",
        details = {"$1.name", "=resource_sub_type_#{#snake($1.subType)}_label"})
    public UuidObject registerProtectedEnvironment(@RequestBody @Valid ProtectedEnvironmentDto environmentDto) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        BeanUtils.copyProperties(environmentDto, environment);
        UuidObject uuidObject = new UuidObject();
        ResourceUtil.supplySourceTypeWhenUuidNull(environment, ResourceConstants.SOURCE_TYPE_REGISTER);
        String lockKey = distributedLockService.getResourceLockKey(ResourceConstants.UPDATE_ENV_LOCK, environment);
        distributedLockService.tryLockAndRun(lockKey, environment, (env) ->
            uuidObject.setUuid(environmentService.register(env)));
        return uuidObject;
    }

    /**
     * 检查环境连通性
     *
     * @param environmentDto 即将受保护环境
     * @return 检查结果
     */
    @ExterAttack
    @PostMapping("/action/check")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE},
        target = "#environmentDto.uuid")
    @Logging(name = "0x20640332003B", target = "Environment",
        details = {"$1.name", "=resource_sub_type_#{#snake($1.subType)}_label"})
    public ActionResult[] checkEnvironmentConnectivity(@RequestBody @Valid ProtectedEnvironmentDto environmentDto) {
        // Permission切面无法完成dependency权限校验，将校验功能下移。
        if (environmentDto.getDependencies() != null) {
            verifyDependencyResources(environmentDto.getDependencies());
        }

        ProtectedEnvironment environment = new ProtectedEnvironment();
        BeanUtils.copyProperties(environmentDto, environment);
        return environmentService.checkProtectedEnvironment(environment);
    }

    /**
     * 将dependency中的List<ProtectedResource>及其中子dependency中的ProtectedResource中的uuid全部取出，并做权限校验
     * 关键算法：广度优先遍历。
     *
     * @param dependency 环境的dependency结构
     */
    private void verifyDependencyResources(Map<String, List<ProtectedResource>> dependency) {
        TokenBo.UserBo userBo = TokenBo.get().getUser();
        if (DefaultRoleHelper.isAdmin(userBo.getId())) {
            return;
        }

        List<String> dependencyResourceUuidList = new ArrayList<>();

        // 初始化，dependency结构的第一层Resource入队。
        Queue<ProtectedResource> queue = new LinkedList<>();
        dependency.values().stream().filter(Objects::nonNull).flatMap(Collection::stream).forEach(queue::offer);
        while (queue.size() > 0) {
            ProtectedResource pollResource = queue.poll();
            if (pollResource == null) {
                continue;
            }
            if (!VerifyUtil.isEmpty(pollResource.getUuid())) {
                dependencyResourceUuidList.add(pollResource.getUuid());
            }
            if (VerifyUtil.isEmpty(pollResource.getDependencies())) {
                continue;
            }
            pollResource.getDependencies()
                .values()
                .stream()
                .filter(Objects::nonNull)
                .flatMap(Collection::stream)
                .forEach(queue::offer);
        }
        // 环境中所有dependency结构相关的资源uuid已取出，并放进dependencyResourceUuidList
        resourceOwnershipVerifier.verify(userBo, dependencyResourceUuidList);
    }

    /**
     * 删除受保护环境
     *
     * @param envId 受保护环境ID
     */
    @ExterAttack
    @DeleteMapping("/{envId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.DELETE,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE},
        target = "#envId")
    @Logging(name = "0x206403320034", target = "Environment", requires = "$environment",
        details = {"$1", "$environment?.name", "=resource_sub_type_#{#snake($environment?.subType)}_label"},
        context = @Context(name = "environment", statement = "@resource_dao_select_by_id.call($1)"))
    public void deleteProtectedEnvironment(@PathVariable("envId") String envId) {
        Optional<ProtectedEnvironment> resOptional = environmentService.getBasicEnvironmentById(envId);
        if (!resOptional.isPresent()) {
            return;
        }
        Boolean isCheckSuccess = Optional
                .ofNullable(providerManager.findProvider(ProtectedEnvironmentDeleteProvider.class,
                        resOptional.get().getSubType(), null))
                .map(provider -> provider.frontCheck(resOptional.get())).orElse(true);
        if (!isCheckSuccess) {
            return;
        }
        environmentService.deleteEnvironmentById(envId);
    }

    /**
     * 修改受保护环境
     *
     * @param envId 受保护环境ID
     * @param environmentDto 受保护环境
     */
    @ExterAttack
    @PutMapping("/{envId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.MODIFY,
        authOperations = {AuthOperationEnum.MANAGE_RESOURCE},
        target = "#envId")
    @Logging(name = "0x206403320033", target = "Environment",
        details = {"$1", "$environment.name", "=resource_sub_type_#{#snake($environment.subType)}_label"},
        context = @Context(name = "environment", statement = "@resource_dao_select_by_id.call($1)"))
    public void updateProtectedEnvironment(@PathVariable("envId") String envId,
        @RequestBody @Valid BaseProtectedEnvironmentDto environmentDto) {
        ProtectedEnvironment environment = environmentService.getEnvironmentById(envId);

        // 使用更新后的数据更新已有的数据
        environment.setName(environmentDto.getName());
        environment.setEndpoint(environmentDto.getEndpoint());
        environment.setPort(environmentDto.getPort());
        environment.setUsername(environmentDto.getUsername());
        environment.setPassword(environmentDto.getPassword());
        environment.setOsType(environmentDto.getOsType());
        Map<String, String> extendInfo = environmentDto.getExtendInfo();
        Map<String, String> oldExtendInfo = environment.getExtendInfo();
        environment.setDependencies(environmentDto.getDependencies());

        // 合并扩展信息
        if (extendInfo != null && oldExtendInfo != null) {
            oldExtendInfo.putAll(extendInfo);
        }
        if (extendInfo != null && oldExtendInfo == null) {
            environment.setExtendInfo(extendInfo);
        }
        mergeAuth(environment, environmentDto);
        ResourceUtil.supplySourceTypeWhenUuidNull(environment, ResourceConstants.SOURCE_TYPE_REGISTER);
        String lockKey = distributedLockService.getResourceLockKey(ResourceConstants.UPDATE_ENV_LOCK, environment);
        distributedLockService.tryLockAndRun(lockKey, environment, environmentService::updateEnvironment);
    }

    private void mergeAuth(ProtectedEnvironment environment, BaseProtectedEnvironmentDto environmentDto) {
        Map<String, String> oldAuthExtend = Optional.ofNullable(environment.getAuth())
            .map(Authentication::getExtendInfo)
            .orElse(new HashMap<>());
        Map<String, String> authExtend = Optional.ofNullable(environmentDto.getAuth())
            .map(Authentication::getExtendInfo)
            .orElse(new HashMap<>());
        environment.setAuth(environmentDto.getAuth());
        if (environment.getAuth() != null) {
            environment.getAuth().setExtendInfo(ResourceUtil.merge(Map.class, oldAuthExtend, authExtend, true));
        }
    }

    /**
     * 浏览环境资源
     *
     * @param environmentConditions 查询资源的条件
     * @return 返回分页查询数据
     */
    @ExterAttack
    @GetMapping("/{envId}/resources")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        resourceSetType = ResourceSetTypeEnum.RESOURCE, operation = OperationTypeEnum.QUERY,
        target = "#envId")
    public PageListResponse<ProtectedResourceDto> browseEnvironmentResource(
        BrowseEnvironmentResourceConditions environmentConditions) {
        PageListResponse<ProtectedResource> result = environmentService.browse(environmentConditions);
        List<ProtectedResourceDto> resourceDtoList = result.getRecords()
            .stream()
            .map(this::boToDto)
            .collect(Collectors.toList());
        return new PageListResponse<>(result.getTotalCount(), resourceDtoList);
    }

    private ProtectedResourceDto boToDto(ProtectedResource resource) {
        ProtectedResourceDto dto = new ProtectedResourceDto();
        BeanUtils.copyProperties(resource, dto);
        return dto;
    }
}
