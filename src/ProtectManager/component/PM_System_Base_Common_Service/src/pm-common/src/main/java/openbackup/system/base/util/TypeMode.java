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
package openbackup.system.base.util;

/**
 * 输入参数类型
 *
 */
public interface TypeMode {
    /**
     * 通用校验
     */
    int COMMON = 1;

    /**
     * url格式校验
     */
    int URL = 2;

    /**
     * 输入姓名校验
     */
    int NAME = 3;

    /**
     * 原有黑名单中去除部分字符 去掉逗号，分号，中横线，圆括号
     */
    int COMMON_LOCAL_REMARK = 4;

    /**
     * 通用特殊字符校验
     */
    int SPECIAL_CHARACTERS_COMMON = 5;
}
