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

/**
 * 用户访问控制列表
 *
 * @author t00104528
 * @version [Lego V100R002C10, 2010-8-27]
 * @since 2018-10-30
 */
@Getter
@Setter
public class UserAclBo {
    //
    private static final long serialVersionUID = 1L;

    // ACL id
    private long userAclId = 0L;

    // 用户ID
    private long userId = 0L;

    private Long startIp = 0L;

    private String startIpStr;

    private Long endIp = 0L;

    private String endIpStr = "255.255.255.255";

    private String netWork = "";

    private String netWorkMaskStr = "";

    // 多少位数的子网隐码
    private Integer netWorkMask = 0;

    // 描述信息
    private String des = "";
}
