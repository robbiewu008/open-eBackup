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
package openbackup.system.base.sdk.auth.model;

import openbackup.system.base.common.constants.DataView;
import openbackup.system.base.common.enums.AccessControlEnum;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonView;

import lombok.Getter;
import lombok.Setter;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashSet;
import java.util.Set;

/**
 * 前台用户对象
 *
 */
@JsonView(DataView.Normal.class)
@Getter
@Setter
public class UserBo {
    /**
     * me
     */
    public static final String ME = "me";

    private Set<ResourceSetAuthorization> resourceSetAuthorizationSets = new HashSet<>();

    private Set<RoleBo> rolesSet = new HashSet<>();

    private Set<String> rolesIdsSet = new HashSet<>();

    // 用户访问控制列表 <变量的意义、目的、功能和可能被用到的地方>
    private Set<UserAclBo> userAclBoSet = new HashSet<>();

    private String userId;

    private long loginTime = 0L;

    private String ipAdr = "";

    private String userName = "";

    @JsonView(DataView.Confidential.class)
    private String userPassword = "";

    @JsonView(DataView.Confidential.class)
    @JsonIgnore
    private String userPasswordSalt = "";

    private String description = "";

    @JsonIgnore
    private String session = "";

    private String loginDate = "";

    private String stayTime = "";

    @JsonIgnore
    private long setTime;

    // 是否应该强制用户修改密码的标志（第一次登录强制用户修改密码）
    @JsonIgnore
    private boolean mustModifyPwd = false;

    private boolean lock = false;

    // 密码认证模式
    @JsonIgnore

    private int passwordModel = 0;

    // 查询设备方式
    @JsonIgnore
    private int selectType = 0;

    @JsonIgnore
    private boolean isHcsUserManagePermission = true;

    // 用户是否为administrator角色
    @JsonIgnore
    private String isAdministrator = "false";

    // 开启session个数控制
    private Boolean sessionControl = false;

    // 开启session限制的个数
    private Integer sessionLimit = 0;

    // 是否为默认内置用户
    private boolean defaultUser;

    // 资源列表，不需要入库
    private Set<Long> resIds = new HashSet<Long>();

    private String domainId;

    /**
     * 用户的访问控制，控制用户是否可以登录REST API还是GUI
     */
    private AccessControlEnum accessControlEnum;

    private boolean isLogin;

    private boolean isVisitable;

    private boolean sysAdmin;

    private Long passwordVersion;

    private String userType;

    private int loginType;

    private String dynamicCode;

    private String dynamicCodeEmail;

    private int language = 1;

    private boolean isNeverExpire;

    private void writeObject(ObjectOutputStream stream) throws IOException {
        stream.defaultWriteObject();
    }

    private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
        stream.defaultReadObject();
    }

    /**
     * toString
     *
     * @return String
     */
    @Override
    public String toString() {
        return "User [userName=" + userName + "]";
    }
}
