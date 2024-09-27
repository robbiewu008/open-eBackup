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
 * 池化HttpClient常量
 *
 * @author y30037959
 * @since 2023-03-29
 */
public final class PoolingHttpClientConstant {
    /**
     * 每个路由分配的最大连接数
     */
    public static final int MAX_CONN_PER_ROUTE = 10;

    /**
     * 最大总连接数
     */
    public static final int MAX_CONN_TOTAL = 50;

    /**
     * 连接存活时间
     */
    public static final int TIME_TO_LIVE = 10 * 60 * 1000;

    /**
     * 失活检测时间（失活后多久检查连接是否有效）
     */
    public static final int VALIDATE_AFTER_INACTIVITY = 5 * 60 * 1000;

    private PoolingHttpClientConstant() {
    }
}
