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
package com.huawei.oceanprotect.system.base.initialize.network.common;

import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 转换方法
 *
 */
public class DmIpList extends ArrayList<String> {
    /**
     * DmIpList
     *
     * @param list list
     */
    public DmIpList(List<String> list) {
        super();
        list.stream().forEach(str -> {
            if (!StringUtils.isEmpty(str)) {
                this.add("\"" + str + "\"");
            }
        });
    }
}
