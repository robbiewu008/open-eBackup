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

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.MalformedJwtException;

import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.security.PublicKey;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;

/**
 * JWT token utils
 *
 * @author y00407642
 * @since 2020-06-05
 */
public class JwtTokenUtils {
    private static final String CLAIM_KEY_USERINFO = "user";

    private static final String CLAIM_KEY_CREATED = "issued_at";

    private static final String CLAIM_KEY_EXPIRES = "expires_at";

    private JwtTokenUtils() {
    }

    /**
     * get claims from token
     *
     * @param token token
     * @param key public key
     * @return claims
     */
    public static Claims getClaimsFromToken(String token, PublicKey key) {
        return Jwts.parser().setSigningKey(key).build().parseClaimsJws(token).getBody();
    }

    /**
     * 获取toekn的明文
     *
     * @param token token
     * @return TokenBo
     */
    public static TokenBo parseToken(String token) {
        return parseToken(token, KeystoreUtils.getInstance().getPublicKey());
    }

    /**
     * 获取token的明文
     *
     * @param token token
     * @param key public key
     * @return TokenBo
     */
    public static TokenBo parseToken(String token, PublicKey key) {
        Claims claims = getClaimsFromToken(token, key);
        TokenBo.UserBo userBo = JSONObject.toBean(JSONObject.fromObject(claims.get(CLAIM_KEY_USERINFO)),
            TokenBo.UserBo.class);
        final Object exp = claims.get(CLAIM_KEY_EXPIRES);
        final Object created = claims.get(CLAIM_KEY_CREATED);
        if (!(exp instanceof Long)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "exp is not instance of Long");
        }
        if (!(created instanceof Long)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "created is not instance of Long");
        }
        return TokenBo.builder().user(userBo).exp((Long) exp).created((Long) created).build();
    }

    /**
     * 从token中解析出用户
     *
     * @param authToken tokenString
     * @return tokenObject信息
     */
    public static TokenBo.UserBo getJwtUserInfo(String authToken) {
        if (authToken == null) {
            return null;
        }
        TokenBo tokenBo;
        try {
            tokenBo = parseToken(authToken);
        } catch (MalformedJwtException e) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
        return tokenBo.getUser();
    }

    /**
     * 在HttpRequest中获取用户认证token
     *
     * @return token信息
     */
    @ExterAttack
    public static String parsingTokenFromRequest() {
        final RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        if (!(requestAttributes instanceof ServletRequestAttributes)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "requestAttributes is not instance of ServletRequestAttributes");
        }
        HttpServletRequest request = ((ServletRequestAttributes) requestAttributes).getRequest();
        // 从http header中获取token
        return request.getHeader(Constants.AUTH_TOKEN);
    }
}
