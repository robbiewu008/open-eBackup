/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.converter;

/**
 * 网络设置转化器
 *
 * @author zwx1016945
 * @since 2021-03-24
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
