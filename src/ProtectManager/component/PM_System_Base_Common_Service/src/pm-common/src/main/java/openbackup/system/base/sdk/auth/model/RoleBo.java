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

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashSet;
import java.util.Set;

/**
 * Role
 *
 * @author c00106403
 * @version [Lego V100R002C10, 2011-1-7]
 * @since 2018-10-30
 */
@Setter
@Getter
public class RoleBo {
    @JsonIgnore
    private Set<OptAuthBo> optsSet = new HashSet<OptAuthBo>();

    @JsonIgnore
    private Set<Long> optIdsSet = new HashSet<Long>();

    // 该角色所包含的用户
    @JsonIgnore
    private Set<UserBo> userBoSet = new HashSet<UserBo>();

    private String creatorId;

    @JsonProperty("is_default")
    private boolean isDefault;

    private String roleId = "0";

    private String createTime;

    // 该角色所包含的用户数量--该字段用于前台显示每个角色所有的用户数量时使用
    private int userNum = 0;

    private String roleDescription = "";

    private String roleName = "";

    @JsonIgnore
    private String defaultStr = "";

    @JsonIgnore
    private int count = 0;

    private void writeObject(ObjectOutputStream stream) throws IOException {
        stream.defaultWriteObject();
    }

    private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
        stream.defaultReadObject();
    }
}
