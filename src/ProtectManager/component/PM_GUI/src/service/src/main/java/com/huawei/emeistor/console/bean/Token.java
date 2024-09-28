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

import com.huawei.emeistor.console.contant.NumberConstant;

import lombok.Getter;
import lombok.Setter;

import java.io.Serializable;

/**
 * 必须实现序列化才能写入redis
 *
 */
@Getter
@Setter
public class Token implements Serializable {
    private String token;

    private boolean modifyPassword;

    private long expireDay = NumberConstant.DEFAULT_VALUE;

    private String userId;

    private String timeZone;

    private String serviceProduct;
}
