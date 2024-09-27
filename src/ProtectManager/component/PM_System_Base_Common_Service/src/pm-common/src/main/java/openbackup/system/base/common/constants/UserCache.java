/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00422407
 * @since 2021-02-04
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
