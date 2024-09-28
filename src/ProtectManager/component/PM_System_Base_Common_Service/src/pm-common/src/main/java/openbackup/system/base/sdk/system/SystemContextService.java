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
package openbackup.system.base.sdk.system;

import openbackup.system.base.sdk.system.model.TimeZoneInfo;

/**
 * 系统上下文服务接口定义，提供系统相关上下文信息的获取接口定义
 *
 */
public interface SystemContextService {
    /**
     * 获取系统的语言
     *
     * @return 返回语言
     */
    String getSystemLanguage();

    /**
     * 查询DM系统时区信息
     *
     * @return 系统时区信息
     */
    TimeZoneInfo getSystemTimeZone();
}
