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
package openbackup.system.base.ssh;

/**
 * SSHService
 *
 * @author l30057246
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/19
 */
public interface AgentRegisterService {
    /**
     * 检查当前agent是否在线
     *
     * @param ip endpoint信息
     * @return agent是否已经在线
     */
    boolean isAgentOnline(String ip);
}
