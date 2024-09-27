/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.bean.RedisExpireEntity;
import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.bean.UserCache;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.contant.RedisExpireTypeEnum;
import com.huawei.emeistor.console.dao.RedisExpireDao;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SecurityPolicyService;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.CookieUtils;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.SessionUtils;
import com.huawei.emeistor.console.util.StringUtil;
import com.huawei.emeistor.console.util.TimeoutUtils;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RBucket;
import org.redisson.api.RKeys;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.stereotype.Service;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @since [生成时间]
 */
@Service
@Slf4j
public class SessionServiceImpl implements SessionService {
    private static final String USER_LOCK = "USER_LOCK";

    private static final String USER_CACHE = "USER_LOCK_CACHE";

    private static final String FAILED_TIMES = "FAILED_TIMES";

    private static final int ONE_MINUTES = 1;

    private static final String TIMEOUT_DETAIL = "timeout_detail_";

    private static final String LIVE_LANGUAGE = "livelanguage";

    private static final String LOCALE = "locale";

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private SecurityPolicyService securityPolicyService;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private TimeoutUtils timeoutUtils;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    @Autowired
    private SHA256Encryptor sha256Encryptor;

    @Autowired
    private RedisExpireDao redisExpireDao;

    @Override
    @ExterAttack
    public void deleteSession(String sessionId, boolean isEncrypt) {
        String encSessionId = sessionId;
        if (encSessionId == null) {
            return;
        }
        if (!isEncrypt) {
            encSessionId = sha256Encryptor.encryptionSessionId(encSessionId);
        }
        // 清除redis中的Token
        RBucket<Object> rBucket = redissonClient.getBucket(encSessionId);
        if (rBucket.isExists()) {
            rBucket.delete();
        }
        RBucket<String> userIp = redissonClient.getBucket(TIMEOUT_DETAIL + encSessionId);
        if (userIp.isExists()) {
            userIp.delete();
        }
    }

    @Override
    @ExterAttack
    public void deleteSessionFromReq(HttpServletRequest request, boolean isDeleteSessionId) {
        Cookie[] cookies = request.getCookies();
        if (cookies == null) {
            return;
        }
        for (Cookie cookie : cookies) {
            // 只有EmeiSessionId是后台维护的cookie
            if (StringUtils.equals(cookie.getName(), ConfigConstant.SESSION)) {
                String sessionId = cookie.getValue();
                // 清除cookie
                CookieUtils.clearCookie(response, sessionId);
                String requestUrl = request.getRequestURI();
                if (timeoutUtils.getWhiteList().contains(requestUrl)) {
                    continue;
                }
                // sessionId不合法，则忽略
                if (StringUtils.isEmpty(sessionId) || (!sessionId.startsWith("userId="))) {
                    log.error("Session ID is invalid.");
                    continue;
                }
                if (isDeleteSessionId) {
                    // 清除redis中的Token
                    deleteSession(sessionId, false);
                }
                // 移除redis中USER_LOCK_CACHE中的sessionId
                removeSessionId(sessionId);
                StringUtil.tripleWriteZero(sessionId);
                continue;
            }
            if (LIVE_LANGUAGE.equals(cookie.getName()) || LOCALE.equals(cookie.getName())) {
                continue;
            }
            // 其他比如前台设置的cookie在登出时也要清除
            Cookie respCookie = new Cookie(cookie.getName(), cookie.getValue());
            respCookie.setPath(ConfigConstant.COOKIE_PATH);
            respCookie.setSecure(true);
            respCookie.setMaxAge(0);
            response.addCookie(respCookie);
        }
    }

