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
package openbackup.system.base.service.email;

import openbackup.system.base.service.email.entity.RemoteNotifyServer;

/**
 * 邮箱服务器查询服务
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-14
 */
public interface NotifyServerQueryService {
    /**
     * 获取邮箱服务器
     *
     * @return RemoteNotifyServer
     */
    RemoteNotifyServer queryRemoteNotifyServer();
}
