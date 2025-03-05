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
package openbackup.system.base.common.constants;

/**
 * FeignClient策略常量
 *
 */
public class FeignClientConstant {
    /**
     * FeignClient连接默认超时时间(ms)
     */
    public static final int CONNECT_TIMEOUT = 30 * 1000; // 30s

    /**
     * FeignClient连接默认超时时间(s)
     */
    public static final int CONNECT_TIMEOUT_SEC = 30; // 30s

    /**
     * FeignClient读取默认超时(ms)
     */
    public static final int READ_TIMEOUT = 2 * 60 * 1000; // 2分钟

    /**
     * FeignClient快速失败连接超时时间(ms)
     */
    public static final int FAST_FAIL_CONNECT_TIMEOUT = 5 * 1000; // 5s

    /**
     * FeignClient快速失败读取超时(ms)
     */
    public static final int FAST_FAIL_READ_TIMEOUT = 5 * 1000; // 5s

    /**
     * FeignClient读取默认超时(s)
     */
    public static final int READ_TIMEOUT_SEC = 2 * 60; // 2分钟

    /**
     * VMwareFeignClient读取默认超时(ms)
     */
    public static final int VMWARE_READ_TIMEOUT = 60 * 60 * 1000; // 60分钟

    /**
     * Member FeignClient读取默认超时(ms)
     */
    public static final int MEMBER_READ_TIMEOUT = 5 * 60 * 1000; // 5分钟

    /**
     * Member Cluster Client FeignClient读取默认超时(ms)
     */
    public static final int BACKUP_CLUSTER_JOB_CLIENT_READ_TIMEOUT = 10 * 1000; // 10秒

    /**
     * Dme FeignClient读取默认超时(ms)
     */
    public static final int DME_READ_TIMEOUT = 5 * 60 * 1000; // 5分钟

    /**
     * FeignClient Retry 间隔周期(ms)
     */
    public static final int PERIOD = 60 * 1000; // 1分钟

    /**
     * DME Retry 间隔周期(ms)
     */
    public static final int DME_PERIOD = 5 * 1000; // 5秒钟

    /**
     * FeignClient Retry 快速失败间隔周期(ms)
     */
    public static final int FAST_FAIL_PERIOD = 1;

    /**
     * FeignClient Retry 快速失败最大间隔周期(ms)
     */
    public static final int FAST_FAIL_MAX_PERIOD = 1;

    /**
     * FeignClient Retry 最大间隔周期(ms)
     */
    public static final int MAX_PERIOD = 60 * 1000; // 1分钟

    /**
     * DME Retry 最大间隔周期(ms)
     */
    public static final int DME_MAX_PERIOD = 5 * 1000; // 5秒钟

    /**
     * FeignClient Retry 快速失败重试次数
     */
    public static final int FAST_FAIL_MAX_ATTEMPTS = 2;

    /**
     * FeignClient Retry 重试次数
     */
    public static final int MAX_ATTEMPTS = 3;

    /**
     * 路由服务单次连接超时时间 5s
     */
    public static final int ROUTE_SERVICE_CONNECT_TIMEOUT = 5 * 1000;

    /**
     * 路由服务单次读取超时时间 10s
     */
    public static final int ROUTE_SERVICE_READ_TIMEOUT = 10 * 1000;

    /**
     * 路由服务 Retry 重试间隔 3s
     */
    public static final int ROUTE_SERVICE_PERIOD = 3 * 1000;

    /**
     * 路由服务 Retry 重试最大间隔 3s
     */
    public static final int ROUTE_SERVICE_MAX_PERIOD = 3 * 1000;

    /**
     * 路由服务 Retry 最大重试次数
     */
    public static final int ROUTE_SERVICE_ATTEMPT = 3;
}
