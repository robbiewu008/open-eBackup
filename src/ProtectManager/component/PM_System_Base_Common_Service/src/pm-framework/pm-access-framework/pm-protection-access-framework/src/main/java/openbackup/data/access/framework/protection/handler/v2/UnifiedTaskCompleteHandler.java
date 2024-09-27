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
package openbackup.data.access.framework.protection.handler.v2;

import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;

/**
 * 统一任务完成处理器
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-07
 */
public abstract class UnifiedTaskCompleteHandler implements TaskCompleteHandler {
    /**
     * 处理器版本
     */
    public static final String V2 = "v2";

    /**
     * 处理器的版本
     *
     * @return 返回处理器的版本
     */
    protected String version() {
        return V2;
    }
}
