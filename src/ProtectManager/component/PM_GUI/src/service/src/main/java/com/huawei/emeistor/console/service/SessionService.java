/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.service;

import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.bean.UserCache;

import java.util.List;

import javax.servlet.http.HttpServletRequest;

/**
 * Session 相关的操作类
 *
 * @author t00482481
 * @version [LEGO V100R002C01, 2010-7-13]
 * @since 2020-07-05
 */
public interface SessionService {
    /**
     * 删除session，仅清除redis数据
     *
     * @param key sessionID
     * @param isEncrypt sessionId是否已经加密
     */
    void deleteSession(String key, boolean isEncrypt);

    /**
     * 删除session，该方法会清除当前会话的cookie，请谨慎使用（目前只有logout和超时时使用）
     *
     * @param request request对象
     * @param isDeleteSessionId 是否删除会话信息 true 删除
     */
    void deleteSessionFromReq(HttpServletRequest request, boolean isDeleteSessionId);

    /**
     * 默认不删除session
     *
     * @param request request
     */
    default void deleteSessionFromReq(HttpServletRequest request) {
        deleteSessionFromReq(request, false);
    }

    /**
     * 获取用户session信息
     *
     * @param userCache 用户信息
     * @param secBo 安全策略
     * @param token 后台返回的认证信息
     * @return 生成的session信息
     */
    SessionInfo genSession(UserCache userCache, SecurityPolicyBo secBo, Token token);

    /**
     * 锁定改用户
     *
     * @param userName 用户Name
     */
    void unlock(String userName);

    /**
     * 解锁用户
     *
     * @param userName 用户Name
     */
    void lock(String userName);

    /**
     * 获取所有在线的User的userId
     *
     * @return 返回所有在线的User的userId
     */
    List<String> getOnlineUserList();

    /**
     * 获取所有在线的User的sessionId
     *
     * @return 返回所有在线的User的sessionId
     */
    List<String> getOnlineSessionIdList();

    /**
     * 根据sessionId获取session信息
     *
     * @param sessionId sessionId
     * @return session信息
     */
    SessionInfo getSessionInfo(String sessionId);

    /**
     * 保存session信息到缓存中
     *
     * @param session session信息
     * @param secBo 安全策略
     * @param userCache 用户信息缓存
     */
    void saveSession(SessionInfo session, SecurityPolicyBo secBo, UserCache userCache);

    /**
     * 判断session是否超时
     *
     * @param sessionId sessionId
     * @return session是否超时
     */
    boolean checkTimeout(String sessionId);

    /**
     * 通过用户名获取用户缓存信息
     *
     * @param userName 用户Name
     * @return 用户缓存信息
     */
    UserCache getUserCache(String userName);

    /**
     * 判断ip是否需要生产验证码
     *
     * @param ip sessionId
     * @param isFailed 登录是否成功
     * @return session是否超时
     */
    boolean needVerificationCode(String ip, boolean isFailed);

    /**
     * 删除用户强制所有用户下线
     *
     * @param userId 用户ID
     */
    void logoutAllSession(String userId);

    /**
     * 更新token
     *
     * @param token token
     */
    void refreshToken(String token);
}
