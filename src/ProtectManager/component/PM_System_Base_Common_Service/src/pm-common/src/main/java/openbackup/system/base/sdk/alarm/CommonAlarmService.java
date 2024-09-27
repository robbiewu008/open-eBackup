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
package openbackup.system.base.sdk.alarm;

import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.constants.LegoInternalEvent;

import java.util.List;

/**
 * 公共方法接口
 *
 * @author y30000858
 * @since 2020-06-21
 */
public interface CommonAlarmService {
    /**
     * 产生告警\事件
     *
     * @param alarm alarm
     */
    void generateAlarm(LegoInternalAlarm alarm);

    /**
     * 清除告警
     *
     * @param alarm alarm
     */
    void clearAlarm(LegoInternalAlarm alarm);

    /**
     * 清理备节点告警
     *
     * @param alarm alarm
     */
    void clearStandByClusterAlarm(LegoInternalAlarm alarm);

    /**
     * 节点告警是否存在
     *
     * @param alarm alarm
     * @param esn esn
     * @param locationParam locationParam
     * @return 是否存在告警
     */
    boolean clusterAlarmExist(LegoInternalAlarm alarm, String esn, String[] locationParam);

    /**
     * 生成事件
     *
     * @param event event
     */
    void generateEvent(LegoInternalEvent event);

    /**
     * 操作日志
     *
     * @param event   event对象
     * @param isSuccess 是否操作成功
     */
    void generateSysLog(LegoInternalEvent event, Boolean isSuccess);

    /**
     * 检查告警entity用户权限
     *
     * @param userId 用户ID
     * @param entityList 告警唯一标识集合
     */
    void verifyAlarmEntityOwnership(String userId, List<String> entityList);
}
