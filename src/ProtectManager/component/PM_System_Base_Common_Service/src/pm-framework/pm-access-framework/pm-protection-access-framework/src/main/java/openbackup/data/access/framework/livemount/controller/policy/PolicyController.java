package openbackup.data.access.framework.livemount.controller.policy;

import openbackup.data.access.framework.livemount.controller.policy.request.CreatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.sdk.copy.model.BasePage;
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

import javax.validation.Valid;
import javax.validation.constraints.Size;

/**
 * Updating Policy Controller
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Slf4j
@Validated
@RestController
@RequestMapping("v1/livemount-policies")
public class PolicyController {
    @Autowired
    private PolicyService liveMountPolicyService;

    /**
     * create a updating policy
     *
     * @param createRequest CreateRequest
     */
    @ExterAttack
    @PostMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT_POLICY, operation = OperationTypeEnum.CREATE,
        authOperations = {AuthOperationEnum.LIVE_MOUNT_POLICY})
    @Logging(name = "0x206403380008", target = EventTarget.LIVE_MOUNT, details = "$1.name")
    public void createPolicy(@RequestBody @Valid CreatePolicyRequest createRequest) {
        liveMountPolicyService.createPolicy(createRequest, TokenBo.get().getUser());
    }

    /**
     * update a live mount policy
     *
     * @param policyId policy id
     * @param updateRequest update live mount request
     */
    @ExterAttack
    @PutMapping("/{policyId}/action/update")
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "live_mount_policy:$1", resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT_POLICY,
        operation = OperationTypeEnum.MODIFY, authOperations = {AuthOperationEnum.LIVE_MOUNT_POLICY},
        target = "#policyId")
    @Logging(
            name = "0x206403380009",
            target = EventTarget.LIVE_MOUNT,
            details = {"$policy.name"},
            context = {
                @Context(name = "policy", statement = "@live_mount_policy_dao_select_by_id.call($1)", required = true)
            })
    public void updatePolicy(@PathVariable @Size(max = 128) String policyId,
            @RequestBody @Valid UpdatePolicyRequest updateRequest) {
        liveMountPolicyService.updatePolicy(policyId, updateRequest);
    }

    /**
     * query live mount policies
     *
     * @param page page
     * @param size size
     * @param conditions condition
     * @param orders order
     * @return live mount policies
     */
    @ExterAttack
    @GetMapping
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    public BasePage<LiveMountPolicyEntity> getPolicies(
            @RequestParam("page") int page,
            @RequestParam("size") int size,
            @RequestParam(value = "conditions", required = false) String conditions,
            @RequestParam(value = "orders", required = false) List<String> orders) {
        return liveMountPolicyService.getPolices(page, size, conditions, orders);
    }

    /**
     * query a live mount by id
     *
     * @param policyId live mount policy id
     * @return live mount policy
     */
    @ExterAttack
    @GetMapping("/{policyId}")
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        resources = "live_mount_policy:$1", resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT_POLICY,
        operation = OperationTypeEnum.QUERY, authOperations = {AuthOperationEnum.LIVE_MOUNT_POLICY},
        target = "#policyId")
    public LiveMountPolicyVo getPolicy(@PathVariable(value = "policyId") @Size(max = 128) String policyId) {
        return liveMountPolicyService.getPolicy(policyId);
    }

    /**
     * delete a  live mount policy
     *
     * @param policyId policy id
     */
    @ExterAttack
    @DeleteMapping("/{policyId}")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "live_mount_policy:$1", resourceSetType = ResourceSetTypeEnum.LIVE_MOUNT_POLICY,
        operation = OperationTypeEnum.MODIFY, authOperations = {AuthOperationEnum.LIVE_MOUNT_POLICY},
        target = "#policyId")
    @Logging(
            name = "0x20640338000A",
            target = EventTarget.LIVE_MOUNT,
            details = {"$policy.name"},
            context = {
                @Context(name = "policy", statement = "@live_mount_policy_dao_select_by_id.call($1)", required = true)
            })
    public void deletePolicy(@PathVariable("policyId") @Size(max = 128) String policyId) {
        liveMountPolicyService.deletePolicy(policyId);
    }
}
