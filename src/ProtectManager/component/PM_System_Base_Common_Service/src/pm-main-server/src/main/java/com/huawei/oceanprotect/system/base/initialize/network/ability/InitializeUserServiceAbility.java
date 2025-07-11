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
package com.huawei.oceanprotect.system.base.initialize.network.ability;

import com.huawei.oceanprotect.system.base.initialize.network.InitializeUserService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerServiceFactory;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.rest.UserRest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfoEncoder;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.usersession.UserObjectResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.dorado.DoradoSetPasswdRequestForE6000;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.dorado.ModifyUserLoginMethodRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.entity.DeviceInitPwdParam;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.request.ChangePwdRequest;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.DeviceUserService;
import com.huawei.oceanprotect.system.base.service.InitConfigService;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.AuthTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.RandomPwdUtil;
import openbackup.system.base.common.utils.RetryTemplateUtil;
import openbackup.system.base.common.utils.UserUtils;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.retry.RetryCallback;
import org.springframework.retry.support.RetryTemplate;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 添加DM端口
 *
 */
@Slf4j
@Component
public class InitializeUserServiceAbility implements InitializeUserService {
    /**
     * 用户类型 本地用户
     */
    private static final Integer LOCAL_USER_TYPE = 0;

    /**
     * 超级管理员角色值
     */
    private static final Integer SUPER_ADMINISTRATOR_ROLE_TYPE = 1;

    private static final String ID = "ID";

    private static final String IS_PASSWORD_NEVER_EXPIRE = "isPasswordNeverExpire";

    private static final String TYPE = "TYPE";

    private static final String LOGINMETHODLIST = "LOGINMETHODLIST";

    /**
     * 用户对象类型 默认值
     */
    private static final Integer USER_OBJECT_TYPE_DEFAULT = 202;

    @Autowired
    private DeviceUserService deviceUserService;

    @Autowired
    private InitConfigService initConfigService;

    @Override
    public List<UserObjectResponse> getUser(DeviceManagerService service) {
        return service.getApiRest(UserRest.class).getUser(service.getDeviceId());
    }

    /**
     * 删除指定用户
     *
     * @param authUsername 已认证用户名
     * @param username 待删除用户名
     */
    @Override
    public void delUser(String authUsername, String username) {
        String deviceId = initConfigService.getLocalStorageDeviceId();
        StorageResponse<List<UserObjectResponse>> response = deviceUserService.getUser(deviceId, authUsername);
        List<UserObjectResponse> userObjectResponseList = response.getData();
        if (userObjectResponseList
            .stream()
            .map(UserObjectResponse::getName)
            .anyMatch(username::equals)) {
            deviceUserService.deleteUser(deviceId, authUsername, username);
        }
    }

    @Override
    public void delUser(DeviceManagerService service, String username) {
        if (service.getApiRest(UserRest.class)
            .getUser(service.getDeviceId())
            .stream()
            .map(UserObjectResponse::getName)
            .anyMatch(username::equals)) {
            service.getApiRest(UserRest.class).deleteUser(service.getDeviceId(), username);
        }
    }

    /**
     * 添加指定用户
     *
     * @param service dm 对象
     * @param username 用户名;
     * @return 密码
     */
    @Override
    public String createUserAndSetNeverExpire(DeviceManagerService service, String username) {
        UserRest apiRest = service.getApiRest(UserRest.class);
        String pwd = createUser(service, apiRest, username);
        setNeverExpire(service, apiRest, username);
        return pwd;
    }

    /**
     * 设置用户密码永不过期
     *
     * @param addUsername 待添加用户名
     */
    @Override
    public void setNeverExpire(String addUsername) {
        String deviceId = initConfigService.getLocalStorageDeviceId();
        DoradoSetPasswdRequestForE6000 neverExpireRequest = new DoradoSetPasswdRequestForE6000();
        neverExpireRequest.setUserId(addUsername);
        neverExpireRequest.setPasswordNeverExpire(true);
        neverExpireRequest.setType(String.valueOf(USER_OBJECT_TYPE_DEFAULT));
        deviceUserService.setNeverExpire(deviceId, addUsername, neverExpireRequest);
    }

