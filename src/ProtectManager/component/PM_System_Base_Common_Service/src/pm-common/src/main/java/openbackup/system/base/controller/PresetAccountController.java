package openbackup.system.base.controller;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.sdk.auth.model.request.PresetAccountRequest;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.service.PresetAccountServiceImpl;

import lombok.extern.slf4j.Slf4j;

import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * 功能描述
 *
 * @author y30021475
 * @since 2023-08-07
 */
@Slf4j
@RestController
@RequestMapping("/v1/external-account/action/save")
public class PresetAccountController {
    private final PresetAccountServiceImpl presetAccountService;

    /**
     * 构造方法
     *
     * @param presetAccountService presetAccountService
     */
    public PresetAccountController(PresetAccountServiceImpl presetAccountService) {
        this.presetAccountService = presetAccountService;
    }

    /**
     * 保存预置账号密码
     *
     * @param presetAccountRequest presetAccountRequest
     * @return UuidObject
     */
    @ExterAttack
    @PutMapping
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    @Logging(name = "0x206403460010", target = "System", details = {"$1.userName"})
    public UuidObject savePresetAccountAndPwd(@RequestBody PresetAccountRequest presetAccountRequest) {
        log.info("start savePresetAccountAndPwd");
        return presetAccountService.savePresetAccountAndPwd(presetAccountRequest);
    }
}
