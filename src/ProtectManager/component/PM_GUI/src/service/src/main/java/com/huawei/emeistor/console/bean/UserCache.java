/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import com.huawei.emeistor.console.contant.CommonConstant;

import lombok.Getter;
import lombok.Setter;

import java.io.Serializable;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

/**
 * 用户缓存相关信息
 * 必须实现序列化才能写入redis
 *
 * @author t00482481
 * @since 2020-07-15
 */
@Getter
@Setter
public class UserCache implements Serializable {
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

    private boolean sessionControl;

    private int sessionLimit = CommonConstant.DEFAULT_SESSION_LIMIT;
}