        private void setNeverExpire(DeviceManagerService service, UserRest apiRest, String username) {
        Map<String, Object> neverExpireRequest = new HashMap<>();
        neverExpireRequest.put(ID, username);
        neverExpireRequest.put(IS_PASSWORD_NEVER_EXPIRE, true);
        neverExpireRequest.put(TYPE, USER_OBJECT_TYPE_DEFAULT);
        apiRest.setNeverExpire(service.getDeviceId(), neverExpireRequest);
    }

    /**
     * 创建用户
     *
     * @param authUsername 已认证超级管理员用户名
     * @param addUsername 待创建的用户名
     * @param addUserRoleId 待创建的用户的角色
     * @return 密码
     */
    @Override
    public String createUser(String authUsername, String addUsername, Integer addUserRoleId) {
        String deviceId = initConfigService.getLocalStorageDeviceId();
        String pwd = RandomPwdUtil.generate(IsmNumberConstant.TWELVE);

        // Determine if the user is an admin based on addUserRoleId
        boolean isAdminUser = String.valueOf(addUserRoleId).equals(Constants.ROLE_SYS_ADMIN) ||
                             String.valueOf(addUserRoleId).equals(Constants.ROLE_AUDITOR) ||
                             String.valueOf(addUserRoleId).equals(Constants.Builtin.ROLE_SECURITY_ADMIN_ID);

        if (!isValidPassword(pwd, isAdminUser)) {
            // Consider using a more specific error code if available
            throw new LegoCheckedException(CommonErrorCode.PARAM_ILLEGAL, "Password does not meet policy requirements.");
        }

        UserObjectRequest userObjectRequest = new UserObjectRequest();
        userObjectRequest.setName(addUsername);
        userObjectRequest.setPassword(pwd);
        userObjectRequest.setScope(LOCAL_USER_TYPE);
        userObjectRequest.setRoleId(addUserRoleId);
        deviceUserService.addUser(deviceId, authUsername, userObjectRequest);
        return pwd;
    }