    @Override
    public SessionInfo genSession(UserCache userCache, SecurityPolicyBo secBo, Token token) {
        // 可读性
        if (!userCache.isSessionControl() || userCache.getSessionLimit() > userCache.getUserSessions().size()) {
            String sessionId = SessionUtils.generateSessionId(userCache.getUserId());
            // 用户验证通过，保存session id
            SessionInfo session = new SessionInfo();
            String encSessionId = sha256Encryptor.encryptionSessionId(sessionId);
            session.setSessionId(encSessionId);
            session.setExpireTime(secBo.getSessionTime());
            session.setToken(token.getToken());
            session.setUserId(token.getUserId());
            session.setCsrfToken(createNewCsrfToken());
            session.setClientSessionIp(RequestUtil.getClientIpAddress(request));

            // 保存到redis需要加密
            saveSession(session, secBo, userCache);
            session.setSessionId(sessionId);
            session.setToken(token.getToken());
            return session;
        }
        throw new LegoCheckedException(CommonErrorCode.LOGIN_USER_EXCEED_MAX_SESSION);
    }

    @Override
    @ExterAttack
    public void unlock(String userId) {
        RLock rlock = redissonClient.getLock(getKeyByUserInfo(USER_LOCK, userId));
        rlock.unlock();
    }

    @Override
    @ExterAttack
    public void lock(String userId) {
        RLock rlock = redissonClient.getLock(getKeyByUserInfo(USER_LOCK, userId));
        rlock.lock();
    }

    @Override
    @ExterAttack
    public List<String> getOnlineSessionIdList() {
        RKeys rKeys = redissonClient.getKeys();
        return rKeys.getKeysStreamByPattern("userId*").collect(Collectors.toList());
    }

    @Override
    @ExterAttack
    public List<String> getOnlineUserList() {
        RKeys rKeys = redissonClient.getKeys();
        List<String> sessionList = rKeys.getKeysStreamByPattern("userId*").collect(Collectors.toList());
        for (int i = 0; i < sessionList.size(); i++) {
            sessionList.set(i, sessionList.get(i).substring(0, sessionList.get(i).indexOf("-")));
        }
        return sessionList;
    }

    @Override
    @ExterAttack
    public SessionInfo getSessionInfo(String sessionId) {
        String encSessionId = sha256Encryptor.encryptionSessionId(sessionId);
        RBucket<Object> rBucket = redissonClient.getBucket(encSessionId);
        if (rBucket.isExists()) {
            try {
                if (rBucket.get() instanceof SessionInfo) {
                    return (SessionInfo) rBucket.get();
                }
                if (rBucket.get() instanceof String) {
                    return JSONObject.parseObject(String.valueOf(rBucket.get()), SessionInfo.class);
                }
            } catch (RedisException ex) {
                log.error("Get session information, session ID parameter is invalid.");
            } catch (Exception ex) {
                log.error("Get session information, session ID parameter is invalid.");
            }
        }
        return new SessionInfo();
    }

    @Override
    @ExterAttack
    public void saveSession(SessionInfo session, SecurityPolicyBo secBo, UserCache userCache) {
        RBucket<String> rBucket = redissonClient.getBucket(session.getSessionId());

        // 加密token,不能明文存储在redis中
        String encryptToken = encryptorRestClient.encrypt(session.getToken()).getCiphertext();
        session.setToken(encryptToken);
        rBucket.set(JSONObject.toJSONString(session));
        rBucket.expire(secBo.getSessionTime(), TimeUnit.MINUTES);

        // 将sessionId和用户id保存到数据库，监听redis键过期
        RedisExpireEntity redisExpireEntity = new RedisExpireEntity();
        redisExpireEntity.setKey(session.getSessionId());
        redisExpireEntity.setUserId(userCache.getUserId());
        redisExpireEntity.setType(RedisExpireTypeEnum.SESSION_TYPE.getType());
        redisExpireDao.insertRedisExpireInfo(redisExpireEntity);
        userCache.setLoginFailedCount(0);
        userCache.getUserSessions().add(session.getSessionId());
        RBucket<String> rb = redissonClient.getBucket(getKeyByUserInfo(USER_CACHE, userCache.getUserId()));
        rb.set(JSONObject.toJSONString(userCache));
    }

