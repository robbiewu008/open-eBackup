/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.sdk.auth.model.RoleBo;

import java.util.Set;

/**
 * 功能描述
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-26
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class UserInnerResponse {
    private String userId;

    private String userName;

    private boolean lock;

    private String description;

    private boolean sessionControl;

    private int sessionLimit;

    private String accessControl;

    private boolean defaultUser;

    private Set<RoleBo> rolesSet;

    private boolean sysAdmin;

    private String userType;

    private int loginType;

    private String dynamicCodeEmail;

    private boolean isNeverExpire;
}