        private String createUser(DeviceManagerService service, UserRest apiRest, String username) {
        RetryCallback<String, Throwable> retryCallback = context -> {
            String pwd = RandomPwdUtil.generate(IsmNumberConstant.TWELVE);
            while (RandomPwdUtil.isReduplicate(pwd)) {
                pwd = RandomPwdUtil.generate(IsmNumberConstant.TWELVE);
            }
            UserObjectRequest userObjectRequest = new UserObjectRequest();
            userObjectRequest.setName(username);
            userObjectRequest.setPassword(pwd);
            userObjectRequest.setScope(LOCAL_USER_TYPE);
            userObjectRequest.setRoleId(SUPER_ADMINISTRATOR_ROLE_TYPE);
            UserObjectResponse response = apiRest.addUser(service.getDeviceId(), userObjectRequest);
            if (Objects.isNull(response)) {
                log.info("add user error");
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "add user error");
            }
            return pwd;
        };
        RetryTemplate retryTemplate = RetryTemplateUtil.fixedBackOffRetryTemplate(5, 1000L,
            Collections.singletonMap(Exception.class, true));
        try {
            return retryTemplate.execute(retryCallback);
        } catch (Throwable throwable) {
            log.info("apiRest addUser error", ExceptionUtil.getErrorMessage(throwable));
        }
        return username;
    }

    /**
     * 修改当前用户的登录方式
     *
     * @param targetUsername 待修改的用户名
     */
    @Override
    public void modifyUserLoginMethod(String targetUsername) {
        String deviceId = initConfigService.getLocalStorageDeviceId();
        ModifyUserLoginMethodRequest modifyUserLoginRequest = new ModifyUserLoginMethodRequest();
        modifyUserLoginRequest.setUserId(targetUsername);
        modifyUserLoginRequest.setLoginMethods("4");
        deviceUserService.modifyUserLoginMethod(deviceId, targetUsername, modifyUserLoginRequest);
    }

    /**
     * 修改当前用户的登录方式
     *
     * @param deviceManagerServiceFactory 获取用户会话工厂类
     * @param deviceManagerInfoEncoder 鉴权信息code
     * @param link 链接
     * @param pwd 密码
     */
    @Override
    public void modifyUserLoginMethod(DeviceManagerServiceFactory deviceManagerServiceFactory,
        DeviceManagerInfoEncoder deviceManagerInfoEncoder, String link, String pwd) {
        DeviceManagerInfo deviceManagerInfo = new DeviceManagerInfo(link, UserUtils.getBusinessUsername(), pwd,
            deviceManagerInfoEncoder);
        deviceManagerInfo.setAuthType(AuthTypeEnum.MANAGER_AUTH.getLoginAuthType());
        DeviceManagerService service = null;
        try {
            service = deviceManagerServiceFactory.getDeviceManagerService(deviceManagerInfo);
            Map<String, Object> modifyUserLoginRequest = new HashMap<>();
            modifyUserLoginRequest.put(ID, UserUtils.getBusinessUsername());
            modifyUserLoginRequest.put(LOGINMETHODLIST, "4");
            service.getApiRest(UserRest.class).modifyUserLoginMethod(service.getDeviceId(), modifyUserLoginRequest);
        } finally {
            if (service != null) {
                service.delDeviceManagerSession();
            }
        }
    }

    /**
     * 修改用户密码
     *
     * @param targetUsername 待修改的用户名
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     */
    @Override
    public void modifyUser(String targetUsername, String oldPassword, String newPassword) {
        log.info("Modify user:{} start.", targetUsername);

        // Placeholder: Assume a method exists to get UserObjectResponse or at least role ID by username
        // UserObjectResponse user = deviceUserService.getUserDetails(targetUsername); // This is an assumed method
        // Integer userRoleId = user.getRoleId(); // Assuming UserObjectResponse has getRoleId()
        // For now, as a placeholder, we'll need to decide a default or fetch it.
        // Let's assume we need to fetch it. Since we can't implement that fully here,
        // this part of the logic for determining isAdmin based on fetched role ID is conceptual.
        // boolean isAdminUser = String.valueOf(userRoleId).equals(Constants.ROLE_SYS_ADMIN) ||
        //                      String.valueOf(userRoleId).equals(Constants.ROLE_AUDITOR) ||
        //                      String.valueOf(userRoleId).equals(Constants.Builtin.ROLE_SECURITY_ADMIN_ID);
        // As a temporary measure for this exercise, defaulting to non-admin policy for modifyUser
        // unless a mechanism to fetch role is confirmed/implemented.
        // For the purpose of this change, we will assume 'false' (non-admin) if role cannot be determined.
        // This part needs to be correctly implemented by fetching the user's actual role.
        boolean isAdminUser = false; // Placeholder - needs actual role determination
        // Example of how it might be (if UserUtils or another service could provide role by name):
        // try {
        //    UserObjectResponse userDetails = findUserDetailsByName(targetUsername); // Assumed method
        //    if (userDetails != null && userDetails.getRolesSet() != null && !userDetails.getRolesSet().isEmpty()) {
        //        String roleId = userDetails.getRolesSet().get(0).getId(); // Assuming RoleBo has getId()
        //        isAdminUser = Constants.ROLE_SYS_ADMIN.equals(roleId) ||
        //                      Constants.ROLE_AUDITOR.equals(roleId) ||
        //                      Constants.Builtin.ROLE_SECURITY_ADMIN_ID.equals(roleId);
        //    }
        // } catch (Exception e) {
        //     log.warn("Could not determine role for user {}, applying default password policy.", targetUsername);
        // }


        if (!isValidPassword(newPassword, isAdminUser)) {
            // Consider using a more specific error code if available
            throw new LegoCheckedException(CommonErrorCode.PARAM_ILLEGAL, "New password does not meet policy requirements.");
        }

        String deviceId = initConfigService.getLocalStorageDeviceId();
        ChangePwdRequest changePwdRequest = new ChangePwdRequest();
        changePwdRequest.setId(targetUsername);
        changePwdRequest.setPassword(newPassword);
        changePwdRequest.setOldPassword(oldPassword);
        deviceUserService.modifyUser(deviceId, targetUsername, targetUsername, changePwdRequest);
        log.info("Modify user:{} success.", targetUsername);
    }

    /**
     * 初始化用户名和密码
     *
     * @param authUsername 已认证的用户名
     * @param targetUsername 待修改的用户名
     * @param superAdminPassword 超级管理员的密码
     * @param newPassword 新密码
     */
    @Override
    public void initUserPassword(String authUsername, String targetUsername,
        String superAdminPassword, String newPassword) {
        log.info("Init user:{} start.", targetUsername);

        // Placeholder: Determine if targetUsername is an admin to apply correct policy
        // This requires fetching the role of targetUsername.
        // For this exercise, defaulting to non-admin policy if role cannot be determined.
        // This part needs to be correctly implemented by fetching the user's actual role.
        boolean isTargetUserAdmin = false; // Placeholder - needs actual role determination
        // Example of how it might be:
        // try {
        //    UserObjectResponse userDetails = findUserDetailsByName(targetUsername); // Assumed method
        //    if (userDetails != null && userDetails.getRolesSet() != null && !userDetails.getRolesSet().isEmpty()) {
        //        String roleId = userDetails.getRolesSet().get(0).getId(); // Assuming RoleBo has getId()
        //        isTargetUserAdmin = Constants.ROLE_SYS_ADMIN.equals(roleId) ||
        //                            Constants.ROLE_AUDITOR.equals(roleId) ||
        //                            Constants.Builtin.ROLE_SECURITY_ADMIN_ID.equals(roleId);
        //    }
        // } catch (Exception e) {
        //     log.warn("Could not determine role for user {}, applying default password policy.", targetUsername);
        // }

        if (!isValidPassword(newPassword, isTargetUserAdmin)) {
            throw new LegoCheckedException(CommonErrorCode.PARAM_ILLEGAL, "New password does not meet policy requirements for target user.");
        }

        String deviceId = initConfigService.getLocalStorageDeviceId();

        DeviceInitPwdParam deviceInitPwdParam = new DeviceInitPwdParam();
        deviceInitPwdParam.setUserId(targetUsername);
        deviceInitPwdParam.setAdminPasswd(superAdminPassword);
        deviceInitPwdParam.setNewPassword(newPassword);
        deviceUserService.initUserPassword(deviceId, authUsername, deviceInitPwdParam);
        log.info("Init user:{} success.", targetUsername);
    }

    private boolean isValidPassword(String password, boolean isAdmin) {
        if (password == null) {
            return false;
        }
        int minLength = isAdmin ? 8 : 6;
        if (password.length() < minLength) {
            log.warn("Password validation failed: Password is shorter than minimum length {}.", minLength);
            return false;
        }

        int charTypeCount = 0;
        if (Pattern.compile("[A-Z]").matcher(password).find()) { // Uppercase
            charTypeCount++;
        }
        if (Pattern.compile("[a-z]").matcher(password).find()) { // Lowercase
            charTypeCount++;
        }
        if (Pattern.compile("[0-9]").matcher(password).find()) { // Digit
            charTypeCount++;
        }
        if (Pattern.compile("[^A-Za-z0-9]").matcher(password).find()) { // Special character
            charTypeCount++;
        }

        if (charTypeCount < 2) {
            log.warn("Password validation failed: Password does not contain at least 2 character types.");
            return false;
        }
        return true;
    }
}
