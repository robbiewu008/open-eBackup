/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.constants;

import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.experimental.Tolerate;

import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.io.Serializable;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

/**
 * TokenBo Entity
 *
 * @author y00407642
 * @since 2019-11-13
 */
@Builder
@AllArgsConstructor
@NoArgsConstructor
@Data
public class TokenBo {
    private long created;

    private long exp;

    private UserBo user;

    /**
     * get TokenBo from request
     *
     * @return TokenBo
     */
    public static TokenBo get() {
        return get(new LegoCheckedException("System Error"));
    }

    /**
     * get token bo from request
     *
     * @param exception error
     * @return token bo
     */
    public static TokenBo get(LegoCheckedException exception) {
        final RequestAttributes attributes = RequestContextHolder.getRequestAttributes();
        if (attributes == null) {
            if (exception != null) {
                throw exception;
            }
            return null;
        }
        if (!(attributes instanceof ServletRequestAttributes)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "RequestAttributes is not instance of ServletRequestAttributes");
        }
        ServletRequestAttributes servletRequestAttributes = (ServletRequestAttributes) attributes;
        HttpServletRequest request = servletRequestAttributes.getRequest();
        Object tokenBo = request.getAttribute(OperationLogAspect.TOKEN_BO);
        if (tokenBo instanceof TokenBo) {
            return (TokenBo) tokenBo;
        }
        if (exception != null) {
            throw exception;
        }
        return null;
    }

    /**
     * UserBo Entity
     *
     * @author y00407642
     * @since 2019-11-13
     */
    @Builder
    @Data
    public static class UserBo implements Serializable {
        private static final long serialVersionUID = -1;

        private String id;

        private String name;

        private String userType;

        private List<RoleBo> roles;

        private Long passwordVersion;

        private boolean isHcsUserManagePermission = true;

        private String domainId;

        /**
         * default constructor
         */
        @Tolerate
        public UserBo() {
        }
    }

    /**
     * UserInfo Entity
     *
     * @author c30009520
     * @since 2021-01-30
     */
    @Data
    public static class UserInfo extends UserBo {
        private boolean mustModifyPassword;

        private int loginType;

        private String dynamicCodeEmail;
    }

    /**
     * RoleBo Entity
     *
     * @author y00407642
     * @since 2019-11-13
     */
    @Builder
    @Data
    @AllArgsConstructor
    public static final class RoleBo implements Serializable {
        private static final long serialVersionUID = -1;

        private String name;

        private String id;

        /**
         * default constructor
         */
        @Tolerate
        public RoleBo() {
        }
    }
}
