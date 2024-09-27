/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.restore.service;

import openbackup.data.access.client.sdk.api.framework.dme.CopyVerifyStatusEnum;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.constants.RestoreTaskExtendInfoConstant;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.util.CollectionUtils;

import java.util.Locale;
import java.util.Map;
import java.util.Objects;

/**
 * 恢复任务校验工具类，负责静态校验逻辑，此类不需要依赖外部服务
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/25
 **/
@Slf4j
public abstract class RestoreValidator {
    /**
     * 校验恢复目标环境是否在线
     *
     * @param taskEnvironment 恢复目标环境信息
     * @param restoreFeature 恢复特性开关
     */
    static void checkEnvironmentIsOnline(TaskEnvironment taskEnvironment, RestoreFeature restoreFeature) {
        if (!restoreFeature.isShouldCheckEnvironmentIsOnline()) {
            log.info("Environment: {} no need to check online.", taskEnvironment.getUuid());
            return;
        }
        final LinkStatusEnum linkStatus = LinkStatusEnum.getByStatus(Integer.valueOf(taskEnvironment.getLinkStatus()));
        PowerAssert.state(!LinkStatusEnum.OFFLINE.equals(linkStatus), () -> new LegoCheckedException(
                CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                String.format(Locale.ENGLISH, "target environment[id=%s] is offline", taskEnvironment.getUuid())));
    }

    /**
     * 校验副本是否能允许恢复
     *
     * @param copy 恢复副本对象
     * @param extendInfo 恢复请求扩展参数
     */
    static void checkCopyCanRestore(Copy copy, Map<String, String> extendInfo) {
        if (needForceRestore(copy.getProperties(), extendInfo)) {
            log.info("Copy: {} is need force restore, skip check.", copy.getUuid());
            return;
        }
        final String errorMessage = String.format(Locale.ENGLISH, "Copy[id=%s] status is [%s], can not restore",
            copy.getUuid(), copy.getStatus());
        PowerAssert.state(CopyStatus.NORMAL.equals(CopyStatus.get(copy.getStatus())),
            () -> new LegoCheckedException(CommonErrorCode.COPY_RESTORE_STATUS_ERROR, errorMessage));
    }

    private static boolean needForceRestore(String properties, Map<String, String> extendInfo) {
        if (CollectionUtils.isEmpty(extendInfo) || StringUtils.isBlank(properties)) {
            log.info("Restore request extend info or copy properties is empty.");
            return false;
        }
        String copyVerifyStatus = JSONObject.fromObject(properties)
            .getString(CopyPropertiesKeyConstant.KEY_VERIFY_STATUS, StringUtils.EMPTY);
        boolean isForceRecovery = Boolean.parseBoolean(extendInfo.getOrDefault(
            RestoreTaskExtendInfoConstant.FORCE_RECOVERY, Boolean.FALSE.toString()));
        return isForceRecovery && Objects.equals(copyVerifyStatus,
            CopyVerifyStatusEnum.VERIFY_FAILED.getVerifyStatus());
    }
}
