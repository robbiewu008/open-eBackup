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
package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

import java.util.Date;
import java.util.HashSet;
import java.util.Set;

/**
 * 用户缓存相关信息
 *
 */
@Getter
@Setter
public class UserCache {
    private String userId;

    private Set<String> userSessions = new HashSet<>();

    private Set<String> loginFailedInfos = new HashSet<>();

    private Date loginFailedTime;

    private int loginFailedCount;

    private boolean isLocked;

    private Date lockTime = new Date();

    private Date modifyPwdFailedTime;

    private int modifyPwdFailedCount;

    private boolean isModifyPwdLocked;

    private Date modifyPwdLockTime = new Date();

    private boolean isSessionControl;

    private int sessionLimit;
}
