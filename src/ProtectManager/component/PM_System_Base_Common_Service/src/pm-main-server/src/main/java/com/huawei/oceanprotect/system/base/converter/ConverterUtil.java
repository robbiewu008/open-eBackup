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
package com.huawei.oceanprotect.system.base.converter;

/**
 * 网络设置转化器
 *
 */
public class ConverterUtil {
    /**
     * 整体(InitNetworkBody)占据占位符的数目
     */
    public static final int INITNETWORK_NUMBER = 18;

    /**
     * 归档网络占据占位符的数目
     */
    public static final int ARCHIVENETWORK_NUMBER = 8;

    /**
     * 备份网络占据占位符的数目
     */
    public static final int BACKUPNETWORK_NUMBER = 8;

    /**
     * 空格分隔符
     */
    public static final String SPACE = " ";

    /**
     * 分号分隔符
     */
    public static final String SEMICOLON = "; ";

    /**
     * 竖线分隔符
     */
    public static final String VERTICAL_LINE = "|";

    /**
     * 逗号
     */
    public static final String COMMA = ",";

    /**
     * 不可用状态
     */
    public static final String NA = "--";

    /**
     * 当整个对象为空的情况
     *
     * @param num 填写几个N/A
     * @return 多个N/A通过逗号拼接的结果
     */
    public static String mutiNa(int num) {
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < num - 1; i++) {
            result.append(NA + COMMA);
        }
        result.append(NA);
        return result.toString();
    }

    /**
     * 判断ip类型是否正确
     *
     * @param ipType ip类型
     * @return 是否是正确的ip类型
     */
    public static boolean correctIptype(String ipType) {
        return "ipv4".equalsIgnoreCase(ipType) || "ipv6".equalsIgnoreCase(ipType);
    }
}
