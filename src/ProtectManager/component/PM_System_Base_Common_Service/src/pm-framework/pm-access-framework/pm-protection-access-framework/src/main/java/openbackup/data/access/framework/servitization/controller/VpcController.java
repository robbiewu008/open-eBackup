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
package openbackup.data.access.framework.servitization.controller;

import static openbackup.system.base.common.constants.Constants.HUAWEI_CLOUD_STACK;

import com.huawei.oceanprotect.system.base.user.entity.hcs.HcsRoleEnum;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.servitization.controller.req.VpcRequest;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.RequestUtil;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.auth.api.HcsTokenAPi;
import openbackup.system.base.sdk.auth.model.HcsToken;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.hibernate.validator.constraints.Length;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.time.Instant;
import java.util.Collections;
import java.util.Objects;
import java.util.Optional;

import javax.servlet.http.HttpServletRequest;

/**
 * VpcController
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/services/vpc-endpoints")
public class VpcController {
    @Autowired
    private IVpcService vpcService;

    @Autowired
    private HcsTokenAPi hcsTokenAPi;

    @Autowired
    private OperationLogService operationLogService;

    /**
     * 保存vpc信息
     *
     * @param hcsToken hcsToken
     * @param request request
     * @param vpcRequest 请求体
     * @param markId markId
     * @return id
     */
    @ExterAttack
    @PutMapping("/{mark_id}/policy")
    public String savePolicy(@RequestHeader(value = "X-Auth-Token") String hcsToken, HttpServletRequest request,
        @RequestBody @Validated VpcRequest vpcRequest,
        @PathVariable("mark_id") @Length(min = 1, max = 128) String markId) {
        log.info("savePolicy:markId={}", markId);
        boolean isSuccess = false;
        String userName = "";
        String ip = RequestUtil.getClientIpAddress(request);
        LegoCheckedException legoCheckedException = new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        try {
            userName = checkPermission(hcsToken);
            String projectId = vpcRequest.getProjectId();
            String vpdId = vpcRequest.getVpcId();
            String uuid = vpcService.saveVpcInfo(projectId, vpdId, markId);
            isSuccess = true;
            return uuid;
        } catch (LegoCheckedException exception) {
            log.error("Save policy failed, markId is: {}.", markId, ExceptionUtil.getErrorMessage(exception));
            legoCheckedException = exception;
            throw exception;
        } finally {
            StringUtil.clean(hcsToken);
            sendLog("0x2064032B001E", new String[] {HUAWEI_CLOUD_STACK, userName, ip, markId},
                isSuccess, legoCheckedException);
        }
    }

    private void sendLog(String name, String[] params, boolean isSuccess, LegoCheckedException exception) {
        LegoInternalEvent event = new LegoInternalEvent();
        event.setSourceType("operation_target_resource_label");
        event.setEventLevel(FaultEnum.AlarmSeverity.INFO);
        event.setMoName(name);
        event.setMoIP(params[1]);
        event.setEventParam(params);
        event.setEventId(name);
        event.setSourceId(name);
        event.setEventTime(Instant.now().getEpochSecond());
        event.setEventSequence(Instant.now().getNano());
        event.setIsSuccess(isSuccess);
        if (!isSuccess && Objects.nonNull(exception)) {
            event.setLegoErrorCode(String.valueOf(exception.getErrorCode()));
        }
        operationLogService.sendEvent(event);
    }

    /**
     * 权限校验
     *
     * @param token token
     * @return 用户名
     */
    public String checkPermission(String token) {
        HcsToken verifyToken = hcsTokenAPi.verifyAuthToken(token, token);
        // 检查是否有 te_admin 角色
        Optional.ofNullable(verifyToken.getRoles())
            .orElse(Collections.emptyList())
            .stream()
            .filter(role -> HcsRoleEnum.TE_ADMIN.getValue().equals(role.getName()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ACCESS_DENIED));
        return verifyToken.getUser().getName();
    }

    /**
     * 删除vpc信息
     *
     * @param hcsToken hcsToken
     * @param request 请求
     * @param markId markId
     * @return 是否成功
     */
    @ExterAttack
    @DeleteMapping("/{mark_id}/policy")
    public boolean deletePolicy(@RequestHeader(value = "X-Auth-Token") String hcsToken, HttpServletRequest request,
        @PathVariable("mark_id") @Length(min = 1, max = 128) String markId) {
        log.info("deletePolicy:markId={}", markId);
        boolean isSuccess = false;
        String userName = "";
        String ip = RequestUtil.getClientIpAddress(request);
        LegoCheckedException legoCheckedException = new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        try {
            userName = checkPermission(hcsToken);
            VpcInfoEntity vpcInfoEntity = vpcService.getProjectIdByMarkId(markId);
            if (vpcInfoEntity == null) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "markId is not correct");
            }
            isSuccess = vpcService.deleteVpcInfo(markId);
            return isSuccess;
        } catch (LegoCheckedException exception) {
            log.error("Delete policy failed, mark id: {}.", markId, ExceptionUtil.getErrorMessage(exception));
            legoCheckedException = exception;
            throw exception;
        } finally {
            StringUtil.clean(hcsToken);
            sendLog("0x20640332003D", new String[] {HUAWEI_CLOUD_STACK, userName, ip, markId},
                isSuccess, legoCheckedException);
        }
    }
}
