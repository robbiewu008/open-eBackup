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
package com.huawei.emeistor.console.bean;

import lombok.Getter;
import lombok.Setter;

import java.io.Serializable;

/**
 * 安全策略VO 必须实现序列化才能写入redis
 *
 */
@Getter
@Setter
public class SecurityPolicyBo implements Serializable {
    /**
     * 密码最小长度
     */
    private Integer passLenVal;

    /**
     * 密码复杂度
     */
    private Integer passComplexVal;

    /**
     * 是否打开有效期&修改间隔时间
     */
    private Boolean passCtrl;

    /**
     * 密码有效期
     */
    private Integer usefulLife;

    /**
     * 密码修改间隔时间
     */
    private Integer minLifetime;

    /**
     * 会话超时时间
     */
    private Integer sessionTime;

    /**
     * 密码连续输入错误次数
     */
    private Integer passErrNum;

    /**
     * 用户锁定时间
     */
    private Integer passLockTime;

    /**
     * 历史密码保留最大个数
     */
    private Integer passHistoryNum;

    /**
     * 历史密码保留最长时间
     */
    private Integer passHistoryDay;

    /**
     * 是否开启登录信息提示
     */
    private Boolean isEnableLoginNotes = true;

    /**
     * 用户是否自定义信息提示
     */
    private Boolean isEnableUserDefNotes = false;

    /**
     * 用户自定义信息
     */
    private String userDefNodes;
}
