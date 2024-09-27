/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.livemount;

import openbackup.data.access.framework.livemount.common.LiveMountOperateType;
import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.UnmountExtendParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountMigrateRequest;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountUpdate;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountUpdateMode;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import lombok.extern.slf4j.Slf4j;

import org.hibernate.validator.constraints.Length;
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

import javax.validation.Valid;
import javax.validation.constraints.Size;

/**
 * Live Mount Controller
 *
 * @author l00272247
 * @since 2020-09-17
 */
@Validated
@RestController
@RequestMapping("v1/live-mount")
@Slf4j
public class LiveMountController {
    @Autowired
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private LiveMountService liveMountService;

    /**
     * create live mount
     *
     * @param liveMountObject live mount object
     * @return livemount uuids
     */
    @ExterAttack
    @PostMapping
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
            resources = {
                "resource:$1.sourceResourceId;$1.targetResourceUuidList",
                "copy:$1.copyId",
                "live_mount_policy:$1.policyId"
            },
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.QUERY,
        target = "#liveMountObject.copyId")
    @Logging(name = "0x206403380001", target = EventTarget.LIVE_MOUNT, details = "#join($return, ' ')")
    public List<String> createLiveMount(@RequestBody @Valid LiveMountObject liveMountObject) {
        return liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * create live mount 创建共享路径恢复-安全一体机
     *
     * @param liveMountObject live mount object
     * @return livemount uuids
     */
    @ExterAttack
    @PostMapping("/cyber")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = {
            "resource:$1.sourceResourceId;$1.targetResourceUuidList",
            "copy:$1.copyId",
            "live_mount_policy:$1.policyId"
        }, enableCheckAuth = false)
    @Logging(name = "0x20640338000B", target = EventTarget.LIVE_MOUNT, details = "#join($return, ' ')")
    public List<String> createLiveMountCyber(@RequestBody @Valid LiveMountObject liveMountObject) {
        return liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * modify live mount
     *
     * @param liveMountId live mount id
     * @param liveMountParam live mount param
     */
    @ExterAttack
    @PutMapping("/{live_mount_id}")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = {"live_mount:$1", "live_mount_policy:$2.policyId"},
        resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT, operation = OperationTypeEnum.MODIFY,
        target = "#liveMountId", authOperations = {AuthOperationEnum.LIVE_MOUNT}
    )
    @Logging(name = "0x206403380002", target = EventTarget.LIVE_MOUNT, details = "$1")
    public void modifyLiveMount(@PathVariable("live_mount_id") @Length(max = 64) String liveMountId,
        @RequestBody @Valid LiveMountParam liveMountParam) {
        liveMountService.modifyLiveMount(liveMountId, liveMountParam);
    }

    /**
     * update live mount
     *
     * @param liveMountId live mount id
     * @param liveMountUpdate live mount update
     */
    @ExterAttack
    @PutMapping("/{live_mount_id}/action/update")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = {"live_mount:$1", "copy:$2.copyId"}, resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT,
        operation = OperationTypeEnum.MODIFY, authOperations = {AuthOperationEnum.LIVE_MOUNT},
        target = "#liveMountId")
    @Logging(name = "0x206403380003", target = EventTarget.LIVE_MOUNT, details = "$1")
    public void updateLiveMount(@PathVariable("live_mount_id") @Size(max = 128) String liveMountId,
        @RequestBody @Valid LiveMountUpdate liveMountUpdate) {
        LiveMountEntity liveMountEntity = liveMountService.selectLiveMountEntityById(liveMountId);
        liveMountService.checkLiveMountStatus(liveMountEntity.getStatus(), LiveMountOperateType.UPDATE);
        if (!LiveMountStatus.canMount(liveMountEntity.getStatus())) {
            throw new LegoCheckedException(
                    CommonErrorCode.STATUS_ERROR, "Current status is not allow update operation.");
        }
        LiveMountUpdateMode mode = liveMountUpdate.getMode();
        LiveMountPolicyEntity policy = constructLiveMountPolicy(liveMountEntity);
        String copyId;
        if (mode == LiveMountUpdateMode.LATEST) {
            policy.setCopyDataSelectionPolicy(CopyDataSelection.LATEST.getName());
            copyId = null;
        } else {
            if (VerifyUtil.isEmpty(liveMountUpdate.getCopyId())
                    || liveMountUpdate.getCopyId().length() > IsmNumberConstant.HUNDRED_TWENTY_EIGHT) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "policy id is greater than 128.");
            }
            copyId = liveMountUpdate.getCopyId();
            Copy copy = liveMountService.queryValidCopy(liveMountEntity.getResourceId(), copyId);
            liveMountService.checkSourceCopy(copy);
        }
        liveMountService.checkTargetEnvironmentStatus(liveMountEntity);
        liveMountService.updateLiveMount(liveMountEntity, policy, copyId, true);
    }

    private LiveMountPolicyEntity constructLiveMountPolicy(LiveMountEntity liveMountEntity) {
        if (!VerifyUtil.isEmpty(liveMountEntity.getPolicyId())) {
            return liveMountPolicyEntityDao.selectPolicy(liveMountEntity.getPolicyId());
        }
        LiveMountPolicyEntity policy = new LiveMountPolicyEntity();
        policy.setRetentionPolicy(RetentionType.PERMANENT.getName());
        return policy;
    }

    /**
     * destroy live mount
     *
     * @param liveMountId live mount id
     * @param isReserveCopy reserve copy
     * @param isForceDelete force delete
     */
    @ExterAttack
    @DeleteMapping("/{live_mount_id}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "live_mount:$1", resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT,
        operation = OperationTypeEnum.MODIFY, target = "#liveMountId", authOperations = {AuthOperationEnum.LIVE_MOUNT})
    @Logging(name = "0x206403380004", target = EventTarget.LIVE_MOUNT, details = "$1")
    public void unmountLiveMount(@PathVariable("live_mount_id") @Size(max = 128) String liveMountId,
        @RequestParam(value = "reserve_copy", required = false, defaultValue = "false") boolean isReserveCopy,
        @RequestParam(value = "force_delete", required = false, defaultValue = "false") boolean isForceDelete) {
        liveMountService.unmountLiveMount(liveMountId, isReserveCopy, isForceDelete, UnmountExtendParam.getInstance());
    }

    /**
     * query live mount entities
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @return live mount entity page
     */
    @ExterAttack
    @GetMapping
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    public BasePage<LiveMountModel> queryLiveMountEntities(@RequestParam("page") int page,
        @RequestParam("size") int size, @RequestParam(value = "conditions", required = false) String conditions,
        @RequestParam(value = "orders", required = false) List<String> orders) {
        return liveMountService.queryLiveMountEntities(page, size, conditions, orders);
    }

    /**
     * query target environments
     *
     * @param copyId copy id
     * @return base page data
     */
    @ExterAttack
    @GetMapping("/environment/{copy_id}")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        resources = "copy:$1", resourceSetType = ResourceSetTypeEnum.COPY,
        operation = OperationTypeEnum.QUERY, target = "#copyId")
    public BasePage<Environment> queryTargetEnvironments(@PathVariable("copy_id") @Size(max = 128) String copyId) {
        BasePage<Environment> basePage = liveMountService.queryTargetEnvironments(copyId);
        List<Environment> environments = basePage.getItems();
        if (!VerifyUtil.isEmpty(environments)) {
            environments.forEach(env -> {
                StringUtil.clean(env.getPassword());
                env.setPassword(null);
            });
        }
        return basePage;
    }

    /**
     * activate the live mount.
     *
     * @param liveMountId liveMount id
     */
    @ExterAttack
    @PutMapping("/{live_mount_id}/action/activate")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "live_mount:$1", resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT,
        operation = OperationTypeEnum.MODIFY, target = "#liveMountId", authOperations = {AuthOperationEnum.LIVE_MOUNT})
    @Logging(name = "0x206403380005", target = EventTarget.LIVE_MOUNT, details = "$1")
    public void active(@PathVariable("live_mount_id") @Size(max = 128) String liveMountId) {
        if (VerifyUtil.isEmpty(liveMountId)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "live mount id is not null.");
        }
        liveMountService.activateLiveMount(liveMountId);
    }

    /**
     * deactivate the live mount.
     *
     * @param liveMountId liveMount id
     */
    @ExterAttack
    @PutMapping("/{live_mount_id}/action/deactivate")
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "live_mount:$1",
        resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT, operation = OperationTypeEnum.MODIFY, target = "#liveMountId",
        authOperations = {AuthOperationEnum.LIVE_MOUNT})
    @Logging(name = "0x206403380006", target = "LiveMount", details = "$1")
    public void deactivate(@PathVariable("live_mount_id") @Size(max = 128) String liveMountId) {
        if (VerifyUtil.isEmpty(liveMountId)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "live mount id is not null.");
        }
        liveMountService.deactivateLiveMount(liveMountId);
    }

    /**
     * migrate the live mount
     *
     * @param liveMountId live mount id
     * @param liveMountMigrateRequest live mount migrate request param
     */
    @ExterAttack
    @PutMapping("/{live_mount_id}/action/migrate")
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "live_mount:$1",
        resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT, operation = OperationTypeEnum.MODIFY, target = "#liveMountId",
        authOperations = {AuthOperationEnum.LIVE_MOUNT})
    @Logging(name = "0x206403380007", target = EventTarget.LIVE_MOUNT, details = "$1")
    public void migrate(@PathVariable("live_mount_id") @Size(max = 128) String liveMountId,
        @RequestBody @Valid LiveMountMigrateRequest liveMountMigrateRequest) {
        if (VerifyUtil.isEmpty(liveMountId)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "live mount id is not null.");
        }
        liveMountService.migrateLiveMount(liveMountId, liveMountMigrateRequest);
    }
}
