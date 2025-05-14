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

/**
 * 初始化错误类
 *
 */
public class InitErrorCode {
    /**
     * 错误场景：执行初始化参数配置时，由于网络平面配置参数不满足要求，操作失败。
     * 原因：网络平面（{0}）配置参数（{1}）不满足要求（{2}）。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long NO_REQUIRED_PARAMETERS_ERROR = 1677935646L;

    /**
     * 错误场景：执行初始化参数配置时，由于配置平面网络名称重复，操作失败。
     * 原因：配置平面网络名称重复。
     * 建议：请修改平面网络名称之后重试。
     */
    public static final long EXIST_NAME_DUPLICATE_ERROR = 1677935654L;

    /**
     * 错误场景：执行初始化参数配置时，由于参数不满足要求，操作失败。
     * 原因：{0}参数（{1}）不满足要求（{2}）。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long LLD_FILE_REQUIRED_ERROR = 1677935655L;

    /**
     * 错误场景：执行初始化参数配置时，由于网络平面下的参数不存在，操作失败。
     * 原因：网络平面{0}下参数（{1}）不存在。
     * 建议：请添加参数之后重试。
     */
    public static final long NET_PLANE_PARAMETER_NO_EXIST = 1677935656L;

    /**
     * 错误场景：执行初始化参数配置时，由于网络平面的接线数量少于所需数量，操作失败。
     * 原因：网络平面（{0}）的{1}接线数量少于所需数量（{2}）。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long NET_PLANE_PORIT_LINE_NO_ENOUGH = 1677935657L;

    /**
     * 错误场景：执行初始化参数配置时，由于参数信息已被使用，操作失败。
     * 原因：参数（{0}：{1}）已被使用。
     * 建议：请修改为其他{0}或者删除之后重试。
     */
    public static final long NET_WORK_CARD_ALREADY_ERROR = 1677935658L;

    /**
     * 错误场景：执行初始化参数配置时，由于业务逻辑端口已存在，操作失败。
     * 原因：业务逻辑端口（{0}）已存在。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long LOGIC_PORT_IP_ALREADY_EXIST = 1677935659L;

    /**
     * 错误场景：执行初始化参数配置时，由于网络平面存在配置相同的容器端口，操作失败。
     * 原因：网络平面（{0）存在配置相同的容器端口。
     * 建议：请修改网络平面的容器端口之后重试。
     */
    public static final long NET_PLANE_EXIST_THE_SAME_CONTAINER_PORT = 1677935660L;

    /**
     * 错误场景：执行初始化参数配置时，由于网络平面个数超过上限，操作失败。
     * 原因：网络平面（{0}）个数超过上限（{1}）。
     * 建议：请修改为满足要求的网络平面个数后重试。
     */
    public static final long NET_PLANE_UPPER_LIMIT = 1677935662L;

    /**
     * 错误场景：执行初始化参数配置时，由于网络平面未配置，操作失败。
     * 原因：网络平面（{0}）未配置。
     * 建议：请添加网络平面后重试。
     */
    public static final long NET_PLANE_NOT_EXIST = 1677935663L;
}
