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
package openbackup.access.framework.resource.service;

/**
 * 功能描述: ProtectObjectService
 *
 */
public interface ProtectObjectConsistentService {
    /**
     * 刷新所有保护对象数据一致性状态
     */
    void refreshProtectObjectConsistentStatus();

    /**
     * 检测数据保护对象一致性状态
     *
     * @param isInit 是否进程重启后触发的
     */
    void checkProtectObjectConsistentStatus(boolean isInit);
}
