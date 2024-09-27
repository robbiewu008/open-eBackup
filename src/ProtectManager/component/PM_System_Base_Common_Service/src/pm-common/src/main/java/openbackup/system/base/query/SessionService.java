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
package openbackup.system.base.query;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.util.ThreadCache;

import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.List;
import java.util.Optional;
import java.util.function.Supplier;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Session Service
 *
 * @author l00272247
 * @since 2021-10-16
 */
@Component
public class SessionService {
    private final ThreadCache<TokenBo.UserBo> cache = new ThreadCache<>();

    /**
     * get current user
     *
     * @return current user
     */
    public TokenBo.UserBo getCurrentUser() {
        TokenBo.UserBo userBo = cache.get();
        if (userBo != null) {
            return userBo;
        }
        return getCurrentUser(RequestContextHolder.getRequestAttributes()).orElse(null);
    }

    private Optional<TokenBo.UserBo> getCurrentUser(RequestAttributes requestAttributes) {
        ServletRequestAttributes servletRequestAttributes;
        if (requestAttributes instanceof ServletRequestAttributes) {
            servletRequestAttributes = (ServletRequestAttributes) requestAttributes;
        } else {
            servletRequestAttributes = null;
        }

        if (servletRequestAttributes != null) {
            String path = servletRequestAttributes.getRequest().getRequestURI();
            if (!path.contains("/internal/")) {
                return Optional.ofNullable(TokenBo.get().getUser());
            }
        }
        return Optional.empty();
    }

    /**
     * call method
     *
     * @param supplier supplier
     * @param userBo user bo
     * @param <E> template type
     * @return result
     */
    public <E> E call(Supplier<E> supplier, TokenBo.UserBo userBo) {
        return cache.call(supplier, userBo);
    }

    /**
     * call method
     *
     * @param supplier supplier
     * @param role role
     * @param <E> template type
     * @return result
     */
    public <E> E call(Supplier<E> supplier, String role) {
        return call(supplier, mock(role));
    }

    /**
     * run method
     *
     * @param runnable runnable
     * @param userBo user bo
     */
    public void run(Runnable runnable, TokenBo.UserBo userBo) {
        cache.run(runnable, userBo);
    }

    /**
     * run method
     *
     * @param runnable runnable
     * @param role role
     */
    public void run(Runnable runnable, String role) {
        cache.run(runnable, mock(role));
    }

    /**
     * mock the special role user
     *
     * @param role role
     * @return user bo
     */
    public TokenBo.UserBo mock(String role) {
        return mock(new String[] {role});
    }

    /**
     * mock the special roles user
     *
     * @param roles role
     * @return user bo
     */
    public TokenBo.UserBo mock(String[] roles) {
        List<TokenBo.RoleBo> roleBos =
                Stream.of(roles).map(role -> TokenBo.RoleBo.builder().name(role).build()).collect(Collectors.toList());
        return TokenBo.UserBo.builder().roles(roleBos).build();
    }
}
