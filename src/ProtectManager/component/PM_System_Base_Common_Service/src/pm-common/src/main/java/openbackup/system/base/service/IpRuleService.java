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
package openbackup.system.base.service;

/**
 * 更新ip路由服务
 *
 */
public interface IpRuleService {
    /**
     * 在底座上给网卡添加到指定目的ip的路由
     *
     * @param destinationIp 目的地ip
     * @param port 目的地端口号
     * @param taskType 任务类型
     */
    void addIpRule(String destinationIp, String port, String taskType);

    /**
     * 在底座上删除到指定目的ip的路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void deleteIpRule(String destinationIp, String taskType);


    /**
     * 只在本控上添加路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void addIpRuleLocal(String destinationIp, String taskType);

    /**
     * 只在本控上删除路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void deleteIpRuleLocal(String destinationIp, String taskType);
}