    @Override
    @ExterAttack
    public boolean checkTimeout(String encSessionId) {
        RBucket<Object> rBucket = redissonClient.getBucket(encSessionId);
        if (!rBucket.isExists()) {
            log.info("user's status abnormal");
            // 用户状态异常
            throw new LegoCheckedException(CommonErrorCode.USER_STATUS_ABNORMAL, new String[] {},
                "User's status is abnormal.");
        }
        if (checkSessionIp(rBucket)) {
            return true;
        }
        HttpHeaders httpHeaders = requestUtil.getForwardHeaderAndValidCsrf();

        // 本端接口只查询本端的安全策略
        httpHeaders.remove(ConfigConstant.CLUSTER_TYPE);
        SecurityPolicyBo secPolicy = securityPolicyService.getSecurityPolicy(new HttpEntity<>(httpHeaders));
        rBucket.expire(secPolicy.getSessionTime(), TimeUnit.MINUTES);
        RBucket<String> userIp = redissonClient.getBucket(TIMEOUT_DETAIL + encSessionId);
        userIp.set(RequestUtil.getClientIpAddress(request), secPolicy.getSessionTime() + ONE_MINUTES, TimeUnit.MINUTES);
        return false;
    }

    private boolean checkSessionIp(RBucket<Object> rBucket) {
        String clientIpAddress = RequestUtil.getClientIpAddress(request);
        SessionInfo sessionInfo = new SessionInfo();
        if (rBucket.get() instanceof SessionInfo) {
            sessionInfo = (SessionInfo) rBucket.get();
        }
        if (rBucket.get() instanceof String) {
            sessionInfo = JSONObject.parseObject(String.valueOf(rBucket.get()), SessionInfo.class);
        }
        String sessionIp = sessionInfo.getClientSessionIp();
        if (!StringUtils.equals(clientIpAddress, sessionIp)) {
            log.error("clientIpAddress: {}, ip: {}", clientIpAddress, sessionIp);
            return true;
        }
        return false;
    }

    @Override
    @ExterAttack
    public UserCache getUserCache(String userId) {
        RBucket<String> rb = redissonClient.getBucket(getKeyByUserInfo(USER_CACHE, userId));
        UserCache userCache;
        if (rb.isExists()) {
            userCache = JSONObject.parseObject(rb.get(), UserCache.class);
            userCache.setUserId(userId);
            refreshUserCache(userCache);
            return userCache;
        }
        userCache = new UserCache();
        userCache.setUserId(userId);
        rb.set(JSONObject.toJSONString(userCache));
        return userCache;
    }

    /**
     * 更新redis中USER_LOCK_CACHE中的sessionId列表（需要重新塞入redis中才能生效）
     *
     * @param userCache 用户缓存
     */
    private void refreshUserCache(UserCache userCache) {
        Set<String> newSessionSet = new HashSet<>();
        // redis已经加密，不能重复加密
        for (String sessionId : userCache.getUserSessions()) {
            if (redissonClient.getBucket(sessionId).isExists()) {
                newSessionSet.add(sessionId);
            }
        }
        userCache.setUserSessions(newSessionSet);
    }

