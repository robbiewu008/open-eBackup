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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.InitConstants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.constants.UserCache;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.OpServiceUtil;
import openbackup.system.base.util.SpringBeanUtils;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;

import java.util.Optional;

/**
 * 用户相关的工具类
 *
 * @author l00422407
 * @since 2021-02-04
 */
@Slf4j
public class UserUtils {
    private static final String USER_LOCK_CACHE = "USER_LOCK_CACHE";

    private static final String USER_TYPE_LDAP = "LDAP";

    private static final String USER_TYPE_LDAP_GROUP = "LDAPGROUP";

    private static final String USER_TYPE_COMMON = "COMMON";

    /**
     * 删除用户的缓存和session信息，一般强制用户下线可以调用此接口
     *
     * @param redissonClient redis客户端接口
     * @param userId 需要删除缓存和session的用户id
     */
    public static void deleteUserCacheAndSessionInfo(RedissonClient redissonClient, String userId) {
        if (userId == null) {
            return;
        }
        RBucket<String> rb = redissonClient.getBucket(USER_LOCK_CACHE + userId);
        UserCache userCache;
        if (rb.isExists()) {
            userCache = JSONObject.parseObject(rb.get(), UserCache.class);
            for (String sessionId : userCache.getUserSessions()) {
                deleteSessionInfo(redissonClient, sessionId);
            }
            rb.delete();
        }
    }

    /**
     * 根据sessionId删除redis中的session信息
     *
     * @param redissonClient redis客户端接口
     * @param sessionId 需要删除session的id
     */
    public static void deleteSessionInfo(RedissonClient redissonClient, String sessionId) {
        if (sessionId == null) {
            return;
        }
        RBucket<Object> sessionInfoBucket = redissonClient.getBucket(sessionId);
        if (sessionInfoBucket.isExists()) {
            sessionInfoBucket.delete();
        }
        StringUtil.clean(sessionId);
    }

    /**
     * 是否是Ldap用户或用户组用户
     *
     * @param userType 用户组
     * @return 是否是Ldap用户或用户组用户 true->是 false->否
     */
    public static boolean isLdapUser(String userType) {
        return USER_TYPE_LDAP_GROUP.equals(userType) || USER_TYPE_LDAP.equals(userType);
    }

    /**
     * 是否是Ldap用户组用户
     *
     * @param userType 用户组
     * @return 是否是Ldap用户组用户 true->是 false->否
     */
    public static boolean isLdapGroupUser(String userType) {
        return StringUtils.equals(USER_TYPE_LDAP_GROUP, userType);
    }

    /**
     * 是否是ADFS用户
     *
     * @param userType 用户类型
     * @return 是否是ADFS用户
     */
    public static boolean isADFSUser(String userType) {
        return UserTypeEnum.ADFS.getValue().equals(userType);
    }

    /**
     * 是否是DME用户
     *
     * @param userType 用户类型
     * @return 是否是dme用户
     */
    public static boolean isDmeUser(String userType) {
        return UserTypeEnum.DME.getValue().equals(userType);
    }

    /**
     * 需要校验密码版本的用户类型
     *
     * @param userType 用户类型
     * @return 是否需要校验version
     */
    public static boolean isNeedValidatePasswordVersion(String userType) {
        if (StringUtils.isBlank(userType)) {
            return false;
        }
        return StringUtils.equals(userType, USER_TYPE_COMMON);
    }

    /**
     * 校验token
     *
     * @param user token信息
     * @param userInfo 根据id查询数据库的用户信息
     */
    public static void checkToken(TokenBo.UserBo user, Optional<TokenBo.UserInfo> userInfo) {
        // 如果是底座的设备管理员,不校验token
        if (StringUtils.equals(Constants.ROLE_DEVICE_MANAGER, user.getRoles().get(0).getId())) {
            return;
        }
        // 用户不存在报错
        if (!userInfo.isPresent()) {
            log.info("user info is not matches");
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "user is not matches");
        }
        TokenBo.UserInfo dbUserInfo = userInfo.get();
        // 如果是LDAPGROUP类型用户，用户类型匹配即可
        if (UserUtils.isLdapGroupUser(user.getUserType()) && UserUtils.isLdapGroupUser(dbUserInfo.getUserType())) {
            return;
        }
        if (!StringUtils.equals(user.getName(), dbUserInfo.getName()) || !StringUtils.equals(user.getUserType(),
            dbUserInfo.getUserType())) {
            log.info("user info is not matches");
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "user is not matches");
        }
        // COMMON用户校验密码版本
        if (isNeedValidatePasswordVersion(user.getUserType())) {
            checkPasswordVersion(user, userInfo.get());
        }
    }

    /**
     * 校验用户密码version
     *
     * @param userBo token信息
     * @param userDbInfo 根据id查询数据库的用户信息
     */
    public static void checkPasswordVersion(TokenBo.UserBo userBo, TokenBo.UserInfo userDbInfo) {
        if (!StringUtils.equals(String.valueOf(Constants.ROLE_DEVICE_MANAGER), userBo.getRoles().get(0).getId())) {
            if (userDbInfo.getPasswordVersion() == null || userBo.getPasswordVersion() == null
                || !userDbInfo.getPasswordVersion().equals(userBo.getPasswordVersion())) {
                log.error("db pass version: {}, token pass version: {}, userId: {} ", userDbInfo.getPasswordVersion(),
                    userBo.getPasswordVersion(), userBo.getId());
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
            }
        }
    }

    /**
     * 获取业务账户名称
     *
     * @return 业务账户名称
     */
    public static String getBusinessUsername() {
        DeployTypeService deployTypeService = SpringBeanUtils.getBean(DeployTypeService.class);
        if (deployTypeService.isCloudBackup() || deployTypeService.isCyberEngine()
                || deployTypeService.isHyperDetectDeployType() || OpServiceUtil.isHcsService()) {
            return InitConstants.ADMIN;
        }
        return InitConstants.DATAPROTECT_ADMIN;
    }
}
