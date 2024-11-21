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
package com.huawei.emeistor.console.kmchelp.kmc;

/**
 * 文 件 名: Constant
 * 包 名: com.huawei.kmctests.unitest
 * 描 述: 静态变量
 *
 */
public interface Constant {
    /**
     * 卷数目 1024
     */
    int VOLUME_1024 = 1024;

    /**
     * 主密钥
     */
    String PRIMARY_KEY_STORE_FILE = "primary_key_store_file";

    /**
     * 备份密钥
     */
    String STANDBY_KEY_STORE_FILE = "standby_key_store_file";

    /**
     * caller index from StackTraceElement
     */
    int STACK_CALLER_INDEX = 2;

    /**
     * 回车换行符
     */
    String CR = "\n";

    /**
     * TRUE字符串
     */
    String TRUE = "true";

    /**
     * FALSE字符串
     */
    String FALSE = "false";
}