    /**
     * 创建新的token
     *
     * @return token
     */
    private String createNewCsrfToken() {
        byte[] token = new byte[ConfigConstant.CSRF_TOKEN_LENGTH];
        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            random.nextBytes(token);
        } catch (NoSuchAlgorithmException e) {
            log.error("Failed to generate random number exception:", e);
        }
        return byte2Hex(token);
    }

    /**
     * 将byte数组转换为16进制字符串
     *
     * @param bytes 存放哈希值结果的 byte 数组
     * @return 十六进制字符串
     */
    private String byte2Hex(byte[] bytes) {
        StringBuilder stringBuilder = new StringBuilder();
        String temp;
        for (int i = 0; i < bytes.length; i++) {
            temp = Integer.toHexString(bytes[i] & 0xFF);
            stringBuilder.append(temp.length() == 1 ? '0' : temp);
        }
        return stringBuilder.toString();
    }

    /**
     * 登出时需要删除USER_LOCK_CACHE中的sessionId
     * sessionId格式如下：userId=88a94c476f12a21e016f12a246e50009-loginTime=
     * 161603708496820178f4f45223ea5991dbf25fcacd8acc814c61660db2ea272db11973422a98b
     *
     * @param sessionId sessionId
     */
    @ExterAttack
    private void removeSessionId(String sessionId) {
        if (StringUtils.isBlank(sessionId)) {
            return;
        }
        String[] sessionIdSplit = StringUtils.split(sessionId, "-");
        if (sessionIdSplit.length != 2) {
            return;
        }
        String[] halfSessionIdSplit = StringUtils.split(sessionIdSplit[0], "=");
        if (halfSessionIdSplit.length != 2) {
            return;
        }

        String userId = halfSessionIdSplit[1];

        RBucket<String> rb = redissonClient.getBucket(getKeyByUserInfo(USER_CACHE, userId));
        if (rb.isExists()) {
            UserCache userCache = JSONObject.parseObject(rb.get(), UserCache.class);
            String sessionEnc = sha256Encryptor.encryptionSessionId(sessionId);
            userCache.getUserSessions().remove(sessionEnc);
            rb.set(JSONObject.toJSONString(userCache));
        }
    }

    /**
     * 根据前缀和userId获取sessionKey
     *
     * @param prefix 前缀
     * @param userId userId
     * @return sessionKey
     */
    private String getKeyByUserInfo(String prefix, String userId) {
        return prefix + userId;
    }

    /**
     * 判断是否需要生成验证码
     *
     * @param ip 用户ip
     * @param isFailed 是否需要生成验证码
     * @return 是否需要生成验证码
     */
    @Override
    @ExterAttack
    public boolean needVerificationCode(String ip, boolean isFailed) {
        RBucket<Integer> rBucket = redissonClient.getBucket(FAILED_TIMES + ip);
        if (!isFailed && rBucket.isExists()) {
            rBucket.delete();
            Cookie cap = new Cookie(ConfigConstant.CAPCHA, null);
            cap.setMaxAge(0);
            cap.setPath(ConfigConstant.COOKIE_PATH);
            response.addCookie(cap);
            return false;
        }
        if (isFailed) {
            if (rBucket.isExists()) {
                rBucket.set(rBucket.get() + 1);
                return (rBucket.get() + 1) >= ConfigConstant.SHOULD_VA;
            } else {
                rBucket.set(1);
                return false;
            }
        }
        return false;
    }

    @Override
    @ExterAttack
    public void logoutAllSession(String userId) {
        RBucket<String> rb = redissonClient.getBucket(getKeyByUserInfo(USER_CACHE, userId));
        UserCache userCache;
        if (rb.isExists()) {
            userCache = JSONObject.parseObject(rb.get(), UserCache.class);
            for (String sessionId : userCache.getUserSessions()) {
                deleteSession(sessionId, true);
                StringUtil.tripleWriteZero(sessionId);
            }
            rb.delete();
        }
    }

    @Override
    @ExterAttack
    public void refreshToken(String token) {
        String sessionId = CookieUtils.get(request, ConfigConstant.SESSION);
        String sessionEnc = sha256Encryptor.encryptionSessionId(sessionId);
        RBucket<String> rBucket = redissonClient.getBucket(sessionEnc);
        SessionInfo sessionInfo;
        if (rBucket.isExists()) {
            sessionInfo = JSONObject.parseObject(rBucket.get(), SessionInfo.class);
            sessionInfo.setToken(token);
            rBucket.set(JSONObject.toJSONString(sessionInfo));
        }
    }
}
