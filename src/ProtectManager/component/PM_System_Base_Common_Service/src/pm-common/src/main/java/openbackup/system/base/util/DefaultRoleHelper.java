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
package openbackup.system.base.util;

import static openbackup.system.base.common.constants.Constants.Builtin.DEFAULT_BUILT_IN_ROLES_LIST;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.sdk.user.RoleServiceApi;
import openbackup.system.base.sdk.user.model.RolePo;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 默认角色工具类
 *
 */
@Slf4j
@Component
public class DefaultRoleHelper {
    private static RoleServiceApi roleServiceApi;

    public DefaultRoleHelper(RoleServiceApi roleServiceApi) {
        DefaultRoleHelper.roleServiceApi = roleServiceApi;
    }

    /**
     * 获取默认角色
     *
     * @param userId 用户id
     * @return 默认角色
     */
    public static RolePo getDefaultRoleByUserId(String userId) {
        return roleServiceApi.getDefaultRolePoByUserId(userId);
    }

    /**
     * 是否为系统管理员或审计员
     *
     * @param roleName 角色名称
     * @return check result
     */
    public static boolean isAdminOrAuditByRoleName(String roleName) {
        return Constants.Builtin.ADMIN_AUDITOR.contains(roleName);
    }

    /**
     * 是否为系统管理员或审计员
     *
     * @param userId 用户id
     * @return check result
     */
    public static boolean isAdminOrAudit(String userId) {
        return Constants.Builtin.ADMIN_AUDITOR.contains(getDefaultRoleByUserId(userId).getRoleName());
    }

    /**
     * 是否为远端设备管理员
     *
     * @param userId 用户id
     * @return check result
     */
    public static boolean isRdAdmin(String userId) {
        return Constants.Builtin.ROLE_RD_ADMIN.equals(getDefaultRoleByUserId(userId).getRoleName());
    }

    /**
     * 是否为审计员
     *
     * @param userId 用户id
     * @return check result
     */
    public static boolean isAudit(String userId) {
        return StringUtils.equals(getDefaultRoleByUserId(userId).getRoleName(), Constants.Builtin.ROLE_AUDITOR);
    }

    /**
     * 是否为系统管理员
     *
     * @param userId 用户id
     * @return check result
     */
    public static boolean isAdmin(String userId) {
        return StringUtils.equals(getDefaultRoleByUserId(userId).getRoleName(), Constants.Builtin.ROLE_SYS_ADMIN);
    }

    /**
     * 是否为数据保护管理员
     *
     * @param userId 用户id
     * @return true/false
     */
    public static boolean isDp(String userId) {
        return StringUtils.equals(getDefaultRoleByUserId(userId).getRoleName(), Constants.Builtin.ROLE_DP_ADMIN);
    }

    /**
     * 是否为设备管理员
     *
     * @param roleName 角色名称
     * @return true 设备管理员， false 不是设备管理员
     */
    public static boolean isDeviceManagerByRoleName(String roleName) {
        return StringUtils.equals(roleName, Constants.Builtin.ROLE_DEVICE_MANAGER);
    }

    /**
     * 获取不保护系统管理员和数据保护管理员的内置角色列表
     *
     * @return 角色列表
     */
    public static List<String> getIncludeRoleList() {
        return Arrays.asList(Constants.Builtin.ROLE_RD_ADMIN, Constants.Builtin.ROLE_DR_ADMIN,
                Constants.Builtin.ROLE_DEVICE_MANAGER, Constants.Builtin.ROLE_AUDITOR);
    }

    /**
     * 用于校验只包含数据保护管理员，和自定义用户的（排除方式）
     *
     * @return 角色列表
     */
    public static List<String> getExcludeRoleList() {
        return Arrays.asList(Constants.Builtin.ROLE_RD_ADMIN, Constants.Builtin.ROLE_DR_ADMIN,
            Constants.Builtin.ROLE_DEVICE_MANAGER, Constants.Builtin.ROLE_AUDITOR, Constants.Builtin.ROLE_SYS_ADMIN);
    }

    /**
     * 是否为内置角色角色
     *
     * @param roleId 角色id
     * @return true or false
     */
    @JsonIgnore
    public static boolean isBuiltInRole(String roleId) {
        return DEFAULT_BUILT_IN_ROLES_LIST.contains(roleId);
    }
}
